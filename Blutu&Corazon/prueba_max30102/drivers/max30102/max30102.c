
#include "max30102.h"

#include <stddef.h>
#include "hardware/i2c.h"
#include "pico/time.h"

#define MAX30102_I2C i2c0
#define MAX30102_ADDR 0x57
#define I2C_TIMEOUT_US 10000
#define RESET_TIMEOUT_MS 100

#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

#define PART_ID_ESPERADO 0x15
#define BIT_RESET 0x40

typedef struct
{
    uint8_t registro;
    uint8_t valor;
} max30102_registro_config_t;

static max30102_diagnostico_t ultimo_diagnostico;

static void limpiar_diagnostico(void)
{
    ultimo_diagnostico.tipo = MAX30102_ERROR_NINGUNO;
    ultimo_diagnostico.registro = 0;
    ultimo_diagnostico.esperado = 0;
    ultimo_diagnostico.obtenido = 0;
}

static void registrar_error(max30102_tipo_error_t tipo, uint8_t registro, uint8_t esperado, uint8_t obtenido)
{
    ultimo_diagnostico.tipo = tipo;
    ultimo_diagnostico.registro = registro;
    ultimo_diagnostico.esperado = esperado;
    ultimo_diagnostico.obtenido = obtenido;
}

// Leer un registro interno del MAX30102
static bool leer_registro(uint8_t registro, uint8_t *valor)
{
    if (valor == NULL)
    {
        registrar_error(MAX30102_ERROR_PARAMETRO, registro, 0, 0);
        return false;
    }

    int resultado = i2c_write_timeout_us(MAX30102_I2C, MAX30102_ADDR, &registro, 1, true, I2C_TIMEOUT_US);

    if (resultado != 1)
    {
        registrar_error(MAX30102_ERROR_LECTURA_I2C, registro, 0, 0);
        return false;
    }

    resultado = i2c_read_timeout_us(MAX30102_I2C, MAX30102_ADDR, valor, 1, false, I2C_TIMEOUT_US);

    if (resultado != 1)
    {
        registrar_error(MAX30102_ERROR_LECTURA_I2C, registro, 0, 0);
        return false;
    }

    return true;
}

// Escribir un registro interno del MAX30102
static bool escribir_registro(uint8_t registro, uint8_t valor)
{
    uint8_t datos[2] = {registro, valor};
    int resultado = i2c_write_timeout_us(MAX30102_I2C, MAX30102_ADDR, datos, 2, false, I2C_TIMEOUT_US);

    if (resultado != 2)
    {
        registrar_error(MAX30102_ERROR_ESCRITURA_I2C, registro, valor, 0);
        return false;
    }

    return true;
}

static bool verificar_registro(uint8_t registro, uint8_t esperado)
{
    uint8_t obtenido = 0;

    if (!leer_registro(registro, &obtenido))
    {
        return false;
    }

    if (obtenido != esperado)
    {
        registrar_error(MAX30102_ERROR_VERIFICACION, registro, esperado, obtenido);
        return false;
    }

    return true;
}

// Obtener identificacion y revision del sensor
bool max30102_leer_identificacion(uint8_t *part_id, uint8_t *rev_id)
{
    limpiar_diagnostico();

    if (part_id == NULL || rev_id == NULL)
    {
        registrar_error(MAX30102_ERROR_PARAMETRO, 0, 0, 0);
        return false;
    }

    if (!leer_registro(REG_PART_ID, part_id))
    {
        return false;
    }

    if (!leer_registro(REG_REV_ID, rev_id))
    {
        return false;
    }

    return true;
}

bool max30102_configurar(void)
{
    static const max30102_registro_config_t registros[] = {
        {REG_FIFO_CONFIG, 0x00},
        {REG_FIFO_WR_PTR, 0x00},
        {REG_OVF_COUNTER, 0x00},
        {REG_FIFO_RD_PTR, 0x00},
        {REG_SPO2_CONFIG, 0x27},
        {REG_LED1_PA, 0x1F},
        {REG_LED2_PA, 0x1F},
        {REG_MODE_CONFIG, 0x83}
    };

    uint8_t part_id = 0;
    uint8_t rev_id = 0;

    limpiar_diagnostico();

    if (!max30102_leer_identificacion(&part_id, &rev_id))
    {
        return false;
    }

    if (part_id != PART_ID_ESPERADO)
    {
        registrar_error(MAX30102_ERROR_IDENTIFICACION, REG_PART_ID, PART_ID_ESPERADO, part_id);
        return false;
    }

    if (!escribir_registro(REG_MODE_CONFIG, BIT_RESET))
    {
        return false;
    }

    absolute_time_t limite_reset = make_timeout_time_ms(RESET_TIMEOUT_MS);
    uint8_t mode_config = BIT_RESET;

    while (!time_reached(limite_reset))
    {
        if (!leer_registro(REG_MODE_CONFIG, &mode_config))
        {
            return false;
        }

        if ((mode_config & BIT_RESET) == 0)
        {
            break;
        }
    }

    if ((mode_config & BIT_RESET) != 0)
    {
        registrar_error(MAX30102_ERROR_TIMEOUT_RESET, REG_MODE_CONFIG, 0x00, mode_config);
        return false;
    }

    for (size_t i = 0; i < sizeof(registros) / sizeof(registros[0]); i++)
    {
        if (!escribir_registro(registros[i].registro, registros[i].valor))
        {
            return false;
        }
    }

    for (size_t i = 0; i < sizeof(registros) / sizeof(registros[0]); i++)
    {
        if (!verificar_registro(registros[i].registro, registros[i].valor))
        {
            return false;
        }
    }

    return true;
}

