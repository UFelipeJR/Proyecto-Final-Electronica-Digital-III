#ifndef TICK_H
#define TICK_H

#include <stdint.h>

#define TICK_MS  10u        // periodo de la interrupcion de temporizador

void     tick_init(void);   // arranca la interrupcion periodica
uint32_t tick_count(void);  // ticks transcurridos (cada TICK_MS ms)

#endif /* TICK_H */
