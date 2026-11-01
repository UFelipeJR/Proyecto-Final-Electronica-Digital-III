#ifndef ILI9488_FONT_H
#define ILI9488_FONT_H

#include <stdint.h>

#define FONT_FIRST  32      // ' '
#define FONT_LAST   126     // '~'

// Fuente 5x7, una fila de 5 columnas por caracter. Bit 0 = fila superior.
extern const uint8_t font5x7[FONT_LAST - FONT_FIRST + 1][5];

#endif /* ILI9488_FONT_H */
