add_library(ili9488 STATIC
    ili9488_hw.c
    ili9488_init.c
    ili9488_draw.c
    ili9488_gfx.c
    ili9488_font.c
)

# Quien enlace esta librería puede hacer #include "ili9488_hw.h", etc.
target_include_directories(ili9488 PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Dependencias que la librería necesita y transmite a quien la use
target_link_libraries(ili9488 PUBLIC pico_stdlib hardware_spi)