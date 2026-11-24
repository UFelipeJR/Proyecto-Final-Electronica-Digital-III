#include <string.h>
#include "ili9488_draw.h"
#include "ili9488_gfx.h"        // LCD_W / LCD_H
#include "ili9488_font.h"
#include "ili9488_hw.h"

/* ---------- Transmisor de comandos ---------- */

typedef enum { T_CMD, T_FLUSH, T_PARAMS, T_END } tx_st_t;

static tx_st_t       tx_st;
static uint8_t       tx_cmd, tx_n, tx_i;
static const uint8_t *tx_p;
static bool          tx_keep_cs;     // true: no subir CS al terminar (sigue RAMWR)

static void tx_begin(uint8_t cmd, const uint8_t *p, uint8_t n, bool keep_cs) {
    tx_cmd = cmd; tx_p = p; tx_n = n; tx_i = 0;
    tx_keep_cs = keep_cs;
    tx_st = T_CMD;
}

// Devuelve true cuando el comando completo ya salio
static bool tx_step(void) {
    switch (tx_st) {
    case T_CMD:
        lcd_cs_low();
        lcd_dc_cmd();
        if (lcd_spi_try_write(tx_cmd)) tx_st = T_FLUSH;
        break;

    case T_FLUSH:                       // el byte debe salir antes de cambiar D/C
        if (lcd_spi_idle()) {
            lcd_dc_data();
            tx_st = tx_n ? T_PARAMS : T_END;
        }
        break;

    case T_PARAMS:
        if (lcd_spi_try_write(tx_p[tx_i])) {
            tx_i++;
            if (tx_i >= tx_n) tx_st = T_END;
        }
        break;

    case T_END:
        if (lcd_spi_idle()) {
            if (!tx_keep_cs) lcd_cs_high();
            return true;
        }
        break;
    }
    return false;
}

/* ---------- Trabajo de dibujo: relleno solido, texto o imagen de 2 bits ---------- */

#define TXT_MAX  24u

typedef struct {
    uint16_t x0, y0, x1, y1;        // ventana
    uint8_t  ink[3];                // relleno, o tinta del texto
    uint8_t  paper[3];              // fondo del texto
    uint8_t  scale;                 // 0 = relleno solido, >=1 = texto
    uint8_t  len;                   // caracteres del texto
    char     txt[TXT_MAX];
    const uint8_t *img;             // imagen 2 bits/pixel (NULL si no es imagen)
    uint8_t  pal[4][3];             // paleta de la imagen
} job_t;

typedef enum { D_IDLE, D_CASET, D_PASET, D_RAMWR, D_PIXELS, D_CLOSE } d_st_t;

static d_st_t   d_st = D_IDLE;
static uint8_t  caset[4], paset[4], rgb[3];
static uint8_t  ch;                  // canal actual: 0=R, 1=G, 2=B
static uint32_t bytes_left;
static job_t    cur;                 // trabajo en curso
static uint8_t  gi, gx, sx, sy, gy;  // posicion dentro del texto
static uint32_t img_i;               // pixel actual de la imagen

static void set_rgb(uint8_t d[3], uint32_t c) {
    d[0] = (uint8_t)(c >> 16);
    d[1] = (uint8_t)(c >> 8);
    d[2] = (uint8_t)c;
}

static bool job_valid(const job_t *j) {
    return j->x1 >= j->x0 && j->y1 >= j->y0 && j->x1 < LCD_W && j->y1 < LCD_H;
}

// Color del pixel actual del texto: tinta si el bit esta encendido, fondo si no.
static void text_load_pixel(void) {
    bool on = false;
    if (gx < 5 && gy < 7) {                         // columna 5 y fila 7: espacio
        uint8_t c = (uint8_t)cur.txt[gi];
        if (c < FONT_FIRST || c > FONT_LAST) c = '?';
        on = (font5x7[c - FONT_FIRST][gx] >> gy) & 1u;
    }
    const uint8_t *p = on ? cur.ink : cur.paper;
    rgb[0] = p[0]; rgb[1] = p[1]; rgb[2] = p[2];
}

// Color del pixel actual de la imagen: 2 bits -> entrada de la paleta.
static void image_load_pixel(void) {
    uint8_t b = cur.img[img_i >> 2];
    uint8_t k = (b >> (6u - 2u * (img_i & 3u))) & 3u;
    rgb[0] = cur.pal[k][0]; rgb[1] = cur.pal[k][1]; rgb[2] = cur.pal[k][2];
}

// Orden de barrido: subpixel x, columna, caracter, subpixel y, fila.
static void text_advance(void) {
    if (++sx < cur.scale) return;
    sx = 0;
    if (++gx < 6) return;
    gx = 0;
    if (++gi < cur.len) return;
    gi = 0;
    if (++sy < cur.scale) return;
    sy = 0;
    gy++;
}

