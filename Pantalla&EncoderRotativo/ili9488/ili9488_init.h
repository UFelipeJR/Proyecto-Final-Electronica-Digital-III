#ifndef ILI9488_INIT_H
#define ILI9488_INIT_H

#include <stdbool.h>

void lcd_init_start(void);   // arranca la FSM (reset por hardware + secuencia)
void lcd_task(void);         // un paso corto por llamada, nunca bloquea
bool lcd_ready(void);        // true cuando la FSM esta libre (iniciar, suspender o reanudar)

// Apagan y encienden el panel por comando (DISPLAY OFF + SLEEP IN, y al reves).
// La memoria de imagen se conserva. Solo funcionan si lcd_ready() es true y no
// hay dibujo en curso; despues hay que esperar a que lcd_ready() vuelva a ser true.
void lcd_suspend(void);
void lcd_resume(void);

#endif /* ILI9488_INIT_H */
