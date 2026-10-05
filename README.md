#  SumoBot Autonomous Robot (4x IBT-2 Driver)

Código de control autónomo para Robot de Sumo optimizado para Arduino Uno/Nano. Implementa máquina de estados finitos, lectura secuencial no bloqueante de radar ultrasónico y control de velocidad diferencial.

##  Especificaciones Hardware
- **Controlador:** Arduino Uno / Nano (ATmega328P)
- **Drivers de Motor:** 4x IBT-2 (H-Bridge 43A)
- **Sensores de Distancia:** 3x HC-SR04 (Izquierda, Centro, Derecha)
- **Sensores de Línea:** 2x TCRT5000 (Izquierda, Derecha)

##  Asignación de Pines

| Componente | Función | Pin Arduino | Tipo |
| :--- | :--- | :--- | :--- |
| **IBT-2 Izquierdo** | RPWM / LPWM | Pin 5 / Pin 6 | PWM |
| **IBT-2 Izquierdo** | R_EN / L_EN | Pin 7 / Pin 8 | Digital |
| **IBT-2 Derecho** | RPWM / LPWM | Pin 9 / Pin 10 | PWM |
| **IBT-2 Derecho** | R_EN / L_EN | Pin 3 / Pin 4 | Digital |
| **Ultrasonido Centro** | TRIG / ECHO | Pin 11 / Pin 12 | Digital |
| **Ultrasonido Izq** | TRIG / ECHO | Pin A0 / Pin A1 | Analógico / Digital |
| **Ultrasonido Der** | TRIG / ECHO | Pin A4 / Pin A5 | Analógico / Digital |
| **Sensores Línea** | IZQ / DER | Pin A3 / Pin A2 | Digital In |

##  Máquina de Estados
1. `INICIO_RUSH`: Arranque directo tras los 5 segundos reglamentarios.
2. `BUSCANDO`: Patrón de escaneo alternado con memoria de última dirección vista.
3. `ATACANDO`: Embestida con corrección proporcional ($K_p$) para centrar al oponente.
4. `ESCAPANDO`: Maniobra de retroceso de emergencia al detectar borde blanco.
5. `GIRANDO180`: Giro rápido de desenganche.
