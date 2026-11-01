#include "ili9488_gfx.h"
#include "ili9488_draw.h"

#define C_R(c)  ((uint8_t)((c) >> 16))
#define C_G(c)  ((uint8_t)((c) >> 8))
#define C_B(c)  ((uint8_t)(c))

static bool in_screen(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    return w && h && (uint32_t)x + w <= LCD_W && (uint32_t)y + h <= LCD_H;
}

bool lcd_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, lcd_color_t c) {
    if (!in_screen(x, y, w, h) || lcd_fill_free() < 1) return false;
    return lcd_fill_push(x, y, x + w - 1, y + h - 1, C_R(c), C_G(c), C_B(c));
}

bool lcd_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t th, lcd_color_t c) {
    return lcd_rect(x, y, len, th, c);
}

bool lcd_vline(uint16_t x, uint16_t y, uint16_t len, uint16_t th, lcd_color_t c) {
    return lcd_rect(x, y, th, len, c);
}

// Contorno de espesor th hacia adentro. Cuatro rectangulos sin solaparse.
bool lcd_rect_outline(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                      uint16_t th, lcd_color_t c) {
    if (!in_screen(x, y, w, h) || th == 0 ||
        w < 2u * th || h < 2u * th || lcd_fill_free() < 4) return false;

    uint16_t mid_h = h - 2u * th;                  // alto de los lados
    lcd_rect(x,               y,          w,  th,    c);   // superior
    lcd_rect(x,               y + h - th, w,  th,    c);   // inferior
    if (mid_h) {
        lcd_rect(x,           y + th,     th, mid_h, c);   // izquierdo
        lcd_rect(x + w - th,  y + th,     th, mid_h, c);   // derecho
    }
    return true;
}

bool lcd_text(uint16_t x, uint16_t y, const char *s,
              lcd_color_t ink, lcd_color_t paper, uint8_t scale) {
    return lcd_text_push(x, y, s, ink, paper, scale);
}
