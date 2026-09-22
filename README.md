# Pipboy

**Proyecto:** Segunda iteración — Electrónica Digital III  
**Integrantes:** Felipe Jiménez Ramírez, Jose Manuel Arias Toro, Sergio Andrés Alarcón López y Leonel José Pimienta Rodríguez  


## Nombre del proyecto

Pipboy

## Descripción

El proyecto consiste en el desarrollo de un tipo de brazalete inteligente, inspirado en los smartwatches convencionales, pero en un formato mucho menos compacto. Como equipo se planteó implementar un sistema capaz de monitorear variables de salud en el cuerpo humano, con pantalla TFT donde el usuario pueda interactuar por medio de un menú con todas las funcionalidades que se quieren implementar utilizando un codificador rotativo. Entre estas funcionalidades se incluye la lectura del ritmo cardíaco junto con el nivel de oxígeno en la sangre, un sensor de corriente y voltaje para estimar la corriente consumida por el sistema junto el voltaje de la fuente de alimentación, una IMU para obtener información inercial, un sensor para obtener la respuesta galvánica de la piel y poder determinar si el portador se encuentra bajo estrés o algún otro estado, un módulo RTC para guardar la hora, un módulo bluetooth BLE para comunicar el dispositivo mediante comunicación inalámbrica y un sensor de temperatura inflarrojo para la medición sin contacto. Adicional a esto se integrará un buzzer y un motor de vibración háptica.

Durante esta segunda iteración de la idea hemos realizado algunos cambios , buscando dejar más claro alcance y el propósito del dispositivo, simplificar la implementación y concentrarnos en las funciones más importantes para este proyecto.

1.  **Selección de la pantalla TFT:** Inicialmente contemplábamos utilizar una pantalla TFT u OLED. Finalmente, decidimos utilizar una pantalla TFT, ya que nos permite desarrollar la interfaz gráfica a color que queremos implementar y fue la pantalla más grande que encontramos accesible en internet.

2.  **Selección del codificador rotativo:** En la propuesta inicial considerábamos utilizar una perilla o, si era posible, implementar una pantalla táctil. Decidimos utilizar un codificador rotativo KY-040 porque permite navegar por los menús y seleccionar opciones mediante su pulsador. Además, nos permite simplificar la interacción con el dispositivo.

3.  **Eliminación del módulo GPS:** Inicialmente queríamos incorporar un GPS para obtener información sobre la ubicación del usuario. Sin embargo, decidimos retirar esta funcionalidad para concentrarnos en el monitoreo de variables fisiológicas y reducir la cantidad de componentes que debemos integrar. Con esto también buscamos disminuir el consumo energético, la complejidad y los costos.

4.  **Incorporación del módulo Bluetooth BLE:** Agregamos un módulo Bluetooth para permitir que el dispositivo transmita de forma inalámbrica la información obtenida por los sensores. Consideramos que esta funcionalidad complementa la visualización de los datos en la pantalla, ya que permite consultarlos desde otro dispositivo compatible y se contrasta con la retroalimentación realizada por el docente.

5.  **Incorporación del sensor de temperatura infrarrojo:** Decidimos agregar el sensor MLX90614 para incluir la medición de temperatura superficial sin contacto. Con esto ampliamos las variables que podemos medir y aprovechamos esto para complementar las mediciones fisiológicas.

6.  **Definición del sistema de vibración háptica:** Desde la idea inicial contemplábamos utilizar un motor de vibración. En esta iteración definimos su función como un sistema de notificaciones hápticas, de manera que pueda complementar los sonidos emitidos por el buzzer permitiendo que el usuario reciba notificaciones mediante vibraciones.

## Viabilidad Económica

### Presupuesto de Componentes Electrónicos

| **Componente** | **Descripción** | **Costo ($)** |
| --- | --- | --- |
| Raspberry Pi Pico RP2040 | Microcontrolador encargado de procesar los datos y controlar el sistema. | 25.000 |
| Pantalla TFT con driver ILI9488 | Interfaz gráfica a color para visualización de datos. | 82.600 |
| Encoder Rotativo KY040 | Control de navegación y selección en menús. | 4.600 |
| Sensor Ritmo cardiaco y oxígeno en sangre | Medición de pulso y saturación de oxígeno (SpO2). | 24.000 |
| Sensor de Temperaturas MLX90614 | Medición de temperatura corporal sin contacto por infrarrojos. | 50.000 |
| INA-219 | Monitoreo de voltaje, corriente y potencia del sistema. | 17.900 |
| GY-521 | Acelerómetro y giroscopio de 6 ejes (módulo MPU6050). | 18.000 |
| Sensor GSR | Medición de respuesta galvánica de la piel (sudoración/estrés). | 71.400 |
| Sensor DS1307 | Reloj de tiempo real (RTC) para el registro temporal de eventos. | 5.700 |
| Módulo Bluetooth HM-10 4.0 | Comunicación inalámbrica de bajo consumo (BLE). | 30.000 |
| Buzzer Activo | Generación de alertas sonoras del sistema. | 1.200 |
| Motor vibrador | Generación de notificaciones hápticas y alarmas silenciosas. | 6.000 |
| Módulo TP4056 con protección | Gestión de carga y protección de una batería de litio 18650. | 7.000 |
| **Subtotal** |  | **343.400** |

