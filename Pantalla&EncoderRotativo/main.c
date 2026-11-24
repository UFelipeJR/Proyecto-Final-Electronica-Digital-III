#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/structs/sio.h"
#include "ili9488_hw.h"
#include "ili9488_init.h"
#include "ili9488_draw.h"
#include "encoder.h"
#include "menu.h"
#include "tick.h"

#define MASK_LED_OB  (1u << PICO_DEFAULT_LED_PIN)

// true si todavia hay algo por hacer: la pantalla arrancando o dibujando,
// un evento del encoder sin atender, o el menu sin terminar de pintar.
static bool hay_trabajo(void) {
    return !lcd_ready() || !lcd_draw_idle() || enc_hay_eventos() || menu_ocupado();
}

int main(void) {
    stdio_init_all();
    gpio_init_mask(MASK_LED_OB);
    gpio_set_dir_out_masked(MASK_LED_OB);

    lcd_hw_init();
    lcd_init_start();
    enc_init();
    tick_init();

    while (true) {
        lcd_task();            // inicializacion de la pantalla (FSM)
        lcd_draw_task();       // envio de la cola de dibujo (FSM)
        menu_step();           // eventos del encoder -> dibujo

        if (lcd_ready()) sio_hw->gpio_set = MASK_LED_OB;   // LED de la placa: pantalla lista

        // Sin trabajo pendiente, el microcontrolador duerme hasta la proxima
        // interrupcion (encoder o temporizador). Las interrupciones se bloquean
        // un instante para que un evento no llegue entre la revision y el sueno:
        // si llega, queda pendiente y el __wfi() despierta de inmediato.
        uint32_t irq = save_and_disable_interrupts();
        if (!hay_trabajo()) {
            __wfi();
        }
        restore_interrupts(irq);
    }
}
