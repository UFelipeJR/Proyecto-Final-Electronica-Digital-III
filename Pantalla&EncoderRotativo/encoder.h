#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>
#include <stdint.h>

#define ENC_PIN_CLK  10
#define ENC_PIN_DT   11
#define ENC_PIN_SW   12

typedef enum {
    ENC_EVT_CW,      // un clic hacia la derecha
    ENC_EVT_CCW,     // un clic hacia la izquierda
    ENC_EVT_PRESS    // pulsador presionado
} enc_evt_t;

typedef struct {
    enc_evt_t type;
    uint32_t  t_us;  // hora en que ocurrio (para medir la latencia despues)
} enc_event_t;

void     enc_init(void);
bool     enc_pop(enc_event_t *e);   // saca un evento del buzon; false si esta vacio
bool     enc_hay_eventos(void);     // true si el buzon tiene eventos sin atender
uint32_t enc_dropped(void);         // eventos perdidos por buzon lleno

#endif /* ENCODER_H */
