#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "menu.h"
#include "encoder.h"
#include "ili9488_init.h"
#include "ili9488_draw.h"
#include "ili9488_gfx.h"
#include "figura.h"
#include "tick.h"

/* ---------- Datos ----------
   Todos los textos tienen ancho FIJO (se rellenan con espacios). Asi el texto
   nuevo tapa por completo al anterior y no hace falta pintar rectangulos negros.
   Los valores son de ejemplo hasta conectar los sensores ("N/D" = no disponible). */
#define N_TABS   5
#define N_SUB    4
#define TAB_CONF 4                      // pestana de interruptores

typedef struct {
    const char *li, *vi, *ui;           // lado izquierdo: rotulo (6), valor (5), unidad (3)
    const char *ld, *vd, *ud;           // lado derecho
} par_t;

typedef struct {
    const char *nombre;                 // 5 caracteres
    uint8_t     nsub;                   // subsecciones usadas (hasta 4)
    const char *sub[N_SUB];             // 6 caracteres
    par_t       par[N_SUB];             // lo que se muestra a los lados en cada subseccion
    const char *cl[4];                  // cajas inferiores, una por sensor: rotulo (12)
    const char *cv[4];                  //                                   valor (6)
} pestana_t;

#define VACIO { "      ", "     ", "   ", "      ", "     ", "   " }

static const pestana_t pestanas[N_TABS] = {
    { "VITAL", 4, { "PULSO ", "SPO2  ", "TEMP  ", "GSR   " },
      { { "PULSO ", "72   ", "BPM", "SPO2  ", "98   ", "%  " },
        { "SPO2  ", "98   ", "%  ", "TEMP  ", "36.5 ", "C  " },
        { "TEMP  ", "36.5 ", "C  ", "GSR   ", "--   ", "US " },
        { "GSR   ", "--   ", "US ", "PULSO ", "72   ", "BPM" } },
      { "PULSO       ", "SPO2        ", "TEMP        ", "GSR         " },
      { "72 BPM",       "98 %  ",       "36.5 C",       "-- US " } },

    { "MOV  ", 3, { "ACEL  ", "GIRO  ", "PASOS ", "      " },
      { { "ACEL  ", "--   ", "G  ", "GIRO  ", "--   ", "DPS" },
        { "GIRO  ", "--   ", "DPS", "PASOS ", "--   ", "   " },
        { "PASOS ", "--   ", "   ", "ACEL  ", "--   ", "G  " },
        VACIO },
      { "ACEL        ", "GIRO        ", "PASOS       ", "IMU         " },
      { "-- G  ",       "-- DPS",       "--    ",      "N/D   " } },

    { "PWR  ", 4, { "BATER ", "VOLT  ", "CORR  ", "POT   " },
      { { "BATER ", "87   ", "%  ", "VOLT  ", "--   ", "V  " },
        { "VOLT  ", "--   ", "V  ", "CORR  ", "--   ", "MA " },
        { "CORR  ", "--   ", "MA ", "POT   ", "--   ", "MW " },
        { "POT   ", "--   ", "MW ", "BATER ", "87   ", "%  " } },
      { "BATERIA     ", "VOLTAJE     ", "CORRIENTE   ", "POTENCIA    " },
      { "87 %  ",       "-- V  ",       "-- MA ",       "-- MW " } },

    { "HORA ", 3, { "HORA  ", "FECHA ", "AJUST ", "      " },
      { { "HORA  ", "--:--", "   ", "FECHA ", "--/--", "   " },
        { "FECHA ", "--/--", "   ", "AJUST ", "--   ", "   " },
        { "AJUST ", "--   ", "   ", "HORA  ", "--:--", "   " },
        VACIO },
      { "HORA        ", "FECHA       ", "AJUSTES     ", "RTC         " },
      { "--:-- ",       "--/-- ",       "--    ",      "N/D   " } },

    // CONF: los datos de los lados y de las cajas se arman al dibujar (ver par_de y cv_de)
    { "CONF ", 4, { "BLE   ", "BUZZER", "VIBRA ", "SALIR " },
      { VACIO, VACIO, VACIO, VACIO },
      { "BLUETOOTH   ", "BUZZER      ", "VIBRACION   ", "            " },
      { "      ",       "      ",       "      ",       "      " } },
};

// Interruptores de CONF: 0 = Bluetooth, 1 = buzzer, 2 = vibracion
static bool cfg[3] = { true, true, true };