### Componentes Adicionales de Alimentación y Conexión

| **Componente** | **Descripción** | **Costo ($)** |
| --- | --- | --- |
| Batería recargable 18650 | Fuente de alimentación portátil del dispositivo. | 18.000 |
| Portabatería 18650 | Soporte para la conexión de la batería al circuito. | 3.000 |
| Convertidor elevador de voltaje | Generación de una tensión de alimentación superior a la de la batería cuando sea necesario. | 4.000 |
| Conversor de niveles lógicos | Adaptación de señales entre componentes que operan con diferentes niveles de tensión. | 6.000 |
| Batería de respaldo RTC | Conservación de la fecha y hora cuando se interrumpe la alimentación principal. | 4.000 |
| Interruptor de encendido | Permite conectar y desconectar la alimentación del sistema. | 2.000 |
| Componentes electrónicos auxiliares | Resistencias, transistores, diodos y capacitores necesarios para el acondicionamiento y control. | 5.000 |
| **Subtotal** |  | **42.000** |

### Materiales y Servicios

| **Material o servicio** | **Descripción** | **Costo ($)** |
| --- | --- | --- |
| Placa perforada | Base para el montaje y soldadura de los componentes electrónicos. | 8.000 |
| Cables Dupont | Conexiones eléctricas durante las etapas de desarrollo y pruebas. | 7.000 |
| Cable para conexiones | Cableado para las conexiones permanentes del prototipo. | 5.000 |
| Cable USB | Programación y alimentación de la Raspberry Pi Pico. | 5.000 |
| Correas y velcro | Sistema de sujeción del brazalete al brazo del usuario. | 8.000 |
| Material aislante | Cinta aislante y termorretráctil para proteger las conexiones eléctricas. | 5.000 |
| Estaño y consumibles | Materiales necesarios para el proceso de soldadura y ensamblaje. | 5.000 |
| Filamento para impresión 3D | Material disponible para fabricar la carcasa y sus soportes. | 0 |
| Servicio de impresión 3D | Fabricación de la carcasa mediante impresora propia de uno de los participantes. | 0 |
| Diseño y programación | Desarrollo del software y diseño del prototipo realizados por los participantes. | 0 |
| Transporte y envíos | Gastos estimados asociados a la adquisición de componentes y materiales. | 15.000 |
| **Subtotal** |  | **58.000** |

### Análisis de Viabilidad Económica

El proyecto cuenta con cuatro integrantes y, de acuerdo con el presupuesto realizado, se estima un costo total de $487.740 COP. Si dividimos este valor de manera equitativa, cada integrante tendría que aportar aproximadamente $120000 COP. Consideramos que contamos con los recursos necesarios para cubrir estos gastos, por lo que el proyecto es económicamente viable. Sin embargo, creemos que el presupuesto está un poco sobredimensionado, ya que algunos precios son estimados y podrían encontrarse alternativas más económicas. Por esta razón, buscaremos comparar precios entre diferentes proveedores y aprovechar los componentes y materiales que ya tenemos disponibles, sin afectar las funcionalidades que queremos implementar. Además, uno de los integrantes cuenta con una impresora 3D, lo que nos permitirá fabricar la carcasa del dispositivo sin tener que pagar por un servicio de impresión externo. En general, contamos con el presupuesto para desarrollar el proyecto tal como lo planteamos inicialmente, pero buscaremos reducir los gastos siempre que sea posible, manteniendo las funcionalidades y los requisitos.

### Presupuesto General

| **Categoría** | **Costo ($)** |
| --- | --- |
| Componentes electrónicos principales | 343.400 |
| Componentes adicionales de alimentación y conexión | 42.000 |
| Materiales y servicios | 58.000 |
| Subtotal del proyecto | 443.400 |
| Reserva para imprevistos (10%) | 44.340 |
| **Costo total estimado** | **487.740** |

## Interpretación y Delimitación

