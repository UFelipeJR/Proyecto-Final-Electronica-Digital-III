#include "pico/stdlib.h"
#include "motor_vibrador.h"

#define PIN_MOTOR_VIB 17U

int main(void) {
    stdio_init_all();

    vibrador_t motor;
    /* 25 kHz elimina el pitido audible en las bobinas del motor */
    vibrador_init(&motor, PIN_MOTOR_VIB, 25000U);

    absolute_time_t siguiente_evento = make_timeout_time_ms(2000U);

    while (true) {
        /* Debe ejecutarse continuamente en el superloop sin demoras bloqueantes */
        vibrador_update(&motor);

        /* Ejemplo: dispara un pulso de 300 ms al 60% cada 2 segundos */
        if (time_reached(siguiente_evento)) {
            vibrador_pulso_ms(&motor, 60U, 300U);
            siguiente_evento = make_timeout_time_ms(2000U);
        }

        /* Aquí pueden correr el resto de tus sensores y tareas */
    }
}