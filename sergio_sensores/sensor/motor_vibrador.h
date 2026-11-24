#ifndef MOTOR_VIBRADOR_H
#define MOTOR_VIBRADOR_H

#include <stdint.h>
#include <stdbool.h>
#include "pico/time.h"

/** Duración del pulso inicial al 100% para vencer la fricción estática (ms). */
#define VIBRADOR_KICKSTART_MS 25U

/** Estados internos de la máquina de estados del motor. */
typedef enum {
    VIB_APAGADO = 0,
    VIB_KICKSTART,
    VIB_ACTIVO
} vibrador_estado_t;

/** Descriptor del periférico motor vibrador. */
typedef struct {
    uint gpio_pin;
    uint slice_num;
    uint channel;
    uint16_t wrap;
    vibrador_estado_t estado;
    uint8_t intensidad_objetivo;
    bool tiene_temporizador;
    absolute_time_t t_fin_kickstart;
    absolute_time_t t_fin_pulso;
} vibrador_t;

/**
 * @brief Inicializa el GPIO en modo PWM y configura el slice del RP2040.
 *
 * @param dev      Puntero a la instancia del motor.
 * @param gpio_pin Número de pin GPIO conectado a la resistencia de base del BJT.
 * @param freq_hz  Frecuencia de PWM deseada en Hz (recomendado: 25000 Hz para
 *                 evitar zumbido audible, o entre 500 y 1000 Hz).
 */
void vibrador_init(vibrador_t *dev, uint gpio_pin, uint32_t freq_hz);

/**
 * @brief Fija una intensidad continua de vibración.
 *
 * @param dev            Puntero a la instancia del motor.
 * @param porcentaje     Ciclo de trabajo de 0 a 100%.
 * @param usar_kickstart Si es true y el motor estaba detenido, aplica 100%
 *                       durante VIBRADOR_KICKSTART_MS antes de bajar al objetivo.
 */
void vibrador_set_intensidad(vibrador_t *dev, uint8_t porcentaje, bool usar_kickstart);

/**
 * @brief Dispara un pulso de vibración no bloqueante con apagado automático.
 *
 * @param dev         Puntero a la instancia del motor.
 * @param porcentaje  Intensidad objetivo (1 a 100%).
 * @param duracion_ms Tiempo total de vibración en milisegundos.
 */
void vibrador_pulso_ms(vibrador_t *dev, uint8_t porcentaje, uint32_t duracion_ms);

/**
 * @brief Detiene inmediatamente el motor (duty cycle = 0%).
 *
 * @param dev Puntero a la instancia del motor.
 */
void vibrador_apagar(vibrador_t *dev);

/**
 * @brief Actualiza los temporizadores de kickstart y pulsos (llamar en el bucle principal).
 *
 * @param dev Puntero a la instancia del motor.
 */
void vibrador_update(vibrador_t *dev);

#endif /* MOTOR_VIBRADOR_H */