El sistema consiste en una alternativa para el monitoreo de algunos parámetros relacionados con el estado de salud del usuario. El dispositivo cuenta con diferentes sensores encargados de adquirir información fisiológica, una interfaz visual para presentar los datos obtenidos y una perilla mediante la cual el usuario puede interactuar con el sistema y seleccionar las diferentes opciones disponibles. El sistema procesa la información obtenida por los sensores y presenta al usuario indicadores relacionados con los parámetros monitoreados. Su propósito es proporcionar información de carácter preventivo y orientativo, por lo que los resultados no deben interpretarse como un diagnóstico médico ni como un sustituto de la valoración realizada por un profesional de la salud. El dispositivo está concebido para ser utilizado por una persona durante condiciones normales de operación. Debido a sus características de construcción, no es resistente al agua y no se recomienda su utilización bajo condiciones climáticas extremas que puedan afectar el funcionamiento de sus componentes electrónicos o sensores. La temática visual y funcional del dispositivo está basada en el Pip-Boy de la franquicia Fallout, utilizando esta referencia como concepto de diseño e interacción. Sin embargo, el sistema desarrollado tiene como finalidad específica el monitoreo de parámetros fisiológicos definidos para el proyecto.

### Alcance

El proyecto abarca el diseño (circuitería, modelado 3D de la carcasa), construcción y programación de un prototipo funcional de brazalete inteligente inspirado en el Pip-Boy. El sistema será capaz de adquirir, procesar y visualizar de forma local e inalámbrica (vía BLE) variables fisiológicas (ritmo cardíaco, SpO2, temperatura superficial, respuesta galvánica de la piel) y datos inerciales. El alcance no incluye el diseño de sistemas resistentes al agua o polvo, el uso del dispositivo bajo condiciones climáticas extremas y cualquier uso en el ámbito médico formal.

### Actores

- Portador: La persona que lleva el pipboy en su brazo, la que interactúa con el codificador rotativo a la que se miden las mediciones fisiológicas.

- Observador: Persona externa o el mismo usuario que consulta las mediciones de forma remota.

### Entradas

- Físicas: Giros (horario y antihorario) y pulsaciones provenientes del codificador rotativo (KY-040).

- Fisiológicas:

  - Absorción de luz infrarroja/roja en el dedo (ritmo cardíaco y SpO2).

  - Radiación infrarroja emitida por la piel (temperatura superficial vía MLX90614).

  - Conductividad eléctrica de la piel (sensor GSR).

  - Aceleración lineal y velocidad angular (IMU GY-521).

- Eléctricas: Energía suministrada por la batería de litio 18650, gestionada por el módulo TP4056 y el convertidor elevador.

### Salidas

- Visuales: Interfaz gráfica a color (menús, indicadores, valores de los sensores, advertencias) renderizada en la pantalla TFT.

- Inalámbricas: Paquetes de datos con la telemetría de los sensores transmitidos mediante el módulo Bluetooth BLE (HM-10) hacia un celular.

- Sonoras: Tonos de notificación y alarmas emitidos por el buzzer activo.

- Hápticas: Patrones de vibración generados por el motor vibrador.

### Modos de Operación

- El sistema reduce el consumo energético, la pantalla puede atenuarse o mostrar únicamente la fecha/hora (RTC DS1307), y la lectura de sensores no críticos se suspende.

- Modo de Operación Normal (Monitoreo): El microcontrolador RP2040 adquiere datos de todos los sensores de forma continua, actualiza la pantalla TFT y transmite la telemetría por Bluetooth.

- Modo de Alerta: Se activa cuando el sistema detecta que un sensor se ha desconectado, que las lecturas no están disponibles, o si se programan umbrales específicos. En este modo se activan patrones sonoros y hápticos distintivos.

### Supuestos

- El usuario utilizará el brazalete en un entorno interior o bajo condiciones ambientales estables y secas.

- Se asume que el usuario portador cuenta con un dispositivo móvil compatible (smartphone) con una aplicación genérica de escaneo BLE para la recepción de los datos, ya que el desarrollo de una app móvil a medida no hace parte del alcance.

- Las mediciones del sensor GSR y de temperatura requieren que la carcasa mantenga el contacto físico adecuado y cercanía con la piel del usuario sin generar incomodidad.

### Preguntas Pendientes

- ¿Cuál será la autonomía real del prototipo alimentado alimentado con la batería 18650 considerando el consumo de la pantalla TFT iluminada, el módulo BLE transmitiendo y el procesamiento continuo del microcontrolador?

- ¿Qué estrategias de diseño mecánico (en el modelado y la impresión 3D de la carcasa) se implementarán para aislar el calor generado por el convertidor de voltaje y la electrónica, evitando que altere las lecturas del sensor de temperatura infrarrojo (MLX90614)?

