#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

// Una pasada de la FSM del menu: consume como mucho un evento del encoder
// y encola el dibujo necesario. Nunca bloquea.
void menu_step(void);

// true si el menu aun tiene algo por hacer (evento, repintado o medicion pendiente).
// Mientras sea false y la pantalla este libre, el loop puede dormir con __wfi().
bool menu_ocupado(void);

// Interruptores de la pestana CONF (por defecto, todos encendidos).
// El resto del programa los consulta para habilitar o apagar cada funcion.
bool menu_ble_habilitado(void);
bool menu_buzzer_habilitado(void);
bool menu_vibra_habilitado(void);

#endif /* MENU_H */
