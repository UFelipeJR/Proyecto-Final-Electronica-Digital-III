#include "tick.h"
#include "pico/stdlib.h"

/* Interrupcion periodica: solo cuenta. Su funcion principal es despertar al
   microcontrolador del __wfi() para que el loop revise si hay trabajo (por
   ejemplo el parpadeo del cursor). El loop usa tick_count() como reloj grueso. */
static volatile uint32_t ticks;
static struct repeating_timer temporizador;

static bool tick_isr(struct repeating_timer *t) {
    (void)t;
    ticks++;
    return true;                // seguir repitiendo
}

void tick_init(void) {
    add_repeating_timer_ms(-(int32_t)TICK_MS, tick_isr, NULL, &temporizador);
}

uint32_t tick_count(void) { return ticks; }