- ¿Cómo se gestionará por software el filtrado de ruido en las lecturas del sensor GSR y el acelerómetro provocado por los movimientos naturales del brazo del usuario?

## Especificaciones de Requisitos

### Requisitos Funcionales

| **ID** | **Descripción** | **Fuente** | **Criterio de Verificabilidad** |
| --- | --- | --- | --- |
| RF-01 | El sistema deberá medir el ritmo cardíaco y la saturación de oxígeno del usuario de forma continua. | Propuesta original del equipo. | Comparación de las lecturas obtenidas con rangos fisiológicos de referencia encontrados en fuentes confiables de internet. |
| RF-02 | El sistema deberá permitir la navegación por un menú en la pantalla utilizando el codificador rotativo. | Decisión de diseño (Iteración 2) para simplificar hardware. | Verificación visual del cambio de opciones al girar el codificador y de la selección al presionarlo. |
| RF-03 | El sistema deberá transmitir la información del cuerpo medida de forma inalámbrica mediante el módulo Bluetooth BLE. | Retroalimentación del profesor (Iteración 1). | Verificación de la recepción de los datos transmitidos mediante una aplicación Bluetooth BLE instalada en un celular. |
| RF-04 | El sistema deberá registrar la respuesta galvánica de la piel (GSR) del usuario para obtener información sobre su actividad electrodérmica. | Propuesta original del equipo. | Verificación de los cambios en las lecturas al colocar y retirar los dedos del sensor GSR. |
| RF-05 | El sistema deberá medir el voltaje de la fuente de alimentación y estimar la corriente consumida por el circuito. | Propuesta original del equipo. | Comparación de las lecturas de voltaje con un multímetro y observación de los cambios de corriente al activar diferentes componentes del sistema. |
| RF-06 | El sistema deberá obtener información inercial del movimiento del dispositivo mediante una unidad de medición inercial (IMU). | Propuesta original del equipo. | Verificación de los cambios en las lecturas de aceleración y velocidad angular al mover y rotar el dispositivo. |
| RF-07 | El sistema deberá mantener un registro actualizado de la fecha y hora de manera autónoma mediante un módulo RTC. | Propuesta original del equipo. | Comparación de la fecha y hora del dispositivo con las de un celular, antes y después de reiniciar el sistema. |
| RF-08 | El sistema deberá medir la temperatura superficial del usuario mediante un sensor infrarrojo sin contacto. | Adición de diseño (Iteración 2). | Verificación de que el sensor proporciona lecturas de temperatura y que estas cambian al medir superficies con diferentes temperaturas. |
| RF-09 | El sistema deberá mostrar en la pantalla TFT las mediciones obtenidas por los diferentes sensores integrados. | Propuesta original del equipo. | Verificación visual de que las mediciones de los sensores se muestran correctamente en la pantalla TFT. |
| RF-10 | El sistema deberá generar señales sonoras mediante un buzzer para indicar eventos y alertas previamente definidos. | Propuesta original del equipo. | Comprobación auditiva de la activación del buzzer al ejecutar los eventos programados. |
| RF-11 | El sistema deberá generar señales hápticas mediante un motor de vibración para notificar eventos y alertas previamente definidos. | Definición de diseño (Iteración 2). | Comprobación de la activación del motor de vibración al ejecutar los eventos programados. |
| RF-12 | El sistema deberá permitir establecer y restablecer una conexión inalámbrica Bluetooth BLE con un dispositivo compatible. | Retroalimentación del profesor (Iteración 1). | Prueba de conexión, desconexión y reconexión del dispositivo mediante una aplicación Bluetooth BLE en un celular. |
| RF-13 | El sistema deberá permitir consultar la fecha y hora actuales mediante la interfaz gráfica. | Propuesta original del equipo. | Comparación de la fecha y hora mostradas en pantalla con las de un celular. |
| RF-14 | El sistema deberá identificar cuando las lecturas de los sensores no estén disponibles y notificarlo al usuario. | Buenas prácticas de ingeniería de software. | Desconexión controlada de un sensor y verificación de que el sistema indique que la lectura no está disponible. |

### Requisitos No Funcionales

