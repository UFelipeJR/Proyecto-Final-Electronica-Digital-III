
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "max30102.h"

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define UMBRAL_DEDO_IR 20000u  // Sin dedo ~1000; con dedo ~109000
#define RANGO_AC_MINIMO 150  // Variacion minima para considerar una senal pulsatil
#define RANGO_AC_MAXIMO 4000 // Variaciones mayores suelen indicar movimiento
#define VENTANA_AC_MS 2000u
#define ESTABILIZACION_MS 2000u
#define INTERVALO_MIN_MS 450u
#define INTERVALO_MAX_MS 1500u
#define SIN_LATIDO_MS 1800u
#define HUECO_MUESTRAS_MS 250u
#define DURACION_MIN_VALLE_MS 30u
#define TOLERANCIA_INTERVALO_PCT 30u
#define VENTANA_R_MS 5000u
#define SPO2_INTERCEPTO 110.0
#define SPO2_PENDIENTE 25.0

static void mostrar_error(const max30102_diagnostico_t *diagnostico)
{
    switch (diagnostico->tipo)
    {
        case MAX30102_ERROR_PARAMETRO:
            printf("# Error: parametro no valido\n");
            break;

        case MAX30102_ERROR_LECTURA_I2C:
            printf("# Error de lectura I2C en el registro 0x%02X\n", diagnostico->registro);
            break;

        case MAX30102_ERROR_ESCRITURA_I2C:
            printf("# Error de escritura I2C en el registro 0x%02X\n", diagnostico->registro);
            break;

        case MAX30102_ERROR_IDENTIFICACION:
            printf("# Error de identificacion en el registro 0x%02X: esperado 0x%02X, obtenido 0x%02X\n", diagnostico->registro, diagnostico->esperado, diagnostico->obtenido);
            break;

        case MAX30102_ERROR_TIMEOUT_RESET:
            printf("# Error: tiempo agotado esperando el reset del MAX30102\n");
            break;

        case MAX30102_ERROR_VERIFICACION:
            printf("# Error de verificacion en el registro 0x%02X: esperado 0x%02X, obtenido 0x%02X\n", diagnostico->registro, diagnostico->esperado, diagnostico->obtenido);
            break;

        case MAX30102_ERROR_DESBORDAMIENTO:
            printf("# FIFO desbordada (OVF: 0x%02X); contenido descartado y medicion reiniciada\n", diagnostico->obtenido);
            break;

        default:
            printf("# Error desconocido del MAX30102\n");
            break;
    }
}

