#include <Arduino.h>

const uint8_t RPWM_IZQ = 5;
const uint8_t LPWM_IZQ = 6;
const uint8_t RPWM_DER = 9;
const uint8_t LPWM_DER = 10;

const uint8_t SENSOR_LINEA_IZQ = A3;
const uint8_t SENSOR_LINEA_DER = A2;

const uint8_t TRIG_CEN = 11, ECHO_CEN = 12;
const uint8_t TRIG_IZQ = A0, ECHO_IZQ = A1;
const uint8_t TRIG_DER = A4, ECHO_DER = A5;

const int VELOCIDAD_MAXIMA = 200;
const int VELOCIDAD_GIRO   = 150; 

enum EstadoRobot {
  INICIO_RUSH,
  BUSCANDO,
  ATACANDO,
  ESCAPANDO
};

EstadoRobot estadoActual = INICIO_RUSH;

unsigned long tiempoInicio = 0;
unsigned long tiempoEstado = 0;
const unsigned long TIEMPO_ESPERA_REGLAMENTO = 5000;
const int DISTANCIA_UMBRAL_CM = 45;

long distCen = 999, distIzq = 999, distDer = 999;

void controlarMotores(int velIzq, int velDer);
void controlarDriver(uint8_t rPwm, uint8_t lPwm, int velocidad);
long medirDistanciaCM(uint8_t trigPin, uint8_t echoPin);
bool detectarLinea();
void actualizarSensores();

void setup() {
  pinMode(RPWM_IZQ, OUTPUT);
  pinMode(LPWM_IZQ, OUTPUT);
  pinMode(RPWM_DER, OUTPUT);
  pinMode(LPWM_DER, OUTPUT);

  pinMode(SENSOR_LINEA_IZQ, INPUT);
  pinMode(SENSOR_LINEA_DER, INPUT);

  pinMode(TRIG_CEN, OUTPUT); pinMode(ECHO_CEN, INPUT);
  pinMode(TRIG_IZQ, OUTPUT); pinMode(ECHO_IZQ, INPUT);
  pinMode(TRIG_DER, OUTPUT); pinMode(ECHO_DER, INPUT);

  tiempoInicio = millis();
}

void loop() {
  if (detectarLinea() && estadoActual != ESCAPANDO) {
    estadoActual = ESCAPANDO;
    tiempoEstado = millis();
  }

  switch (estadoActual) {

    case INICIO_RUSH:
      if (millis() - tiempoInicio >= TIEMPO_ESPERA_REGLAMENTO) {
        controlarMotores(VELOCIDAD_MAXIMA, VELOCIDAD_MAXIMA);
        delay(350); 
        estadoActual = BUSCANDO;
      } else {
        controlarMotores(0, 0);
      }
      break;

    case BUSCANDO:
      actualizarSensores();
      if (distCen < DISTANCIA_UMBRAL_CM || distIzq < DISTANCIA_UMBRAL_CM || distDer < DISTANCIA_UMBRAL_CM) {
        estadoActual = ATACANDO;
      } else {
        controlarMotores(VELOCIDAD_GIRO, -VELOCIDAD_GIRO);
      }
      break;

    case ATACANDO:
      actualizarSensores();
      if (distCen < DISTANCIA_UMBRAL_CM) {
        controlarMotores(VELOCIDAD_MAXIMA, VELOCIDAD_MAXIMA);
      } else if (distIzq < DISTANCIA_UMBRAL_CM) {
        controlarMotores(VELOCIDAD_GIRO / 2, VELOCIDAD_MAXIMA);
      } else if (distDer < DISTANCIA_UMBRAL_CM) {
        controlarMotores(VELOCIDAD_MAXIMA, VELOCIDAD_GIRO / 2);
      } else {
        estadoActual = BUSCANDO;
      }
      break;

    case ESCAPANDO: {
      unsigned long transcurrido = millis() - tiempoEstado;
      if (transcurrido < 350) {
        controlarMotores(-VELOCIDAD_MAXIMA, -VELOCIDAD_MAXIMA);
      } else if (transcurrido < 650) {
        controlarMotores(VELOCIDAD_MAXIMA, -VELOCIDAD_MAXIMA);
      } else {
        estadoActual = BUSCANDO;
      }
      break;
    }
  }
}

void actualizarSensores() {
  distCen = medirDistanciaCM(TRIG_CEN, ECHO_CEN);
  distIzq = medirDistanciaCM(TRIG_IZQ, ECHO_IZQ);
  distDer = medirDistanciaCM(TRIG_DER, ECHO_DER);
}

void controlarMotores(int velIzq, int velDer) {
  controlarDriver(RPWM_IZQ, LPWM_IZQ, velIzq);
  controlarDriver(RPWM_DER, LPWM_DER, velDer);
}

void controlarDriver(uint8_t rPwm, uint8_t lPwm, int velocidad) {
  velocidad = constrain(velocidad, -VELOCIDAD_MAXIMA, VELOCIDAD_MAXIMA);
  if (velocidad >= 0) {
    analogWrite(rPwm, velocidad);
    analogWrite(lPwm, 0);
  } else {
    analogWrite(rPwm, 0);
    analogWrite(lPwm, abs(velocidad));
  }
}

long medirDistanciaCM(uint8_t trigPin, uint8_t echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracion = pulseIn(echoPin, HIGH, 18000);
  if (duracion == 0) return 999;
  return duracion * 0.034 / 2;
}

bool detectarLinea() {
  return (digitalRead(SENSOR_LINEA_IZQ) == HIGH || digitalRead(SENSOR_LINEA_DER) == HIGH);
}