| **ID** | **Descripción** | **Fuente** | **Criterio de Verificabilidad** |
| --- | --- | --- | --- |
| RNF-01 | El sistema deberá actualizar la interfaz de la pantalla TFT en un tiempo no mayor a 200 ms tras interactuar con el codificador. | Criterio de usabilidad e interacción humano-máquina. | Medición del tiempo entre la interacción con el codificador y la actualización de pantalla mediante registros de tiempo en el programa. |
| RNF-02 | Las señales sonoras y hápticas del sistema deberán presentar patrones diferenciables para los estados de espera, operación normal y alerta. | Criterio de diseño de interfaces de usuario (UI/UX). | Activación de cada estado del sistema y comprobación de que los patrones de sonido y vibración sean diferentes. |
| RNF-03 | La interfaz gráfica de usuario deberá implementar una temática visual y funcional basada en el diseño del Pip-Boy de Fallout. | Concepto estético original del equipo. | Comparación visual de la interfaz desarrollada con imágenes de referencia del Pip-Boy de Fallout encontradas en internet. |
| RNF-04 | El ensamble físico del sistema deberá tener el formato de un brazalete para ser portado en el brazo del usuario. | Restricción física del proyecto. | Prueba de colocación del brazalete en el brazo y verificación de que permanezca sujeto durante movimientos habituales. |
| RNF-05 | El sistema deberá mantener una comunicación Bluetooth BLE estable dentro del alcance de funcionamiento establecido para el proyecto. | Limitaciones técnicas del módulo HM-10. | Prueba de transmisión de datos hacia un celular a diferentes distancias, comprobando que la conexión se mantenga dentro del alcance establecido. |
| RNF-06 | El sistema deberá actualizar las mediciones mostradas en pantalla dentro de los intervalos establecidos para cada variable. | Capacidades de muestreo (Datasheets) de los sensores. | Medición del tiempo entre actualizaciones mediante registros de tiempo en el programa y comparación con los intervalos establecidos. |
| RNF-07 | El sistema deberá proporcionar una autonomía mínima de funcionamiento de acuerdo con la capacidad de la fuente de alimentación seleccionada. | Restricción de hardware (Batería 18650). | Medición del tiempo de funcionamiento del dispositivo desde una carga completa hasta alcanzar el nivel mínimo de carga establecido. |
| RNF-08 | El sistema deberá proporcionar lecturas coherentes y repetibles de los sensores bajo condiciones de medición constantes. | Requisito de instrumentación electrónica. | Registro de varias lecturas consecutivas bajo condiciones similares y comparación de los valores obtenidos para identificar variaciones inesperadas. |
| RNF-09 | La interfaz gráfica deberá presentar los datos y opciones de navegación de forma legible, sin superposición de elementos. | Resolución física de la pantalla TFT ILI9488. | Inspección visual de los diferentes menús para comprobar que los textos, valores e iconos se visualicen completamente y sin superposiciones. |
| RNF-10 | El sistema deberá mantener su funcionamiento durante la operación simultánea de los sensores, la pantalla y la comunicación inalámbrica, sin presentar bloqueos durante el periodo de prueba establecido. | Requisito de arquitectura de software embebido. | Prueba de funcionamiento continuo durante un periodo establecido, verificando que los sensores, la pantalla y el Bluetooth funcionen sin bloqueos ni reinicios inesperados. |
| RNF-11 | El sistema deberá impedir el acceso no autorizado a los datos transmitidos mediante Bluetooth BLE. | Estándar de seguridad de datos personales. | Intento de acceso a los datos desde un segundo celular no autorizado, verificando que el mecanismo de protección implementado impida su lectura. |
| RNF-12 | El ensamblaje físico deberá impedir el contacto accidental del usuario con las conexiones eléctricas durante su utilización normal. | Normativa básica de seguridad eléctrica para wearables. | Inspección visual del ensamblaje y prueba de uso para comprobar que las conexiones eléctricas no queden expuestas al contacto accidental. |

### Requisitos Ambientales

| **ID** | **Descripción** | **Fuente** | **Criterio de Verificabilidad** |
| --- | --- | --- | --- |
| RA-01 | El sistema deberá operar dentro del rango de temperatura ambiente establecido para los componentes electrónicos integrados. | Especificaciones técnicas (Datasheets) de los componentes electrónicos. | Consulta de los rangos de temperatura de operación en las hojas de datos de los componentes y prueba de funcionamiento a temperatura ambiente. |
| RA-02 | El sistema deberá mantener su funcionamiento durante los movimientos habituales del brazo del usuario, sin presentar desconexiones eléctricas ni interrupciones inesperadas. | Restricción mecánica por tratarse de un dispositivo portable (wearable). | Prueba de uso realizando movimientos de flexión, extensión y rotación del brazo, verificando que el dispositivo continúe funcionando. |
| RA-03 | El sistema deberá mantener sus conexiones y componentes sujetos durante las vibraciones producidas por el movimiento habitual del usuario y el accionamiento del motor háptico. | Consideración de diseño mecánico. | Activación del motor de vibración y realización de movimientos habituales del brazo, comprobando que los componentes y conexiones permanezcan sujetos. |
| RA-04 | El sistema deberá permitir la lectura de la pantalla TFT bajo las condiciones de iluminación ambiental definidas para su utilización. | Limitaciones de retroiluminación de la pantalla TFT. | Prueba de lectura de la pantalla en un ambiente interior iluminado y en otro con menor iluminación, verificando que la información sea legible. |

