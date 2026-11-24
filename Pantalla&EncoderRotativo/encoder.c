#include "encoder.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"

#define ENC_MASK_DT   (1u << ENC_PIN_DT)
#define ENC_MASK_ALL  ((1u << ENC_PIN_CLK) | (1u << ENC_PIN_DT) | (1u << ENC_PIN_SW))

#define ENC_INVERT        0        // pon 1 si el sentido sale al reves
#define ROT_REPOSO_US     5000u    // tras subir CLK, ignora bajadas (son rebote)
#define SW_DEBOUNCE_US    30000u   // ignora rebotes del pulsador

/* ---------- Buzon: cola circular de eventos ----------
   La ISR escribe en "escribe". El loop principal lee de "lee".
   Cada uno mueve solo su propio indice, asi no se pisan. */
#define BUZON_TAM  16

static volatile enc_event_t buzon[BUZON_TAM];
static volatile uint8_t     escribe;
static volatile uint8_t     lee;
static volatile uint32_t    perdidos;

static void guardar_evento(enc_evt_t tipo, uint32_t t_us) {
    uint8_t siguiente = (escribe + 1) % BUZON_TAM;
    if (siguiente == lee) {            // buzon lleno: se pierde el evento
        perdidos++;
        return;
    }
    buzon[escribe].type = tipo;
    buzon[escribe].t_us = t_us;
    escribe = siguiente;
}

bool enc_pop(enc_event_t *e) {
    if (lee == escribe) return false;  // buzon vacio
    e->type = buzon[lee].type;
    e->t_us = buzon[lee].t_us;
    lee = (lee + 1) % BUZON_TAM;
    return true;
}

bool enc_hay_eventos(void) { return lee != escribe; }

uint32_t enc_dropped(void) { return perdidos; }

/* ---------- Interrupcion: hace lo minimo ---------- */
static uint32_t t_subida_clk;     // ultima vez que CLK subio
static uint32_t t_borde_sw;       // ultimo flanco del pulsador (subida o bajada)

static void enc_isr(uint gpio, uint32_t eventos) {
    uint32_t ahora = time_us_32();

    if (gpio == ENC_PIN_CLK) {
        // Cada vez que CLK sube, anotamos la hora.
        if (eventos & GPIO_IRQ_EDGE_RISE) t_subida_clk = ahora;

        if (eventos & GPIO_IRQ_EDGE_FALL) {
            // Una bajada pegada a una subida es rebote, no un clic nuevo.
            if (ahora - t_subida_clk < ROT_REPOSO_US) return;

            // CLK acaba de bajar. Si DT sigue alto, el giro es horario.
            bool horario = (sio_hw->gpio_in & ENC_MASK_DT) != 0;
            if (ENC_INVERT) horario = !horario;
            guardar_evento(horario ? ENC_EVT_CW : ENC_EVT_CCW, ahora);
        }
    }
    else if (gpio == ENC_PIN_SW) {
        // Se mide desde el ULTIMO flanco (bajada o subida): tanto el rebote
        // al presionar como el rebote al soltar quedan dentro de la ventana.
        bool aceptar = (ahora - t_borde_sw >= SW_DEBOUNCE_US) && (eventos & GPIO_IRQ_EDGE_FALL);
        t_borde_sw = ahora;
        if (aceptar) guardar_evento(ENC_EVT_PRESS, ahora);
    }
}

/* ---------- Configuracion ---------- */
void enc_init(void) {
    gpio_init_mask(ENC_MASK_ALL);
    gpio_set_dir_in_masked(ENC_MASK_ALL);   // los tres pines son entradas
    gpio_pull_up(ENC_PIN_CLK);
    gpio_pull_up(ENC_PIN_DT);
    gpio_pull_up(ENC_PIN_SW);

    t_subida_clk   = time_us_32();
    t_borde_sw     = time_us_32() - SW_DEBOUNCE_US;

    // Una sola funcion atiende los dos pines (el SDK comparte el callback)
    uint32_t ambos_flancos = GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE;
    gpio_set_irq_enabled_with_callback(ENC_PIN_CLK, ambos_flancos, true, enc_isr);
    gpio_set_irq_enabled(ENC_PIN_SW, ambos_flancos, true);
}
