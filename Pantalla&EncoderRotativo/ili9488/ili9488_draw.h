#ifndef ILI9488_DRAW_H
#define ILI9488_DRAW_H

#include <stdbool.h>
#include <stdint.h>

void lcd_draw_task(void);        // un paso corto por llamada

// Encola un relleno. Devuelve false si es invalido o la cola esta llena.
bool lcd_fill_push(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                   uint8_t r, uint8_t g, uint8_t b);

// Encola un texto (hasta 24 caracteres, copiado en la cola). Colores 0x00RRGGBB.
// Celda de 6x8 px por caracter multiplicada por scale. Devuelve false si no cabe.
bool lcd_text_push(uint16_t x, uint16_t y, const char *s,
                   uint32_t ink, uint32_t paper, uint8_t scale);

// Encola una imagen de w x h pixeles a 2 bits por pixel (4 pixeles por byte, el
// primero en los bits altos). pal[k] es el color 0x00RRGGBB del indice k.
// La imagen debe seguir existiendo hasta que se dibuje (usar const en flash).
bool lcd_image_push(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                    const uint8_t *img, const uint32_t pal[4]);

// Espacios libres en la cola de trabajos
uint8_t lcd_fill_free(void);

// true si no hay dibujo en curso ni trabajos en cola
bool lcd_draw_idle(void);

#endif /* ILI9488_DRAW_H */
