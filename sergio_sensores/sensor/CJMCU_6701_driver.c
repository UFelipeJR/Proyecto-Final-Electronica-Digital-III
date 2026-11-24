#include "CJMCU_6701_driver.h"
#include "hardware/gpio.h"
#include "hardware/sync.h" 



static bool cjmcu6701_timer_isr_callback(struct repeating_timer *t) {
    
    cjmcu6701_it_t *dev = (cjmcu6701_it_t *)t->user_data;

    uint8_t rx_buf[2];

    // Inicia conversión bajando CS
    gpio_put(dev->pin_cs, 0);

    //Lectura rápida por hardware SPI (toma ~20us a 800kHz)
    spi_read_blocking(dev->spi, 0x00, rx_buf, 2);

    //Terminar conversión subiendo CS
    gpio_put(dev->pin_cs, 1);

    // Extraer los 12 bits del MCP3201
    uint16_t raw_frame = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
    dev->latest_raw = (raw_frame >> 1) & 0x0FFF;

    // Verificar si el main loop aún no había procesado la muestra anterior
    if (dev->data_ready_flag) {
        dev->sample_overrun_count++;
    }

    //Avisar al main loop que hay un dato fresco
    dev->data_ready_flag = true;

    return true; // Retornar true mantiene el timer repitiéndose
}


void cjmcu6701_init_it(cjmcu6701_it_t *dev, spi_inst_t *spi, uint8_t cs, uint8_t sck, uint8_t miso, float vref) {
    dev->spi = spi;
    dev->pin_cs = cs;
    dev->pin_sck = sck;
    dev->pin_miso = miso;
    dev->vref = vref;

    dev->latest_raw = 0;
    dev->data_ready_flag = false;
    dev->sample_overrun_count = 0;
    dev->buf_index = 0;
    dev->sum = 0;
    dev->is_primed = false;


    spi_init(dev->spi, 800 * 1000);
    spi_set_format(dev->spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(dev->pin_sck, GPIO_FUNC_SPI);
    gpio_set_function(dev->pin_miso, GPIO_FUNC_SPI);


    gpio_init(dev->pin_cs);
    gpio_set_dir(dev->pin_cs, GPIO_OUT);
    gpio_put(dev->pin_cs, 1);
}

bool cjmcu6701_start_sampling_it(cjmcu6701_it_t *dev, uint16_t sample_rate_hz) {
    if (sample_rate_hz == 0) return false;

    // Calcular periodo en microsegundos (negativo para que el SDK cuente desde el inicio de la ISR)
    int64_t period_us = -(1000000 / sample_rate_hz);

    // Registrar la interrupción del timer pasando 'dev' como contexto
    return add_repeating_timer_us(period_us, cjmcu6701_timer_isr_callback, dev, &dev->sample_timer);
}

void cjmcu6701_stop_sampling_it(cjmcu6701_it_t *dev) {
    cancel_repeating_timer(&dev->sample_timer);
    dev->data_ready_flag = false;
}








bool cjmcu6701_is_data_ready(cjmcu6701_it_t *dev) {
    return dev->data_ready_flag;
}

uint16_t cjmcu6701_process_and_get_filtered(cjmcu6701_it_t *dev, uint16_t *out_raw) {
    
    // Deshabilitamos interrupciones un instante (< 0.5us) para copiar el dato y bajar la bandera
    // de forma atómica, evitando que la ISR se dispare justo a mitad de esta lectura.
    uint32_t status = save_and_disable_interrupts();
    uint16_t sample = dev->latest_raw;
    dev->data_ready_flag = false;
    restore_interrupts(status);
    

    if (out_raw != NULL) {
        *out_raw = sample;
    }

    // Procesamiento del filtro de media móvil 
    if (!dev->is_primed) {
        for (int i = 0; i < GSR_FILTER_WINDOW; i++) {
            dev->filter_buf[i] = sample;
        }
        dev->sum = (uint32_t)sample * GSR_FILTER_WINDOW;
        dev->is_primed = true;
        return sample;
    }

    dev->sum -= dev->filter_buf[dev->buf_index];
    dev->filter_buf[dev->buf_index] = sample;
    dev->sum += sample;
    dev->buf_index = (dev->buf_index + 1) % GSR_FILTER_WINDOW;

    return (uint16_t)(dev->sum / GSR_FILTER_WINDOW);
}

float cjmcu6701_raw_to_voltage(cjmcu6701_it_t *dev, uint16_t adc_val) {
    return ((float)adc_val * dev->vref) / 4095.0f;
}