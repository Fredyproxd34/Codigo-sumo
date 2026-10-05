> **Nota de versión:** Este repositorio contiene la versión optimizada para **Tracción Diferencial (2x IBT-2 en paralelo)**. Si buscas la versión con **4 ruedas Mecanum omnidireccionales (4x IBT-2 independientes)**, consulta el repositorio [SumoBot-Mecanum-Omni](https://github.com/Fredyproxd34/SumoBot-Mecanum-Omni).

# SumoBot Autonomous Robot (2x IBT-2 Driver)

![C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Arduino%20Uno%2FNano-green.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)
![Status](https://img.shields.io/badge/Status-Verified%20in%20Dohyo-brightgreen.svg)

Código de control autónomo para Robot de Sumo optimizado para Arduino Uno/Nano. Implementa máquina de estados finitos, lectura secuencial no bloqueante de radar ultrasónico y control de velocidad diferencial.

## Diagrama de Flujo (Máquina de Estados)

```mermaid
graph TD
    A[INICIO_RUSH] -->|350 ms| B[BUSCANDO]
    B -->|Línea detectada| C[ESCAPANDO]
    B -->|Target < 45cm x2| D[ATACANDO]
    B -->|Timeout 2.8s| E[GIRANDO180]
    D -->|Target < 12cm| F[REMATE MAX VEL]
    D -->|Pérdida de rastro > 450ms| B
    C -->|Línea despejada| E
    E -->|Giro completado 420ms| B

``` ## Especificaciones Hardware
- **Controlador:** Arduino Uno / Nano (ATmega328P)
- **Drivers de Motor:** 2x IBT-2 (H-Bridge 43A)
- **Sensores de Distancia:** 3x HC-SR04 (Izquierda, Centro, Derecha)
- **Sensores de Línea:** 2x TCRT5000 (Izquierda, Derecha)

## Asignación de Pines

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

## Máquina de Estados
1. `INICIO_RUSH`: Arranque directo tras los 5 segundos reglamentarios.
2. `BUSCANDO`: Patrón de escaneo alternado con memoria de última dirección vista.
3. `ATACANDO`: Embestida con corrección proporcional ($K_p$) para centrar al oponente.
4. `ESCAPANDO`: Maniobra de retroceso de emergencia al detectar borde blanco.
5. `GIRANDO180`: Giro rápido de desenganche.