bool menu_ble_habilitado(void)    { return cfg[0]; }
bool menu_buzzer_habilitado(void) { return cfg[1]; }
bool menu_vibra_habilitado(void)  { return cfg[2]; }

static par_t par_de(uint8_t t, uint8_t s) {
    if (t != TAB_CONF) return pestanas[t].par[s];
    par_t p;
    p.li = pestanas[t].sub[s];
    p.vi = (s < 3) ? (cfg[s] ? "ON   " : "OFF  ") : "     ";
    p.ui = "   ";
    p.ld = "ACCION";
    p.vd = "PULSA";
    p.ud = "   ";
    return p;
}

static const char *cv_de(uint8_t t, uint8_t k) {
    if (t == TAB_CONF && k < 3) return cfg[k] ? "ON    " : "OFF   ";
    return pestanas[t].cv[k];
}

/* ---------- Geometria (pantalla 480x320) ---------- */
#define TAB_Y       20
#define TAB_X(i)    (20 + 90 * (i))         // 5 pestanas de 7 caracteres (84 px)
#define LINEA_Y     42
#define SUB_Y       50
#define SUB_X(k)    (30 + 108 * (k))        // 4 subsecciones de 8 caracteres (96 px)
#define FIG_X       ((LCD_W - FIGURA_W) / 2)
#define FIG_Y       72
#define IZQ_X       36                      // columna de datos izquierda
#define DER_X       300                     // columna de datos derecha
#define CAJA_Y      220
#define CAJA_X(k)   (24 + 110 * (k))        // 4 cajas, una por sensor
#define CAJA_W      104
#define CAJA_H      32
#define LADO_Y_ROTULO  92                   // filas de los datos laterales
#define LADO_Y_VALOR   116
#define LADO_Y_UNIDAD  146

// Franja inferior: tres cajas (bateria | barra de nivel | hora)
#define BARRA_Y        262
#define BARRA_H        32
#define BARRA_BAT_X    24
#define BARRA_BAT_W    130
#define BARRA_NIVEL_X  160
#define BARRA_NIVEL_W  160
#define BARRA_HORA_X   326
#define BARRA_HORA_W   130
#define BARRA_TEXTO_Y  (BARRA_Y + 8)
#define SEG_X(k)       (166 + 15 * (k))     // 10 segmentos de 12 px
#define SEG_W          12
#define SEG_H          16
#define BAT_NIVEL      9                    // segmentos encendidos (demo)
#define CURSOR_X    466

// Trabajos que ocupa cada dibujo en la cola (maximo 31)
#define JOBS_ARR1   18      // fondo, linea, pestanas, subsecciones, figura, lados
#define JOBS_ARR2   17      // cajas y franja inferior (sin la barra)
#define JOBS_ARR3   10      // segmentos de la barra de bateria
#define JOBS_CAMBIO 20      // cambio de pestana: 2 + 4 + 6 + 8

#define PARPADEO_TICKS   (500u / TICK_MS)       // medio periodo del cursor (500 ms)
#define INACTIVO_TICKS   (10000u / TICK_MS)     // sin usar el encoder 10 s: se suspende la pantalla

/* ---------- Estado ----------
   Lo LOGICO (estado, destino, sub_dest, cfg) cambia al instante con cada evento.
   Lo PINTADO es lo que realmente muestra la pantalla. El dibujo solo se encola
   cuando la cola esta libre y va directo al destino final. */
typedef enum {
    M_INICIO, M_INICIO2, M_INICIO3,     // arranque en tres fases
    M_NAVEGAR,                          // el giro cambia de pestana; el pulsador entra
    M_DENTRO                            // el giro cambia de subseccion; el pulsador sale
                                        // (en CONF, el pulsador cambia el interruptor)
} m_estado_t;

static m_estado_t estado    = M_INICIO;
static uint8_t    destino   = 0, sub_dest = 0;
static uint8_t    pintada   = 0, sub_pint = 0;
static bool       pintado_dentro = false;
static bool       sucio_datos = false;  // un interruptor cambio: repintar lados y cajas

static bool       cursor_on = false;
static uint32_t   cursor_t;            // en ticks (tick_count)

/* Suspension de la pantalla. Es independiente de "estado": al despertar, el menu
   sigue exactamente donde estaba. */
