
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hc08.h"

#define MAX_MENSAJE 64
#define INTERVALO_REPORTE_MS 5000

static void procesar_mensaje(const char *mensaje)
{
    printf("Mensaje recibido: %s\n", mensaje);

    if (strcmp(mensaje, "PING") == 0)
    {
        if (!hc08_send("PONG\n"))
        {
            printf("Buffer TX lleno\n");
        }
    }
    else
    {
        if (!hc08_send("OK\n"))
        {
            printf("Buffer TX lleno\n");
        }
    }
}

int main(void)
{
    stdio_init_all();
    hc08_init();

    char mensaje[MAX_MENSAJE];
    size_t indice = 0;
    bool descartando = false;

    bool estado_anterior = hc08_is_connected();
    uint32_t ultimo_reporte = to_ms_since_boot(get_absolute_time());

    printf("Driver HC-08 iniciado\n");

    while (true)
    {
        uint32_t tiempo_actual = to_ms_since_boot(get_absolute_time());

        // Detectar cambios en la conexion BLE
        bool estado_actual = hc08_is_connected();

        if (estado_actual != estado_anterior)
        {
            estado_anterior = estado_actual;

            // Descartar cualquier mensaje incompleto
            indice = 0;
            descartando = false;

            if (estado_actual)
            {
                printf("Bluetooth CONECTADO\n");
            }
            else
            {
                printf("Bluetooth DESCONECTADO\n");
            }
        }

        // Atender transmision pendiente
        hc08_task();

        // Procesar los datos recibidos
        uint8_t dato;

        for (int i = 0; i < 16; i++)
        {
            if (!hc08_read_byte(&dato))
            {
                break;
            }

            if (dato == '\r')
            {
                continue;
            }

            if (dato == '\n')
            {
                if (descartando)
                {
                    printf("Mensaje demasiado largo\n");
                    descartando = false;
                }
                else if (indice > 0)
                {
                    mensaje[indice] = '\0';
                    procesar_mensaje(mensaje);
                }

                indice = 0;
            }
            else if (!descartando)
            {
                if (indice < MAX_MENSAJE - 1)
                {
                    mensaje[indice++] = (char)dato;
                }
                else
                {
                    descartando = true;
                }
            }
        }

        // Reporte de diagnostico cada 5 segundos
        if ((uint32_t)(tiempo_actual - ultimo_reporte) >= INTERVALO_REPORTE_MS)
        {
            ultimo_reporte = tiempo_actual;

            printf("Estado BLE: %s | Bytes descartados: %lu\n",
                   hc08_is_connected() ? "Conectado" : "Desconectado",
                   (unsigned long)hc08_dropped_bytes());
        }

        tight_loop_contents();
    }

    return 0;
}