### Requisitos Normativos

| **ID** | **Descripción** | **Fuente** | **Criterio de Verificabilidad** |
| --- | --- | --- | --- |
| RNE-01 | El sistema deberá advertir al usuario que los datos obtenidos no constituyen un diagnóstico médico ni sustituyen la evaluación de un profesional de la salud. | Normativa ética y legal sobre dispositivos médicos no certificados. | Encendido del dispositivo y verificación visual de que aparezca una advertencia indicando que las mediciones no constituyen un diagnóstico médico. |

## Planificación de la Verificación

### Especificación de pruebas

| **ID Prueba** | **Requisito(s)** | **Objetivo y procedimiento** | **Criterio de aceptación** |
| --- | --- | --- | --- |
| PR-01 | RF-01 | Colocar el sensor de ritmo cardiaco y SpO2 sobre una persona y registrar 5 lecturas consecutivas en reposo. Comparar los valores contra rangos fisiológicos de referencia (60-100 lpm, SpO2 > 95%) reportados en fuentes médicas confiables. | Las lecturas se ubican dentro de los rangos fisiológicos de referencia en al menos 4 de las 5 mediciones. |
| PR-02 | RF-02 | Girar el codificador en ambos sentidos y presionar su pulsador dentro de cada menú, observando el cambio de opción resaltada y la entrada a submentús. | El cursor de selección se desplaza en la dirección correspondiente al giro y el pulsador confirma la opción resaltada. |
| PR-03 | RNF-01 | Registrar mediante temporizador por software (`millis()`) el intervalo entre la interacción con el codificador (giro o pulsación) y la actualización visible en pantalla, repitiendo la medición 10 veces. | El tiempo de actualización es inferior a 200 ms en el 100% de las repeticiones. |
| PR-04 | RF-03 | Emparejar el módulo BLE con una aplicación de escaneo BLE (por ejemplo nRF Connect) instalada en un celular y verificar la recepción de los paquetes con los datos fisiológicos transmitidos por el dispositivo. | Los valores recibidos en la aplicación coinciden con los mostrados en la pantalla del dispositivo, con una diferencia despreciable atribuible a la latencia de transmisión. |
| PR-05 | RF-12 / RNF-05 | Con el dispositivo encendido, establecer la conexión BLE desde el celular, desconectarla manualmente y volver a conectarla. Repetir la prueba a distancias de 1 m, 5 m y en el límite de alcance declarado. | El dispositivo permite conectar, desconectar y reconectar sin reinicios, manteniendo la conexión estable dentro del alcance establecido para el proyecto. |
| PR-08 | RF-05 | Medir con un multímetro el voltaje de la fuente de alimentación y contrastarlo con la lectura reportada por el INA-219. Activar y desactivar distintos componentes (pantalla, BLE, motor) y observar el cambio en la corriente reportada. | La lectura de voltaje del sistema difiere en menos de 5% respecto al multímetro, y la corriente reportada aumenta de forma coherente al activar cada componente. |
| PR-09 | RF-06 | Mover y rotar el dispositivo en los tres ejes mientras se observan en pantalla o por BLE las lecturas de aceleración y velocidad angular entregadas por el GY-521. | Las lecturas cambian de forma coherente con el movimiento aplicado (aumentan al mover/rotar y se estabilizan en reposo). |
| PR-10 | RF-07 / RF-13 | Configurar la fecha y hora del RTC, consultarlas en la interfaz gráfica y compararlas con las de un celular. Reiniciar el sistema (desconectando la alimentación principal) y verificar que la fecha y hora se conserven gracias a la batería de respaldo. | La fecha y hora mostradas coinciden con las del celular antes y después del reinicio, con una desviación menor a 1 minuto. |
| PR-11 | RF-08 | Apuntar el sensor MLX90614 hacia superficies con temperaturas conocidas y distintas (por ejemplo, piel y un objeto frío) y registrar las lecturas obtenidas. | El sensor reporta lecturas de temperatura que cambian de forma coherente según la superficie medida, dentro de la precisión indicada en su hoja de datos. |
| PR-12 | RF-09 / RNF-09 | Recorrer cada pantalla del menú verificando que las mediciones de todos los sensores se muestren correctamente, con textos, valores e íconos completos y sin superposición de elementos. | Todas las mediciones se visualizan de forma legible y correcta en su pantalla correspondiente, sin elementos superpuestos o cortados. |
| PR-13 | RF-10 / RF-11 / RNF-02 | Provocar de forma controlada los distintos estados del sistema (espera, operación normal y alerta) y verificar de forma auditiva y sensorial la activación del buzzer y el motor de vibración, comparando los patrones entre estados. | El buzzer y el motor de vibración se activan en cada evento programado y los patrones sonoros y hápticos son claramente distinguibles entre los tres estados. |
| PR-15 | RNF-03 | Comparar visualmente la interfaz gráfica desarrollada (colores, tipografía y disposición de menús) contra imágenes de referencia del Pip-Boy de la franquicia Fallout. | La interfaz conserva los elementos visuales característicos de referencia (paleta de color, tipografía y disposición general), a criterio del equipo evaluador. |
| PR-16 | RNF-04 | Colocar el brazalete en el brazo de un integrante y realizar movimientos habituales (caminar, mover el brazo, flexionar la muñeca) durante varios minutos. | El brazalete permanece sujeto al brazo durante los movimientos habituales, sin desprenderse ni desajustarse de forma notoria. |
| PR-17 | RNF-06 | Registrar mediante temporizador por software el intervalo entre actualizaciones sucesivas de cada variable mostrada en pantalla y compararlo con el intervalo definido para esa variable. | El intervalo medido entre actualizaciones coincide con el intervalo establecido, con una tolerancia menor al 10%. |
| PR-19 | RNF-08 | Registrar varias lecturas consecutivas de cada sensor bajo condiciones de medición constantes (mismo usuario, misma posición, mismo ambiente) y comparar los valores obtenidos entre sí. | Las lecturas consecutivas no presentan variaciones mayores a las esperadas por la resolución y el ruido propio de cada sensor, según su hoja de datos. |
| PR-20 | RNF-10 | Operar el sistema de forma simultánea con todos los sensores activos, la pantalla actualizándose y la comunicación BLE conectada, durante un periodo de prueba prolongado. | El sistema funciona durante todo el periodo de prueba sin bloqueos, reinicios inesperados ni pérdida de comunicación. |
| PR-21 | RNF-12 | Inspeccionar visualmente el ensamblaje físico del brazalete y manipularlo durante un uso normal simulado, verificando que ninguna conexión eléctrica quede expuesta al contacto del usuario. | Ninguna conexión eléctrica queda expuesta o accesible al contacto accidental durante la inspección y el uso simulado. |
| PR-22 | RA-01 | Consultar en las hojas de datos de cada componente el rango de temperatura de operación y realizar una prueba de funcionamiento del sistema completo a temperatura ambiente. | El sistema opera correctamente a la temperatura ambiente de prueba, la cual se encuentra dentro del rango soportado por todos los componentes. |
| PR-24 | RA-03 | Activar el motor de vibración de forma sostenida mientras se realizan movimientos habituales del brazo, inspeccionando visualmente las conexiones y componentes internos al finalizar. | Los componentes y conexiones internas permanecen sujetos y en su posición original después de la prueba, sin señales de aflojamiento. |
| PR-26 | RNE-01 | Encender el dispositivo desde apagado y verificar la aparición de un mensaje de advertencia indicando que las mediciones no constituyen un diagnóstico médico. | El mensaje de advertencia aparece de forma visible durante el encendido, antes o durante el acceso a las mediciones. |