typedef enum {
    R_ACTIVA,        // pantalla encendida y en uso
    R_DURMIENDO,     // se mando el apagado; esperando a que termine
    R_APAGADA,       // pantalla apagada; solo se espera un evento para despertar
    R_DESPERTANDO    // se mando el encendido; esperando a que termine
} reposo_t;

static reposo_t   reposo = R_ACTIVA;
static uint32_t   ultimo_uso;          // tick del ultimo evento del encoder

static bool       hay_pend = false;     // evento mas antiguo aun sin pintar
static uint32_t   t_pend;
static bool       midiendo = false;
static uint32_t   t_evento_us, lat_max_us;

/* ---------- Dibujo ---------- */
// modo 0: inactiva
// modo 1: activa mientras se navega (brillo, con corchetes)
// modo 2: activa mientras se esta dentro (medio, con corchetes)
static void rotulo_tab(uint8_t i, uint8_t modo) {
    char b[8];
    b[0] = modo ? '[' : ' ';
    memcpy(&b[1], pestanas[i].nombre, 5);
    b[6] = modo ? ']' : ' ';
    b[7] = '\0';
    lcd_text(TAB_X(i), TAB_Y, b, (modo == 1) ? COL_VERDE_BRILLO : COL_VERDE_MEDIO, COL_NEGRO, 2);
}

// modo 0: inactiva (oscuro)
// modo 1: seleccionada, pero aun no se entra (medio)
// modo 2: activa mientras se esta dentro (brillo, con corchetes)
static void rotulo_sub(uint8_t t, uint8_t k, uint8_t modo) {
    char b[9];
    b[0] = (modo == 2) ? '[' : ' ';
    memcpy(&b[1], pestanas[t].sub[k], 6);
    b[7] = (modo == 2) ? ']' : ' ';
    b[8] = '\0';
    lcd_text(SUB_X(k), SUB_Y, b,
             (modo == 2) ? COL_VERDE_BRILLO : (modo == 1) ? COL_VERDE_MEDIO : COL_VERDE_OSCURO,
             COL_NEGRO, 2);
}

static void subs(uint8_t t, uint8_t activa, bool dentro) {
    rotulo_sub(t, 0, (activa == 0) ? (dentro ? 2 : 1) : 0);
    rotulo_sub(t, 1, (activa == 1) ? (dentro ? 2 : 1) : 0);
    rotulo_sub(t, 2, (activa == 2) ? (dentro ? 2 : 1) : 0);
    rotulo_sub(t, 3, (activa == 3) ? (dentro ? 2 : 1) : 0);
}

static void lados(uint8_t t, uint8_t s) {
    par_t p = par_de(t, s);
    lcd_text(IZQ_X, LADO_Y_ROTULO,  p.li, COL_VERDE_MEDIO,  COL_NEGRO, 2);
    lcd_text(IZQ_X, LADO_Y_VALOR, p.vi, COL_VERDE_BRILLO, COL_NEGRO, 3);
    lcd_text(IZQ_X, LADO_Y_UNIDAD, p.ui, COL_VERDE_MEDIO,  COL_NEGRO, 2);
    lcd_text(DER_X, LADO_Y_ROTULO,  p.ld, COL_VERDE_MEDIO,  COL_NEGRO, 2);
    lcd_text(DER_X, LADO_Y_VALOR, p.vd, COL_VERDE_BRILLO, COL_NEGRO, 3);
    lcd_text(DER_X, LADO_Y_UNIDAD, p.ud, COL_VERDE_MEDIO,  COL_NEGRO, 2);
}

#define CAJA_TXT(t, k) do { \
        lcd_text(CAJA_X(k) + 8, CAJA_Y + 3,  pestanas[t].cl[k], \
                 COL_VERDE_MEDIO,  COL_VERDE_OSCURO, 1); \
        lcd_text(CAJA_X(k) + 8, CAJA_Y + 14, cv_de(t, k), \
                 COL_VERDE_BRILLO, COL_VERDE_OSCURO, 2); \
    } while (0)

static void cajas_texto(uint8_t t) {
    CAJA_TXT(t, 0);
    CAJA_TXT(t, 1);
    CAJA_TXT(t, 2);
    CAJA_TXT(t, 3);
}

// Figura fija del centro (se dibuja una sola vez, al arrancar)
static void dibujar_figura(void) {
    static const uint32_t paleta[4] = {
        COL_NEGRO, COL_VERDE_OSCURO, COL_VERDE_MEDIO, COL_VERDE_BRILLO
    };
    lcd_image_push(FIG_X, FIG_Y, FIGURA_W, FIGURA_H, figura_img, paleta);
}