int main(void)
{
    stdio_init_all();
    printf("# ENTRADA EN MAIN: arranque de la Pico\n");

    // Inicializar comunicacion I2C
    i2c_init(i2c0, 100000);

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    max30102_diagnostico_t diagnostico = {0};
    max30102_diagnostico_t diagnostico_lectura = {0};
    bool configurado = max30102_configurar();
    bool medicion_iniciada = false;

    if (configurado)
    {
        medicion_iniciada = max30102_iniciar_medicion();
    }

    if (!configurado || !medicion_iniciada)
    {
        max30102_obtener_diagnostico(&diagnostico);
        mostrar_error(&diagnostico);
    }
    else
    {
        printf("# MAX30102 configurado correctamente\n");
        printf("# Medicion iniciada\n");
    }

    uint32_t red = 0;
    uint32_t ir = 0;
    uint32_t muestras = 0;
    uint32_t errores_i2c = 0;
    uint32_t recuperaciones = 0;
    uint32_t intervalos[4] = {0};
    uint32_t suma_intervalos = 0;
    uint32_t ultimo_latido = 0;
    uint32_t tiempo_dedo = 0;
    uint32_t tiempo_ultima_muestra = 0;
    uint32_t inicio_ventana_r = 0;
    uint32_t estable_r_desde = 0;
    uint32_t muestras_ventana_r = 0;
    uint64_t suma_red_ac2 = 0;
    uint64_t suma_ir_ac2 = 0;
    uint64_t suma_red_dc = 0;
    uint64_t suma_ir_dc = 0;
    uint32_t tiempo_minimo = 0;
    uint32_t inicio_valle = 0;
    uint32_t inicio_ventana_ac = 0;
    uint8_t indice_intervalo = 0;
    uint8_t cantidad_intervalos = 0;
    int32_t minimo_ac = 0;
    int32_t ir_ac = 0;
    int32_t red_ac = 0;
    int32_t ventana_min_ac = 0;
    int32_t ventana_max_ac = 0;
    int32_t rango_ac = 0;
    int32_t umbral_valle = 0;
    int32_t umbral_rearme = 0;
    float ir_dc = 0.0f;
    float red_dc = 0.0f;
    float red_dc_inicio_r = 0.0f;
    float ir_dc_inicio_r = 0.0f;
    double red_dc_ventana = 0.0;
    double ir_dc_ventana = 0.0;
    double red_rms = 0.0;
    double ir_rms = 0.0;
    double relacion_r = 0.0;
    bool r_valida = false;
    bool resultado_r_pendiente = false;
    const char *razon_r = "ventana incompleta";
    bool filtro_inicializado = false;
    bool dedo_presente = false;
    bool detector_armado = false;
    bool en_valle = false;
    bool referencia_lista = false;
    bool error_lectura = false;
    uint32_t inicio = to_ms_since_boot(get_absolute_time());
    uint32_t tiempo_fifo = inicio - 5;
    uint32_t tiempo_reporte = inicio - 1000;
    uint32_t tiempo_estado = inicio - 5000;

    while (true)
    {
        uint32_t tiempo_actual = to_ms_since_boot(get_absolute_time());

        if (medicion_iniciada && (uint32_t)(tiempo_actual - tiempo_fifo) >= 5)
        {
            tiempo_fifo = tiempo_actual;

            // Hasta ocho muestras por vuelta, sin esperar a que llegue otra.
            for (int i = 0; i < 8; i++)
            {
                if (max30102_leer_muestra(&red, &ir))
                {
                    muestras++;
                    uint32_t tiempo_muestra = to_ms_since_boot(get_absolute_time());

                    if (tiempo_ultima_muestra != 0 && (uint32_t)(tiempo_muestra - tiempo_ultima_muestra) > HUECO_MUESTRAS_MS)
                    {
                        filtro_inicializado = false;
                        estable_r_desde = tiempo_muestra;
                        inicio_ventana_r = 0;
                        muestras_ventana_r = 0;
                        suma_red_ac2 = suma_ir_ac2 = suma_red_dc = suma_ir_dc = 0;
                        r_valida = false;
                        resultado_r_pendiente = false;
                        razon_r = "hueco de muestras";
                        tiempo_dedo = tiempo_muestra;
                        ultimo_latido = 0;
                        suma_intervalos = 0;
                        cantidad_intervalos = 0;
                        indice_intervalo = 0;
                        detector_armado = false;
                        en_valle = false;
                        inicio_ventana_ac = 0;
                        referencia_lista = false;
                        rango_ac = 0;
                    }
                    tiempo_ultima_muestra = tiempo_muestra;

                    bool dedo_ahora = ir >= UMBRAL_DEDO_IR;
                    if (dedo_ahora != dedo_presente)
                    {
                        dedo_presente = dedo_ahora;
                        estable_r_desde = tiempo_muestra;
                        inicio_ventana_r = 0;
                        muestras_ventana_r = 0;
                        suma_red_ac2 = suma_ir_ac2 = suma_red_dc = suma_ir_dc = 0;
                        r_valida = false;
                        resultado_r_pendiente = false;
                        razon_r = dedo_ahora ? "estabilizando dedo" : "sin dedo";
                        tiempo_dedo = tiempo_muestra;
                        ultimo_latido = 0;
                        suma_intervalos = 0;
                        cantidad_intervalos = 0;
                        indice_intervalo = 0;
                        detector_armado = false;
                        en_valle = false;
                        inicio_ventana_ac = 0;
                        referencia_lista = false;
                        rango_ac = 0;
                        filtro_inicializado = false;
                    }

                    if (!filtro_inicializado)
                    {
                        ir_dc = (float)ir;
                        red_dc = (float)red;
                        filtro_inicializado = true;
                    }
                    else
                    {
                        ir_dc += 0.01f * ((float)ir - ir_dc);
                        red_dc += 0.01f * ((float)red - red_dc);
                    }

                    ir_ac = (int32_t)ir - (int32_t)ir_dc;
                    red_ac = (int32_t)red - (int32_t)red_dc;

                    if (dedo_presente && (uint32_t)(tiempo_muestra - estable_r_desde) >= ESTABILIZACION_MS)
                    {
                        if (inicio_ventana_r == 0)
                        {
                            inicio_ventana_r = tiempo_muestra;
                            red_dc_inicio_r = red_dc;
                            ir_dc_inicio_r = ir_dc;
                            if (!resultado_r_pendiente) razon_r = "ventana incompleta";
                        }

                        suma_red_ac2 += (uint64_t)((int64_t)red_ac * red_ac);
                        suma_ir_ac2 += (uint64_t)((int64_t)ir_ac * ir_ac);
                        suma_red_dc += (uint32_t)red_dc;
                        suma_ir_dc += (uint32_t)ir_dc;
                        muestras_ventana_r++;

                        if ((uint32_t)(tiempo_muestra - inicio_ventana_r) >= VENTANA_R_MS)
                        {
                            red_dc_ventana = (double)suma_red_dc / muestras_ventana_r;
                            ir_dc_ventana = (double)suma_ir_dc / muestras_ventana_r;
                            red_rms = sqrt((double)suma_red_ac2 / muestras_ventana_r);
                            ir_rms = sqrt((double)suma_ir_ac2 / muestras_ventana_r);
                            r_valida = red_dc_ventana > 0.0 && ir_dc_ventana > 0.0 && red_rms > 0.0 && ir_rms > 0.0;

                            if (!r_valida)
                            {
                                razon_r = "AC o DC nulo";
                            }
                            else if (fabs((double)red_dc - red_dc_inicio_r) > red_rms || fabs((double)ir_dc - ir_dc_inicio_r) > ir_rms)
                            {
                                r_valida = false;
                                razon_r = "DC cambiante";
                            }
                            else
                            {
                                relacion_r = (red_rms / red_dc_ventana) / (ir_rms / ir_dc_ventana);
                                r_valida = isfinite(relacion_r);
                                razon_r = r_valida ? "valida" : "R no finita";
                            }

                            resultado_r_pendiente = true;

                            inicio_ventana_r = 0;
                            muestras_ventana_r = 0;
                            suma_red_ac2 = suma_ir_ac2 = suma_red_dc = suma_ir_dc = 0;
                        }
                    }

                    if (dedo_presente && (uint32_t)(tiempo_muestra - tiempo_dedo) >= ESTABILIZACION_MS)
                    {
                        if (inicio_ventana_ac == 0)
                        {
                            inicio_ventana_ac = tiempo_muestra;
                            ventana_min_ac = ir_ac;
                            ventana_max_ac = ir_ac;
                        }

                        if (ir_ac < ventana_min_ac) ventana_min_ac = ir_ac;
                        if (ir_ac > ventana_max_ac) ventana_max_ac = ir_ac;

                        if ((uint32_t)(tiempo_muestra - inicio_ventana_ac) >= VENTANA_AC_MS)
                        {
                            rango_ac = ventana_max_ac - ventana_min_ac;
                            referencia_lista = rango_ac >= RANGO_AC_MINIMO && rango_ac <= RANGO_AC_MAXIMO;

                            if (referencia_lista)
                            {
                                umbral_valle = ventana_min_ac + rango_ac / 3;
                                umbral_rearme = ventana_min_ac + 2 * rango_ac / 3;
                            }
                            else
                            {
                                ultimo_latido = 0;
                                suma_intervalos = 0;
                                cantidad_intervalos = 0;
                                indice_intervalo = 0;
                                detector_armado = false;
                                en_valle = false;
                            }

                            inicio_ventana_ac = tiempo_muestra;
                            ventana_min_ac = ir_ac;
                            ventana_max_ac = ir_ac;
                        }

                        if (ultimo_latido != 0 && (uint32_t)(tiempo_muestra - ultimo_latido) > SIN_LATIDO_MS)
                        {
                            ultimo_latido = 0;
                            suma_intervalos = 0;
                            cantidad_intervalos = 0;
                            indice_intervalo = 0;
                            detector_armado = false;
                            en_valle = false;
                        }

                        if (referencia_lista && en_valle)
                        {
                            if (ir_ac < minimo_ac)
                            {
                                minimo_ac = ir_ac;
                                tiempo_minimo = tiempo_muestra;
                            }

                            if ((uint32_t)(tiempo_muestra - inicio_valle) > INTERVALO_MAX_MS)
                            {
                                en_valle = false;
                                detector_armado = false;
                            }
                            else if (ir_ac >= umbral_rearme)
                            {
                                en_valle = false;
                                detector_armado = true;

                                if (minimo_ac >= umbral_valle - rango_ac && (uint32_t)(tiempo_muestra - inicio_valle) >= DURACION_MIN_VALLE_MS)
                                {
                                    if (ultimo_latido == 0)
                                    {
                                        ultimo_latido = tiempo_minimo;
                                    }
                                    else
                                    {
                                        uint32_t intervalo = tiempo_minimo - ultimo_latido;

                                        if (intervalo >= INTERVALO_MIN_MS && intervalo <= INTERVALO_MAX_MS)
                                        {
                                            uint32_t promedio = cantidad_intervalos > 0 ? suma_intervalos / cantidad_intervalos : intervalo;

                                            if (cantidad_intervalos >= 2 &&
                                                (intervalo * 100u < promedio * (100u - TOLERANCIA_INTERVALO_PCT) ||
                                                 intervalo * 100u > promedio * (100u + TOLERANCIA_INTERVALO_PCT)))
                                            {
                                                suma_intervalos = 0;
                                                cantidad_intervalos = 0;
                                                indice_intervalo = 0;
                                                ultimo_latido = tiempo_minimo;
                                            }
                                            else
                                            {
                                                if (cantidad_intervalos == 4)
                                                {
                                                    suma_intervalos -= intervalos[indice_intervalo];
                                                }
                                                else
                                                {
                                                    cantidad_intervalos++;
                                                }

                                                intervalos[indice_intervalo] = intervalo;
                                                suma_intervalos += intervalo;
                                                indice_intervalo = (indice_intervalo + 1) % 4;
                                                ultimo_latido = tiempo_minimo;
                                            }
                                        }
                                        else if (intervalo > INTERVALO_MAX_MS)
                                        {
                                            suma_intervalos = 0;
                                            cantidad_intervalos = 0;
                                            indice_intervalo = 0;
                                            ultimo_latido = tiempo_minimo;
                                        }
                                    }
                                }
                            }
                        }
                        else if (referencia_lista && ir_ac >= umbral_rearme)
                        {
                            detector_armado = true;
                        }
                        else if (referencia_lista && ir_ac <= umbral_valle && detector_armado)
                        {
                            detector_armado = false;
                            en_valle = true;
                            inicio_valle = tiempo_muestra;
                            tiempo_minimo = tiempo_muestra;
                            minimo_ac = ir_ac;
                        }
                    }

                    continue;
                }

                max30102_obtener_diagnostico(&diagnostico);
                if (diagnostico.tipo != MAX30102_ERROR_NINGUNO)
                {
                    if (diagnostico.tipo == MAX30102_ERROR_DESBORDAMIENTO)
                    {
                        recuperaciones++;
                        filtro_inicializado = false;
                        tiempo_ultima_muestra = 0;
                        tiempo_dedo = to_ms_since_boot(get_absolute_time());
                        ultimo_latido = 0;
                        suma_intervalos = 0;
                        cantidad_intervalos = 0;
                        indice_intervalo = 0;
                        detector_armado = false;
                        en_valle = false;
                        inicio_ventana_ac = 0;
                        referencia_lista = false;
                        rango_ac = 0;
                    }
                    else
                    {
                        errores_i2c++;
                    }
                    estable_r_desde = to_ms_since_boot(get_absolute_time());
                    inicio_ventana_r = 0;
                    muestras_ventana_r = 0;
                    suma_red_ac2 = suma_ir_ac2 = suma_red_dc = suma_ir_dc = 0;
                    r_valida = false;
                    resultado_r_pendiente = false;
                    razon_r = diagnostico.tipo == MAX30102_ERROR_DESBORDAMIENTO ? "FIFO recuperada" : "error I2C";
                    error_lectura = true;
                    diagnostico_lectura = diagnostico;
                }
                break;
            }
        }

        if (medicion_iniciada && (uint32_t)(tiempo_actual - tiempo_reporte) >= 1000)
        {
            tiempo_reporte = tiempo_actual;

            if (dedo_presente && cantidad_intervalos >= 3 && ultimo_latido != 0 && (uint32_t)(tiempo_actual - ultimo_latido) <= SIN_LATIDO_MS)
            {
                uint32_t bpm = (60000u * cantidad_intervalos + suma_intervalos / 2) / suma_intervalos;
                printf("IR: %lu | BPM: %lu | Muestras: %lu | Recuperaciones: %lu\n", (unsigned long)ir, (unsigned long)bpm, (unsigned long)muestras, (unsigned long)recuperaciones);
            }
            else
            {
                printf("IR: %lu | BPM: -- | Muestras: %lu | Recuperaciones: %lu\n", (unsigned long)ir, (unsigned long)muestras, (unsigned long)recuperaciones);
            }
        }

        if ((uint32_t)(tiempo_actual - tiempo_estado) >= 5000)
        {
            tiempo_estado = tiempo_actual;

            if (dedo_presente && resultado_r_pendiente && r_valida)
            {
                double spo2_estimada = SPO2_INTERCEPTO - SPO2_PENDIENTE * relacion_r;
                printf("RED_DC: %.1f | IR_DC: %.1f\n", red_dc_ventana, ir_dc_ventana);
                printf("RED_AC_RMS: %.1f | IR_AC_RMS: %.1f\n", red_rms, ir_rms);
                printf("R: %.3f\n", relacion_r);

                if (isfinite(spo2_estimada) && spo2_estimada >= 0.0 && spo2_estimada <= 100.0)
                {
                    printf("SpO2 estimada (NO CALIBRADA): %.1f %%\n", spo2_estimada);
                }
                else
                {
                    printf("SpO2 estimada (NO CALIBRADA): -- (fuera de 0-100 %%)\n");
                }
            }
            else
            {
                printf("R: -- (%s)\n", razon_r);
                printf("SpO2 estimada (NO CALIBRADA): --\n");
            }

            resultado_r_pendiente = false;
            r_valida = false;
            if (dedo_presente) razon_r = "ventana incompleta";

            if (medicion_iniciada)
            {
                uint8_t wr_ptr = 0;
                uint8_t rd_ptr = 0;
                uint8_t ovf_counter = 0;
                uint8_t mode_config = 0;

                if (max30102_leer_estado(&wr_ptr, &rd_ptr, &ovf_counter, &mode_config))
                {
                    printf("# Tiempo: %lu ms | Muestras: %lu | WR: 0x%02X | RD: 0x%02X | OVF: 0x%02X | MODE: 0x%02X | Recuperaciones: %lu | Errores I2C: %lu\n", (unsigned long)tiempo_actual, (unsigned long)muestras, wr_ptr, rd_ptr, ovf_counter, mode_config, (unsigned long)recuperaciones, (unsigned long)errores_i2c);
                }
                else
                {
                    max30102_obtener_diagnostico(&diagnostico);
                    errores_i2c++;
                    error_lectura = true;
                    diagnostico_lectura = diagnostico;
                    printf("# Tiempo: %lu ms | Muestras: %lu | Error leyendo estado | Errores I2C: %lu\n", (unsigned long)tiempo_actual, (unsigned long)muestras, (unsigned long)errores_i2c);
                }
            }
            else
            {
                printf("# Tiempo: %lu ms | Muestras: %lu | Inicializacion fallida\n", (unsigned long)tiempo_actual, (unsigned long)muestras);
                mostrar_error(&diagnostico);
            }

            if (error_lectura)
            {
                mostrar_error(&diagnostico_lectura);
                error_lectura = false;
            }
        }

        tight_loop_contents();
    }

    return 0;
}
