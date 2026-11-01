#include "ili9488_hw.h"

void lcd_hw_init(void) {
    gpio_init_mask(LCD_MASK_CTRL);

    // Estado inicial: CS y RST en alto, D/C y backlight en bajo
    sio_hw->gpio_set = LCD_MASK_CS | LCD_MASK_RST;
    sio_hw->gpio_clr = LCD_MASK_DC | LCD_MASK_LED;
    gpio_set_dir_out_masked(LCD_MASK_CTRL);

    spi_init(LCD_SPI, LCD_SPI_BAUD);
    spi_set_format(LCD_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(LCD_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(LCD_PIN_MOSI, GPIO_FUNC_SPI);
}
