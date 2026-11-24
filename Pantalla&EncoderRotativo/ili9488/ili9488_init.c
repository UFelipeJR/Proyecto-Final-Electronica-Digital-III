#include "ili9488_init.h"
#include "ili9488_hw.h"

typedef struct {
    uint8_t cmd;
    uint8_t n;          // numero de parametros
    uint8_t delay_ms;   // espera despues del comando
    uint8_t p[15];
} lcd_cmd_t;

static const lcd_cmd_t init_seq[] = {
    {0x01,  0, 120, {0}},                                   // SWRESET
    {0x11,  0, 120, {0}},                                   // SLEEP OUT
    {0xE0, 15,   0, {0x00,0x03,0x09,0x08,0x16,0x0A,0x3F,0x78,
                     0x4C,0x09,0x0A,0x08,0x16,0x1A,0x0F}},  // Gamma +
    {0xE1, 15,   0, {0x00,0x16,0x19,0x03,0x0F,0x05,0x32,0x45,
                     0x46,0x04,0x0E,0x0D,0x35,0x37,0x0F}},  // Gamma -
    {0xC0,  2,   0, {0x17,0x15}},                           // Power ctrl 1
    {0xC1,  1,   0, {0x41}},                                // Power ctrl 2
    {0xC5,  3,   0, {0x00,0x12,0x80}},                      // VCOM
    {0x36,  1,   0, {0xE8}},   // MADCTL: horizontal 180 grados (MY+MX+MV) + BGR
    {0x3A,  1,   0, {0x66}},                                // 18 bits/pixel
    {0xB0,  1,   0, {0x00}},                                // Interface mode
    {0xB1,  1,   0, {0xA0}},                                // Frame rate 60 Hz
    {0xB4,  1,   0, {0x02}},                                // Inversion: 2 dot
    {0xB6,  3,   0, {0x02,0x02,0x3B}},                      // Display function
    {0xB7,  1,   0, {0xC6}},                                // Entry mode
    {0xF7,  4,   0, {0xA9,0x51,0x2C,0x82}},                 // Adjust ctrl 3
    {0x29,  0,  25, {0}},                                   // DISPLAY ON
};

// Secuencias cortas para apagar y volver a encender la pantalla.
// Tras SLEEP IN y SLEEP OUT el controlador pide esperar 120 ms.
static const lcd_cmd_t sleep_seq[] = {
    {0x28,  0,   0, {0}},                                   // DISPLAY OFF
    {0x10,  0, 120, {0}},                                   // SLEEP IN
};
static const lcd_cmd_t wake_seq[] = {
    {0x11,  0, 120, {0}},                                   // SLEEP OUT
    {0x29,  0,  25, {0}},                                   // DISPLAY ON
};

#define LEN(tabla)  ((uint8_t)(sizeof(tabla) / sizeof((tabla)[0])))

typedef enum {
    S_IDLE, S_RST_ASSERT, S_RST_LOW_WAIT, S_RST_HIGH_WAIT,
    S_CMD, S_CMD_FLUSH, S_PARAMS, S_END, S_DELAY, S_DONE
} st_t;

static st_t     st = S_IDLE;
static uint8_t  idx, pidx;
static const lcd_cmd_t *seq = init_seq;    // secuencia que se esta enviando
static uint8_t  seq_len = LEN(init_seq);
static uint64_t deadline;

static inline void next_cmd(void) {
    idx++;
    st = (idx >= seq_len) ? S_DONE : S_CMD;
}

void lcd_init_start(void) {
    seq = init_seq;
    seq_len = LEN(init_seq);
    idx = 0;
    st = S_RST_ASSERT;
}

bool lcd_ready(void) { return st == S_DONE; }

// Solo se puede cambiar de secuencia cuando la FSM esta libre (S_DONE)
static void empezar_secuencia(const lcd_cmd_t *tabla, uint8_t largo) {
    if (st != S_DONE) return;
    seq = tabla;
    seq_len = largo;
    idx = 0;
    st = S_CMD;
}

void lcd_suspend(void) { empezar_secuencia(sleep_seq, LEN(sleep_seq)); }
void lcd_resume(void)  { empezar_secuencia(wake_seq,  LEN(wake_seq));  }

void lcd_task(void) {
    switch (st) {

    case S_RST_ASSERT:
        lcd_cs_high();
        lcd_rst_low();
        deadline = time_us_64() + 20000;            // RST en bajo 20 ms
        st = S_RST_LOW_WAIT;
        break;

    case S_RST_LOW_WAIT:
        if (time_us_64() >= deadline) {
            lcd_rst_high();
            deadline = time_us_64() + 120000;       // espera tras reset 120 ms
            st = S_RST_HIGH_WAIT;
        }
        break;

    case S_RST_HIGH_WAIT:
        if (time_us_64() >= deadline) { idx = 0; st = S_CMD; }
        break;

    case S_CMD:                                     // comando: D/C en bajo
        lcd_cs_low();
        lcd_dc_cmd();
        if (lcd_spi_try_write(seq[idx].cmd)) {
            pidx = 0;
            st = S_CMD_FLUSH;
        }
        break;

    case S_CMD_FLUSH:                               // esperar a que salga el byte
        if (lcd_spi_idle()) {
            lcd_dc_data();
            st = seq[idx].n ? S_PARAMS : S_END;
        }
        break;

    case S_PARAMS:                                  // un parametro por pasada
        if (lcd_spi_try_write(seq[idx].p[pidx])) {
            pidx++;
            if (pidx >= seq[idx].n) st = S_END;
        }
        break;

    case S_END:                                     // cerrar transaccion
        if (lcd_spi_idle()) {
            lcd_cs_high();
            if (seq[idx].delay_ms) {
                deadline = time_us_64() + (uint64_t)seq[idx].delay_ms * 1000u;
                st = S_DELAY;
            } else {
                next_cmd();
            }
        }
        break;

    case S_DELAY:
        if (time_us_64() >= deadline) next_cmd();
        break;

    default:                                        // S_IDLE, S_DONE
        break;
    }
}