static bool job_start(const job_t *j) {
    if (d_st != D_IDLE || !job_valid(j)) return false;
    cur = *j;

    caset[0] = j->x0 >> 8; caset[1] = j->x0 & 0xFF;
    caset[2] = j->x1 >> 8; caset[3] = j->x1 & 0xFF;
    paset[0] = j->y0 >> 8; paset[1] = j->y0 & 0xFF;
    paset[2] = j->y1 >> 8; paset[3] = j->y1 & 0xFF;

    bytes_left = (uint32_t)(j->x1 - j->x0 + 1) * (j->y1 - j->y0 + 1) * 3u;
    ch = 0;
    gi = gx = sx = sy = gy = 0;
    img_i = 0;
    if (cur.img) image_load_pixel();
    else if (cur.scale) text_load_pixel();
    else { rgb[0] = cur.ink[0]; rgb[1] = cur.ink[1]; rgb[2] = cur.ink[2]; }

    tx_begin(0x2A, caset, 4, false);
    d_st = D_CASET;
    return true;
}

/* ---------- Cola de trabajos ---------- */

#define Q_SIZE 32u                       // potencia de 2
#define Q_MASK (Q_SIZE - 1u)

static job_t   q[Q_SIZE];
static uint8_t q_head, q_tail;           // head: escribe, tail: lee

static bool job_push(const job_t *j) {
    uint8_t next = (q_head + 1u) & Q_MASK;
    if (!job_valid(j) || next == q_tail) return false;   // invalido o cola llena
    q[q_head] = *j;
    q_head = next;
    return true;
}

uint8_t lcd_fill_free(void) {
    return (uint8_t)((q_tail - q_head - 1u) & Q_MASK);
}

bool lcd_draw_idle(void) {
    return d_st == D_IDLE && q_head == q_tail;
}

bool lcd_fill_push(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                   uint8_t r, uint8_t g, uint8_t b) {
    job_t j;
    memset(&j, 0, sizeof j);
    j.x0 = x0; j.y0 = y0; j.x1 = x1; j.y1 = y1;
    j.ink[0] = r; j.ink[1] = g; j.ink[2] = b;
    return job_push(&j);
}

bool lcd_text_push(uint16_t x, uint16_t y, const char *s,
                   uint32_t ink, uint32_t paper, uint8_t scale) {
    size_t n = strlen(s);
    if (scale == 0 || n == 0 || n > TXT_MAX) return false;

    uint32_t w = (uint32_t)n * 6u * scale;
    uint32_t h = 8u * scale;
    if ((uint32_t)x + w > LCD_W || (uint32_t)y + h > LCD_H) return false;

    job_t j;
    memset(&j, 0, sizeof j);
    j.x0 = x;              j.y0 = y;
    j.x1 = (uint16_t)(x + w - 1u);
    j.y1 = (uint16_t)(y + h - 1u);
    set_rgb(j.ink, ink);
    set_rgb(j.paper, paper);
    j.scale = scale;
    j.len   = (uint8_t)n;
    memcpy(j.txt, s, n);
    return job_push(&j);
}

// Encola una imagen de w x h pixeles, 2 bits por pixel (4 pixeles por byte,
// el primero en los bits altos). pal[k] = color 0x00RRGGBB del indice k.
bool lcd_image_push(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                    const uint8_t *img, const uint32_t pal[4]) {
    if (img == 0 || w == 0 || h == 0) return false;
    if ((uint32_t)x + w > LCD_W || (uint32_t)y + h > LCD_H) return false;

    job_t j;
    memset(&j, 0, sizeof j);
    j.x0 = x;                  j.y0 = y;
    j.x1 = (uint16_t)(x + w - 1u);
    j.y1 = (uint16_t)(y + h - 1u);
    j.img = img;
    set_rgb(j.pal[0], pal[0]); set_rgb(j.pal[1], pal[1]);
    set_rgb(j.pal[2], pal[2]); set_rgb(j.pal[3], pal[3]);
    return job_push(&j);
}

// Envia un byte del pixel actual si el FIFO tiene espacio. Devuelve true si salio.
static bool pixel_byte(void) {
    if (d_st != D_PIXELS) return false;           // el trabajo ya termino
    if (!lcd_spi_try_write(rgb[ch])) return false;

    bytes_left--;
    if (bytes_left == 0) {
        d_st = D_CLOSE;
    } else if (ch == 2) {                         // pixel completo: preparar el siguiente
        ch = 0;
        if (cur.img) { img_i++; image_load_pixel(); }
        else if (cur.scale) { text_advance(); text_load_pixel(); }
    } else {
        ch++;
    }
    return true;
}

/* ---------- FSM de dibujo ---------- */

void lcd_draw_task(void) {
    switch (d_st) {

    case D_IDLE:
        if (q_tail != q_head && job_start(&q[q_tail]))
            q_tail = (q_tail + 1u) & Q_MASK;
        break;

    case D_CASET:
        if (tx_step()) { tx_begin(0x2B, paset, 4, false); d_st = D_PASET; }
        break;

    case D_PASET:
        if (tx_step()) { tx_begin(0x2C, 0, 0, true); d_st = D_RAMWR; }
        break;

    case D_RAMWR:                        // CS queda en bajo, D/C en dato
        if (tx_step()) d_st = D_PIXELS;
        break;

    case D_PIXELS:                       // hasta 3 bytes (un pixel) por pasada, sin ciclos
        if (pixel_byte() && pixel_byte()) pixel_byte();
        break;

    case D_CLOSE:
        if (lcd_spi_idle()) {
            lcd_cs_high();
            d_st = D_IDLE;
        }
        break;

    default:
        break;
    }
}
