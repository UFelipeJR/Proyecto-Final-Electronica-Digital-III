#ifndef ILI9488_GFX_H
#define ILI9488_GFX_H

#include <stdbool.h>
#include <stdint.h>

#define LCD_W  480
#define LCD_H  320

// Color 0x00RRGGBB
typedef uint32_t lcd_color_t;
#define LCD_RGB(r, g, b)  (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

// Paleta fosforo verde
#define COL_NEGRO         LCD_RGB(0,   0,   0)
#define COL_VERDE_OSCURO  LCD_RGB(0,   70,  20)
#define COL_VERDE_MEDIO   LCD_RGB(0,   160, 40)
#define COL_VERDE_BRILLO  LCD_RGB(40,  255, 70)

// Todas devuelven false (sin dibujar nada) si el area sale de la pantalla,
// tiene tamano 0 o la cola no tiene espacio para la figura completa.
bool lcd_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, lcd_color_t c);
bool lcd_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t th, lcd_color_t c);
bool lcd_vline(uint16_t x, uint16_t y, uint16_t len, uint16_t th, lcd_color_t c);
bool lcd_rect_outline(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                      uint16_t th, lcd_color_t c);

// Texto con fuente 5x7. Celda de LCD_GLYPH_W x LCD_GLYPH_H px por caracter,
// multiplicada por scale. El texto pinta tambien su fondo (paper).
#define LCD_GLYPH_W  6
#define LCD_GLYPH_H  8
bool lcd_text(uint16_t x, uint16_t y, const char *s,
              lcd_color_t ink, lcd_color_t paper, uint8_t scale);

#endif /* ILI9488_GFX_H */
