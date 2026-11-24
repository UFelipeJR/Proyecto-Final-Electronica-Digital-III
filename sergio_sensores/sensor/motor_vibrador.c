#include "motor_vibrador.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

/** Aplica el ciclo de trabajo (0-100%) directamente al canal PWM. */
static void aplicar_pwm(const vibrador_t *dev, uint8_t porcentaje) {
    if (porcentaje > 100U) {
        porcentaje = 100U;
    }
    /* En RP2040 el periodo es (wrap + 1) ciclos; nivel = wrap + 1 equivale a 100% DC */
    uint16_t nivel = (uint16_t)(((uint32_t)(dev->wrap + 1U) * porcentaje) / 100U);
    pwm_set_chan_level(dev->slice_num, dev->channel, nivel);
}

void vibrador_init(vibrador_t *dev, uint gpio_pin, uint32_t freq_hz) {
    if (freq_hz == 0U) {
        freq_hz = 25000U;
    }

    dev->gpio_pin = gpio_pin;
    dev->slice_num = pwm_gpio_to_slice_num(gpio_pin);
    dev->channel = pwm_gpio_to_channel(gpio_pin);
    dev->estado = VIB_APAGADO;
    dev->intensidad_objetivo = 0U;
    dev->tiene_temporizador = false;

    /* Configurar pin en modo PWM y aumentar corriente de salida para saturar el BJT */
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    gpio_set_drive_strength(gpio_pin, GPIO_DRIVE_STRENGTH_8MA);

    /* Cálculo dinámico de divisor y TOP (wrap) para soportar desde ~200 Hz hasta >25 kHz */
    uint32_t sys_clk = clock_get_hz(clk_sys);
    uint32_t wrap = 999U; /* Resolución base de 1000 pasos (0.1%) */
    float clkdiv = (float)sys_clk / ((float)freq_hz * (float)(wrap + 1U));

    if (clkdiv > 255.0f) {
        clkdiv = 255.0f;
        uint32_t cuentas = (uint32_t)((float)sys_clk / (clkdiv * (float)freq_hz));
        wrap = (cuentas > 1U) ? (cuentas - 1U) : 1U;
    } else if (clkdiv < 1.0f) {
        clkdiv = 1.0f;
        uint32_t cuentas = (uint32_t)((float)sys_clk / (float)freq_hz);
        wrap = (cuentas > 1U) ? (cuentas - 1U) : 1U;
    }

    if (wrap > 65534U) {
        wrap = 65534U;
    }

    dev->wrap = (uint16_t)wrap;

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, clkdiv);
    pwm_config_set_wrap(&cfg, dev->wrap);

    pwm_init(dev->slice_num, &cfg, true);
    aplicar_pwm(dev, 0U);
}

void vibrador_apagar(vibrador_t *dev) {
    aplicar_pwm(dev, 0U);
    dev->estado = VIB_APAGADO;
    dev->intensidad_objetivo = 0U;
    dev->tiene_temporizador = false;
}

void vibrador_set_intensidad(vibrador_t *dev, uint8_t porcentaje, bool usar_kickstart) {
    if (porcentaje == 0U) {
        vibrador_apagar(dev);
        return;
    }
    if (porcentaje > 100U) {
        porcentaje = 100U;
    }

    dev->tiene_temporizador = false;
    dev->intensidad_objetivo = porcentaje;

    if (usar_kickstart && (dev->estado == VIB_APAGADO) && (porcentaje < 100U)) {
        aplicar_pwm(dev, 100U);
        dev->t_fin_kickstart = make_timeout_time_ms(VIBRADOR_KICKSTART_MS);
        dev->estado = VIB_KICKSTART;
    } else {
        aplicar_pwm(dev, porcentaje);
        dev->estado = VIB_ACTIVO;
    }
}

void vibrador_pulso_ms(vibrador_t *dev, uint8_t porcentaje, uint32_t duracion_ms) {
    if (porcentaje == 0U || duracion_ms == 0U) {
        vibrador_apagar(dev);
        return;
    }

    vibrador_set_intensidad(dev, porcentaje, true);
    dev->t_fin_pulso = make_timeout_time_ms(duracion_ms);
    dev->tiene_temporizador = true;
}

void vibrador_update(vibrador_t *dev) {
    if (dev->estado == VIB_APAGADO) {
        return;
    }

    /* Prioridad al fin de pulso: garantiza apagado exacto incluso en pulsos < 25 ms */
    if (dev->tiene_temporizador && time_reached(dev->t_fin_pulso)) {
        vibrador_apagar(dev);
        return;
    }

    if (dev->estado == VIB_KICKSTART && time_reached(dev->t_fin_kickstart)) {
        aplicar_pwm(dev, dev->intensidad_objetivo);
        dev->estado = VIB_ACTIVO;
    }
}