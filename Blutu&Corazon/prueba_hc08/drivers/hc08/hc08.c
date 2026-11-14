
#include "hc08.h"

#include <stddef.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#define HC08_UART uart1
#define HC08_TX_PIN 4
#define HC08_RX_PIN 5
#define HC08_STATE_PIN 6
#define HC08_BAUD 9600

#define HC08_BUFFER_SIZE 128

// Buffer de recepcion (interrupciones)
static uint8_t rx_buffer[HC08_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;
static volatile uint32_t rx_dropped = 0;

// Buffer de transmision (programa principal)
static uint8_t tx_buffer[HC08_BUFFER_SIZE];
static uint16_t tx_head = 0;
static uint16_t tx_tail = 0;

// Interrupcion de recepcion UART
static void hc08_uart_irq(void)
{
    while (uart_is_readable(HC08_UART))
    {
        uint8_t dato = (uint8_t)uart_getc(HC08_UART);
        uint16_t siguiente = (rx_head + 1) % HC08_BUFFER_SIZE;

        if (siguiente != rx_tail)
        {
            rx_buffer[rx_head] = dato;
            rx_head = siguiente;
        }
        else
        {
            rx_dropped++;
        }
    }
}

void hc08_init(void)
{
    uart_init(HC08_UART, HC08_BAUD);

    gpio_set_function(HC08_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(HC08_RX_PIN, GPIO_FUNC_UART);

    gpio_init(HC08_STATE_PIN);
    gpio_set_dir(HC08_STATE_PIN, GPIO_IN);
    gpio_pull_down(HC08_STATE_PIN);

    uart_set_format(HC08_UART, 8, 1, UART_PARITY_NONE);
    uart_set_hw_flow(HC08_UART, false, false);

    rx_head = 0;
    rx_tail = 0;
    rx_dropped = 0;

    tx_head = 0;
    tx_tail = 0;

    irq_set_exclusive_handler(UART1_IRQ, hc08_uart_irq);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irqs_enabled(HC08_UART, true, false);
}


bool hc08_send(const char *mensaje)
{
    if (mensaje == NULL || !hc08_is_connected())
    {
        return false;
    }

    size_t longitud = strlen(mensaje);
    size_t libres = (tx_tail + HC08_BUFFER_SIZE - tx_head - 1) % HC08_BUFFER_SIZE;

    if (longitud > libres)
    {
        return false;
    }

    for (size_t i = 0; i < longitud; i++)
    {
        tx_buffer[tx_head] = (uint8_t)mensaje[i];
        tx_head = (tx_head + 1) % HC08_BUFFER_SIZE;
    }

    return true;
}



void hc08_task(void)
{
    if (!hc08_is_connected())
    {
        tx_tail = tx_head;
        rx_tail = rx_head;
        return;
    }

    for (int i = 0; i < 16; i++)
    {
        if (tx_head == tx_tail)
        {
            break;
        }

        if (!uart_is_writable(HC08_UART))
        {
            break;
        }

        uart_putc_raw(HC08_UART, (char)tx_buffer[tx_tail]);
        tx_tail = (tx_tail + 1) % HC08_BUFFER_SIZE;
    }
}


bool hc08_read_byte(uint8_t *dato)
{
    if (dato == NULL || rx_head == rx_tail)
    {
        return false;
    }

    *dato = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % HC08_BUFFER_SIZE;

    return true;
}

uint32_t hc08_dropped_bytes(void)
{
    return rx_dropped;
}

bool hc08_is_connected(void)
{
    return gpio_get(HC08_STATE_PIN);
}
