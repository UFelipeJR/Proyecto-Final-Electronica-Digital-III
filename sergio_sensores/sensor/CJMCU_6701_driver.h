#ifndef CJMCU6701_IT_H
#define CJMCU6701_IT_H

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// Ventana del filtro (12 muestras a 60Hz = Filtro Notch exacto a 60Hz de red eléctrica)
#define GSR_FILTER_WINDOW 12

typedef struct {
    spi_inst_t *spi;
    uint8_t pin_cs;
    uint8_t pin_sck;
    uint8_t pin_miso;
    float vref;


    volatile uint16_t latest_raw;
    volatile bool data_ready_flag;
    volatile uint32_t sample_overrun_count;

    uint16_t filter_buf[GSR_FILTER_WINDOW];
    uint8_t buf_index;
    uint32_t sum;
    bool is_primed;


    struct repeating_timer sample_timer;
} cjmcu6701_it_t;


void cjmcu6701_init_it(cjmcu6701_it_t *dev, spi_inst_t *spi, uint8_t cs, uint8_t sck, uint8_t miso, float vref);
//Inicializa el hardware







bool cjmcu6701_start_sampling_it(cjmcu6701_it_t *dev, uint16_t sample_rate_hz);
// Activa la generación de interrupciones de muestreo a la frecuencia dada (Hz)

void cjmcu6701_stop_sampling_it(cjmcu6701_it_t *dev);
// Detiene la generación de interrupciones de muestreo a la frecuencia dada (Hz)


// Funciones para usar por POLLING en el bucle principal
bool cjmcu6701_is_data_ready(cjmcu6701_it_t *dev);
uint16_t cjmcu6701_process_and_get_filtered(cjmcu6701_it_t *dev, uint16_t *out_raw);
float cjmcu6701_raw_to_voltage(cjmcu6701_it_t *dev, uint16_t adc_val);

#endif // CJMCU6701_IT_H