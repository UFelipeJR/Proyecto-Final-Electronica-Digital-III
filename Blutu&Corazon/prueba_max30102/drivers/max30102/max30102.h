
#ifndef MAX30102_H
#define MAX30102_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    MAX30102_ERROR_NINGUNO = 0,
    MAX30102_ERROR_PARAMETRO,
    MAX30102_ERROR_LECTURA_I2C,
    MAX30102_ERROR_ESCRITURA_I2C,
    MAX30102_ERROR_IDENTIFICACION,
    MAX30102_ERROR_TIMEOUT_RESET,
    MAX30102_ERROR_VERIFICACION,
    MAX30102_ERROR_DESBORDAMIENTO
} max30102_tipo_error_t;

typedef struct
{
    max30102_tipo_error_t tipo;
    uint8_t registro;
    uint8_t esperado;
    uint8_t obtenido;
} max30102_diagnostico_t;

typedef struct
{
    uint8_t fifo_config;
    uint8_t mode_config;
    uint8_t spo2_config;
    uint8_t led1_pa;
    uint8_t led2_pa;
} max30102_configuracion_t;

// Leer los registros de identificacion del sensor
bool max30102_leer_identificacion(uint8_t *part_id, uint8_t *rev_id);

// Configurar y verificar el sensor en modo SpO2, manteniendolo en reposo
bool max30102_configurar(void);

// Vaciar la FIFO y activar los LED rojo e infrarrojo
bool max30102_iniciar_medicion(void);

// Leer una muestra RED e IR; false con error NINGUNO significa FIFO vacia
// Un desbordamiento descarta la FIFO y se informa con ERROR_DESBORDAMIENTO
bool max30102_leer_muestra(uint32_t *red, uint32_t *ir);

// Consultar los punteros FIFO, desbordamiento y modo para diagnostico
bool max30102_leer_estado(uint8_t *wr_ptr, uint8_t *rd_ptr, uint8_t *ovf_counter, uint8_t *mode_config);

// Leer los registros principales de configuracion
bool max30102_leer_configuracion(max30102_configuracion_t *configuracion);

// Obtener informacion del ultimo error del driver
void max30102_obtener_diagnostico(max30102_diagnostico_t *diagnostico);

#endif