// Fase 1 del arranque
static void arranque1(void) {
    lcd_rect(0, 0, LCD_W, LCD_H, COL_NEGRO);
    lcd_hline(24, LINEA_Y, 432, 2, COL_VERDE_MEDIO);
    rotulo_tab(0, 1);
    rotulo_tab(1, 0);
    rotulo_tab(2, 0);
    rotulo_tab(3, 0);
    rotulo_tab(4, 0);
    subs(0, 0, false);
    dibujar_figura();
    lados(0, 0);
    pintada = 0; sub_pint = 0; pintado_dentro = false;
}

// Fase 2: cajas de datos y franja inferior (el texto va despues de su caja)
static void arranque2(void) {
    lcd_rect(CAJA_X(0), CAJA_Y, CAJA_W, CAJA_H, COL_VERDE_OSCURO);
    lcd_rect(CAJA_X(1), CAJA_Y, CAJA_W, CAJA_H, COL_VERDE_OSCURO);
    lcd_rect(CAJA_X(2), CAJA_Y, CAJA_W, CAJA_H, COL_VERDE_OSCURO);
    lcd_rect(CAJA_X(3), CAJA_Y, CAJA_W, CAJA_H, COL_VERDE_OSCURO);
    cajas_texto(0);
    lcd_rect(BARRA_BAT_X,   BARRA_Y, BARRA_BAT_W,   BARRA_H, COL_VERDE_OSCURO);
    lcd_rect(BARRA_NIVEL_X, BARRA_Y, BARRA_NIVEL_W, BARRA_H, COL_VERDE_OSCURO);
    lcd_rect(BARRA_HORA_X,  BARRA_Y, BARRA_HORA_W,  BARRA_H, COL_VERDE_OSCURO);
    lcd_text(BARRA_BAT_X + 10,  BARRA_TEXTO_Y, "BAT 87%", COL_VERDE_BRILLO, COL_VERDE_OSCURO, 2);
    lcd_text(BARRA_HORA_X + 35, BARRA_TEXTO_Y, "20:45",   COL_VERDE_BRILLO, COL_VERDE_OSCURO, 2);
}

// Fase 3: barra de bateria (10 segmentos, escritos uno por uno sin ciclos)
#define SEGB(k) lcd_rect(SEG_X(k), BARRA_TEXTO_Y, SEG_W, SEG_H, \
                         ((k) < BAT_NIVEL) ? COL_VERDE_BRILLO : COL_NEGRO)
static void arranque3(void) {
    SEGB(0); SEGB(1); SEGB(2); SEGB(3); SEGB(4);
    SEGB(5); SEGB(6); SEGB(7); SEGB(8); SEGB(9);
}

/* ---------- Paso 1: recibir un evento (solo logica, no dibuja) ---------- */
static void recibir_evento(void) {
    enc_event_t ev;
    if (!enc_pop(&ev)) return;
    ultimo_uso = tick_count();
    if (!hay_pend) { hay_pend = true; t_pend = ev.t_us; }

    if (estado == M_NAVEGAR) {
        if (ev.type == ENC_EVT_PRESS) {
            estado = M_DENTRO;
        } else {
            destino  = (ev.type == ENC_EVT_CW) ? (uint8_t)((destino + 1) % N_TABS)
                                               : (uint8_t)((destino + N_TABS - 1) % N_TABS);
            sub_dest = 0;
        }
    } else {                                           // M_DENTRO
        uint8_t n = pestanas[destino].nsub;
        if (ev.type == ENC_EVT_PRESS) {
            if (destino == TAB_CONF && sub_dest < 3) { // un interruptor: se cambia, no se sale
                cfg[sub_dest] = !cfg[sub_dest];
                sucio_datos = true;
            } else {
                estado = M_NAVEGAR;
            }
        } else {
            sub_dest = (ev.type == ENC_EVT_CW) ? (uint8_t)((sub_dest + 1) % n)
                                               : (uint8_t)((sub_dest + n - 1) % n);
        }
    }
}

