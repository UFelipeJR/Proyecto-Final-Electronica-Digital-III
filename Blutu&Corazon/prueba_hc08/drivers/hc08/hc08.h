
#ifndef HC08_H
#define HC08_H

#include <stdint.h>
#include <stdbool.h>

void hc08_init(void);
bool hc08_send(const char *mensaje);
bool hc08_read_byte(uint8_t *dato);
void hc08_task(void);
uint32_t hc08_dropped_bytes(void);
bool hc08_is_connected(void);

#endif