### Matriz de trazabilidad requisito-prueba

| **Requisito** | **Tipo** | **Prueba(s)** | **Método de verificación** |
| --- | --- | --- | --- |
| RF-01 | Funcional | PR-01 | Prueba |
| RF-02 | Funcional | PR-02 | Demostración |
| RF-03 | Funcional | PR-04 | Prueba |
| RF-04 | Funcional | PR-07 | Prueba |
| RF-05 | Funcional | PR-08 | Prueba |
| RF-06 | Funcional | PR-09 | Prueba |
| RF-07 | Funcional | PR-10 | Prueba |
| RF-08 | Funcional | PR-11 | Prueba |
| RF-09 | Funcional | PR-12 | Inspección |
| RF-10 | Funcional | PR-13 | Demostración |
| RF-11 | Funcional | PR-13 | Demostración |
| RF-12 | Funcional | PR-05 | Prueba |
| RF-13 | Funcional | PR-10 | Prueba |
| RF-14 | Funcional | PR-14 | Prueba |
| RNF-01 | No funcional | PR-03 | Prueba |
| RNF-02 | No funcional | PR-13 | Demostración |
| RNF-03 | No funcional | PR-15 | Inspección |
| RNF-04 | No funcional | PR-16 | Prueba |
| RNF-05 | No funcional | PR-05 | Prueba |
| RNF-06 | No funcional | PR-17 | Prueba |
| RNF-07 | No funcional | PR-18 | Prueba |
| RNF-08 | No funcional | PR-19 | Prueba |
| RNF-09 | No funcional | PR-12 | Inspección |
| RNF-10 | No funcional | PR-20 | Prueba |
| RNF-11 | No funcional | PR-06 | Prueba |
| RNF-12 | No funcional | PR-21 | Inspección |
| RA-01 | Ambiental | PR-22 | Análisis / Prueba |
| RA-02 | Ambiental | PR-23 | Prueba |
| RA-03 | Ambiental | PR-24 | Prueba |
| RA-04 | Ambiental | PR-25 | Prueba |
| RNE-01 | Normativo | PR-26 | Inspección |