/* ---------- Paso 2: repintar si la pantalla quedo libre ---------- */
static void repintar(void) {
    if (!lcd_draw_idle()) return;

    bool dentro = (estado == M_DENTRO);
    bool ct = (destino  != pintada);
    bool cs = (sub_dest != sub_pint);
    bool ce = (dentro   != pintado_dentro);
    if (!ct && !cs && !ce && !sucio_datos) { hay_pend = false; return; }
    if (lcd_fill_free() < JOBS_CAMBIO) return;

    if (ct) {
        rotulo_tab(pintada, 0);                        // la anterior se apaga
        rotulo_tab(destino, dentro ? 2 : 1);
        subs(destino, sub_dest, dentro);
        lados(destino, sub_dest);
        cajas_texto(destino);
    } else {
        rotulo_tab(destino, dentro ? 2 : 1);
        subs(destino, sub_dest, dentro);
        if (cs || sucio_datos) lados(destino, sub_dest);
        if (sucio_datos)       cajas_texto(destino);
    }
    pintada = destino; sub_pint = sub_dest; pintado_dentro = dentro;
    sucio_datos = false;

    t_evento_us = t_pend;
    midiendo    = hay_pend;
    hay_pend    = false;
}

/* ---------- Cursor parpadeante junto a las pestanas ---------- */
static void parpadeo(void) {
    if (hay_pend || !lcd_draw_idle() || lcd_fill_free() < 1) return;
    if (tick_count() - cursor_t < PARPADEO_TICKS) return;
    cursor_t  = tick_count();
    cursor_on = !cursor_on;
    lcd_text(CURSOR_X, TAB_Y, cursor_on ? "_" : " ", COL_VERDE_BRILLO, COL_NEGRO, 2);
}

static void medir_latencia(void) {
    if (!midiendo || !lcd_draw_idle()) return;
    uint32_t lat = time_us_32() - t_evento_us;
    if (lat > lat_max_us) lat_max_us = lat;
    printf("PR-03: %lu ms (max %lu ms) %s\n",
           (unsigned long)(lat / 1000u), (unsigned long)(lat_max_us / 1000u),
           (lat < 200000u) ? "OK" : "SUPERA 200 ms");
    midiendo = false;
}

bool menu_ocupado(void) {
    if (reposo == R_APAGADA) return false;       // solo falta esperar al encoder
    if (reposo != R_ACTIVA)  return true;        // apagando o encendiendo la pantalla
    return estado < M_NAVEGAR || hay_pend || midiendo || sucio_datos
        || destino != pintada || sub_dest != sub_pint
        || ((estado == M_DENTRO) != pintado_dentro);
}

/* ---------- Suspension por inactividad ---------- */
// Pantalla encendida: si no queda nada por hacer y pasaron 10 s sin usar el
// encoder, se apaga el panel.
static void revisar_inactividad(void) {
    if (menu_ocupado() || !lcd_draw_idle()) return;
    if (tick_count() - ultimo_uso < INACTIVO_TICKS) return;
    lcd_suspend();
    reposo = R_DURMIENDO;
}

// Pantalla suspendida (o en transicion). El primer evento solo despierta: no
// cambia de pestana ni entra a ninguna parte.
static void atender_reposo(void) {
    enc_event_t ev;

    switch (reposo) {
    case R_DURMIENDO:
        if (lcd_ready()) reposo = R_APAGADA;
        break;

    case R_APAGADA:
        if (enc_pop(&ev)) {
            lcd_resume();
            reposo = R_DESPERTANDO;
        }
        break;

    case R_DESPERTANDO:
        if (lcd_ready()) {
            reposo     = R_ACTIVA;
            ultimo_uso = tick_count();
            cursor_t   = tick_count();
        }
        break;

    default:
        break;
    }
}

/* ---------- FSM: una pasada ---------- */
void menu_step(void) {
    switch (estado) {
    case M_INICIO:
        if (lcd_ready() && lcd_fill_free() >= JOBS_ARR1) {
            arranque1();
            estado = M_INICIO2;
        }
        break;

    case M_INICIO2:
        if (lcd_fill_free() >= JOBS_ARR2) {
            arranque2();
            estado = M_INICIO3;
        }
        break;

    case M_INICIO3:
        if (lcd_fill_free() >= JOBS_ARR3) {
            arranque3();
            cursor_t   = tick_count();
            ultimo_uso = tick_count();
            estado     = M_NAVEGAR;
        }
        break;

    default:                                    // M_NAVEGAR o M_DENTRO
        if (reposo == R_ACTIVA) {
            recibir_evento();
            repintar();
            parpadeo();
            revisar_inactividad();
        } else {
            atender_reposo();
        }
        break;
    }
    medir_latencia();
}
