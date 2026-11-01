#ifndef ILI9488_HW_H
#define ILI9488_HW_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"

#define LCD_SPI        spi0
#define LCD_SPI_BAUD   (10u * 1000u * 1000u)   // bring-up; se sube despues

#define LCD_PIN_SCK    18
#define LCD_PIN_MOSI   19
#define LCD_PIN_CS     17
#define LCD_PIN_DC     20
#define LCD_PIN_RST    21
#define LCD_PIN_LED    22

#define LCD_MASK_CS    (1u << LCD_PIN_CS)
#define LCD_MASK_DC    (1u << LCD_PIN_DC)
#define LCD_MASK_RST   (1u << LCD_PIN_RST)
#define LCD_MASK_LED   (1u << LCD_PIN_LED)
#define LCD_MASK_CTRL  (LCD_MASK_CS | LCD_MASK_DC | LCD_MASK_RST | LCD_MASK_LED)

// Escritura de pines por mascara (SIO)
static inline void lcd_cs_low(void)   { sio_hw->gpio_clr = LCD_MASK_CS;  }
static inline void lcd_cs_high(void)  { sio_hw->gpio_set = LCD_MASK_CS;  }
static inline void lcd_dc_cmd(void)   { sio_hw->gpio_clr = LCD_MASK_DC;  }
static inline void lcd_dc_data(void)  { sio_hw->gpio_set = LCD_MASK_DC;  }
static inline void lcd_rst_low(void)  { sio_hw->gpio_clr = LCD_MASK_RST; }
static inline void lcd_rst_high(void) { sio_hw->gpio_set = LCD_MASK_RST; }
static inline void lcd_led_on(void)   { sio_hw->gpio_set = LCD_MASK_LED; }
static inline void lcd_led_off(void)  { sio_hw->gpio_clr = LCD_MASK_LED; }

// Primitivas SPI no bloqueantes (sin ciclos)
static inline bool lcd_spi_try_write(uint8_t b) {
    if (!(spi_get_hw(LCD_SPI)->sr & SPI_SSPSR_TNF_BITS)) return false; // FIFO lleno
    spi_get_hw(LCD_SPI)->dr = b;
    return true;
}

static inline bool lcd_spi_idle(void) {
    uint32_t sr = spi_get_hw(LCD_SPI)->sr;
    return (sr & SPI_SSPSR_TFE_BITS) && !(sr & SPI_SSPSR_BSY_BITS);
}

void lcd_hw_init(void);

#endif /* ILI9488_HW_H */