bool max30102_iniciar_medicion(void)
{
    limpiar_diagnostico();

    if (!escribir_registro(REG_FIFO_WR_PTR, 0x00) ||
        !escribir_registro(REG_OVF_COUNTER, 0x00) ||
        !escribir_registro(REG_FIFO_RD_PTR, 0x00) ||
        !verificar_registro(REG_FIFO_WR_PTR, 0x00) ||
        !verificar_registro(REG_OVF_COUNTER, 0x00) ||
        !verificar_registro(REG_FIFO_RD_PTR, 0x00) ||
        !escribir_registro(REG_MODE_CONFIG, 0x03))
    {
        return false;
    }

    return verificar_registro(REG_MODE_CONFIG, 0x03);
}

bool max30102_leer_muestra(uint32_t *red, uint32_t *ir)
{
    uint8_t puntero_escritura = 0;
    uint8_t puntero_lectura = 0;
    uint8_t desbordamientos = 0;
    uint8_t registro = REG_FIFO_DATA;
    uint8_t datos[6];

    limpiar_diagnostico();

    if (red == NULL || ir == NULL)
    {
        registrar_error(MAX30102_ERROR_PARAMETRO, REG_FIFO_DATA, 0, 0);
        return false;
    }

    if (!leer_registro(REG_FIFO_WR_PTR, &puntero_escritura) ||
        !leer_registro(REG_FIFO_RD_PTR, &puntero_lectura) ||
        !leer_registro(REG_OVF_COUNTER, &desbordamientos))
    {
        return false;
    }

    if (desbordamientos != 0)
    {
        // Detener brevemente la conversion antes de descartar la FIFO llena.
        if (!escribir_registro(REG_MODE_CONFIG, 0x83) || !max30102_iniciar_medicion())
        {
            return false;
        }

        registrar_error(MAX30102_ERROR_DESBORDAMIENTO, REG_OVF_COUNTER, 0, desbordamientos);
        return false;
    }

    if (puntero_escritura == puntero_lectura)
    {
        return false;
    }

    if (i2c_write_timeout_us(MAX30102_I2C, MAX30102_ADDR, &registro, 1, true, I2C_TIMEOUT_US) != 1 ||
        i2c_read_timeout_us(MAX30102_I2C, MAX30102_ADDR, datos, 6, false, I2C_TIMEOUT_US) != 6)
    {
        registrar_error(MAX30102_ERROR_LECTURA_I2C, REG_FIFO_DATA, 0, 0);
        return false;
    }

    *red = (((uint32_t)datos[0] << 16) | ((uint32_t)datos[1] << 8) | datos[2]) & 0x3FFFF;
    *ir = (((uint32_t)datos[3] << 16) | ((uint32_t)datos[4] << 8) | datos[5]) & 0x3FFFF;

    return true;
}

bool max30102_leer_estado(uint8_t *wr_ptr, uint8_t *rd_ptr, uint8_t *ovf_counter, uint8_t *mode_config)
{
    limpiar_diagnostico();

    if (wr_ptr == NULL || rd_ptr == NULL || ovf_counter == NULL || mode_config == NULL)
    {
        registrar_error(MAX30102_ERROR_PARAMETRO, 0, 0, 0);
        return false;
    }

    return leer_registro(REG_FIFO_WR_PTR, wr_ptr) &&
           leer_registro(REG_FIFO_RD_PTR, rd_ptr) &&
           leer_registro(REG_OVF_COUNTER, ovf_counter) &&
           leer_registro(REG_MODE_CONFIG, mode_config);
}

bool max30102_leer_configuracion(max30102_configuracion_t *configuracion)
{
    limpiar_diagnostico();

    if (configuracion == NULL)
    {
        registrar_error(MAX30102_ERROR_PARAMETRO, 0, 0, 0);
        return false;
    }

    if (!leer_registro(REG_FIFO_CONFIG, &configuracion->fifo_config) ||
        !leer_registro(REG_MODE_CONFIG, &configuracion->mode_config) ||
        !leer_registro(REG_SPO2_CONFIG, &configuracion->spo2_config) ||
        !leer_registro(REG_LED1_PA, &configuracion->led1_pa) ||
        !leer_registro(REG_LED2_PA, &configuracion->led2_pa))
    {
        return false;
    }

    return true;
}

void max30102_obtener_diagnostico(max30102_diagnostico_t *diagnostico)
{
    if (diagnostico != NULL)
    {
        *diagnostico = ultimo_diagnostico;
    }
}