### Plan de demostración del sistema en la sustentación

El día de la sustentación algún integrante del equipo tigre portará el brazalete y operará el codificador rotativo mientras el resto del equipo apoya con la instrumentación externa y la exposición. La demostración se estructura en las siguientes etapas:

1.  **Encendido y advertencia inicial:** se enciende el dispositivo desde apagado y se muestra la advertencia de que las mediciones no constituyen un diagnóstico médico, junto con la pantalla principal con la temática visual del Pip-Boy.

2.  **Navegación por el menú:** se recorre el menú principal utilizando el codificador rotativo, mostrando el cambio de opción resaltada al girarlo y la selección al presionarlo, evidenciando la fluidez de la actualización de pantalla.

3.  **Signos vitales:** se muestran en vivo las lecturas de ritmo cardiaco, saturación de oxígeno y temperatura infrarroja tomadas sobre un integrante del equipo, resaltando que los valores se actualizan dentro de los intervalos establecidos .

4.  **Movimiento e inercia:** se mueve y rota el brazo con el dispositivo puesto para mostrar el cambio en las lecturas de la IMU, aprovechando el mismo movimiento para evidenciar que el brazalete permanece sujeto y el sistema no se interrumpe.

5.  **Alimentación del sistema:** se muestra en pantalla el voltaje de la batería y el consumo de corriente, activando y desactivando un componente para evidenciar el cambio de consumo.

6.  **Fecha y hora:** se consulta la fecha y hora en la interfaz y se contrasta verbalmente con la de un celular del equipo.

7.  **Comunicación inalámbrica:** se conecta un celular mediante una aplicación BLE previamente instalada, mostrando en pantalla del celular los mismos datos que aparecen en el dispositivo, y se realiza una desconexión y reconexión rápida para evidenciar la estabilidad del enlace.

8.  **Alertas sonoras y hápticas:** se provoca un evento para mostrar la respuesta simultánea del buzzer, el motor de vibración y la notificación en pantalla.

9.  **Cierre:** se resumen brevemente los requisitos verificados durante la demostración y se deja el dispositivo disponible para que el profe lo inspeccione y, si lo desea, lo pruebe directamente.

Como respaldo ante cualquier falla durante la demostración en vivo trataremos de tener un video corto grabado previamente que muestre el funcionamiento completo del sistema.

### Logística necesaria para la sustentación

Para garantizar que la demostración se realice de manera satisfactoria, se requiere la siguiente logística:

- Dispositivo Pipboy con la batería completamente cargada, y una batería de respaldo adicional ya cargada como contingencia.

- Cable USB y cargador disponibles en el lugar de la sustentación, en caso de requerir recargar el dispositivo antes o durante la presentación.

- Un celular con la aplicación de comunicación BLE instalada, configurada y previamente probada con el dispositivo, para evitar demoras de emparejamiento durante la demostración.

- Cronómetro o el registro de tiempos por software ya preparado, para poder mostrar de forma inmediata los tiempos de actualización de pantalla si el jurado lo solicita.

- Video de respaldo con el funcionamiento completo del sistema, cargado en una table o celular, para utilizarlo en caso de falla durante la demostración en vivo.

- Asignación previa de roles entre los integrantes del equipo: quién porta y opera el dispositivo, quién maneja el celular y la aplicación BLE, y quién conduce la exposición verbal, de modo que la demostración se desarrolle de forma coordinada.

- Ensayo previo completo de la demostración, en las condiciones más cercanas posibles a las del día de la sustentación, para verificar tiempos y detectar posibles fallas con anticipación.


