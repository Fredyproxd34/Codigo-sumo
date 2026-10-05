// --- CONFIGURACIÓN DE PINES PWM (Arduino Uno/Nano) ---
const uint8_t RPWM_IZQ = 5;  // PWM
const uint8_t LPWM_IZQ = 6;  // PWM
const uint8_t R_EN_IZQ = 7; 
const uint8_t L_EN_IZQ = 8;

const uint8_t RPWM_DER = 9;  // PWM
const uint8_t LPWM_DER = 10; // PWM
const uint8_t R_EN_DER = 3;  
const uint8_t L_EN_DER = 4;  

const uint8_t TRIG_C = 11; const uint8_t ECHO_C = 12;
const uint8_t TRIG_I = A0; const uint8_t ECHO_I = A1;
const uint8_t TRIG_D = A4; const uint8_t ECHO_D = A5;
const uint8_t LINEA_I = A3; const uint8_t LINEA_D = A2;

// --- PARÁMETROS TÁCTICOS ---
const uint8_t DIST_VISION = 70;
const uint8_t DIST_ATAQUE = 45;
const uint8_t DIST_CONTACTO = 12;
const uint8_t VEL_ATAQUE_BASE = 190;
const uint8_t VEL_MAX = 255;
const uint8_t VEL_BUSQUEDA = 145;
const float Kp = 3.0;

enum Estado { INICIO_RUSH, BUSCANDO, ATACANDO, ESCAPANDO, GIRANDO180 };
Estado estadoActual = INICIO_RUSH;

unsigned long tUltimoRadar = 0, tUltimoAvistamiento = 0, tEstado = 0, tLinea = 0, tBusqueda = 0;
uint8_t turno = 0, confirmacion = 0;
int dC = 999, dI = 999, dD = 999;
int8_t memoriaDireccion = 1;

void setup() {
  uint8_t salidas[] = {RPWM_IZQ, LPWM_IZQ, R_EN_IZQ, L_EN_IZQ, RPWM_DER, LPWM_DER, R_EN_DER, L_EN_DER, TRIG_C, TRIG_I, TRIG_D};
  for(uint8_t p : salidas) pinMode(p, OUTPUT);
  pinMode(ECHO_C, INPUT); pinMode(ECHO_I, INPUT); pinMode(ECHO_D, INPUT);
  pinMode(LINEA_I, INPUT); pinMode(LINEA_D, INPUT);

  // Habilitar drivers IBT-2
  digitalWrite(R_EN_IZQ, HIGH); digitalWrite(L_EN_IZQ, HIGH);
  digitalWrite(R_EN_DER, HIGH); digitalWrite(L_EN_DER, HIGH);

  delay(5000); // 5s reglamentarios
  tEstado = tUltimoAvistamiento = tUltimoRadar = tBusqueda = millis();
}

void loop() {
  // 1. RADAR SECUENCIAL NO BLOQUEANTE (3500us timeout = ~60cm)
  if (millis() - tUltimoRadar >= 12) { 
    tUltimoRadar = millis();
    switch(turno++ % 3) {
      case 0: dC = leerDistancia(TRIG_C, ECHO_C); break;
      case 1: dI = leerDistancia(TRIG_I, ECHO_I); break;
      case 2: dD = leerDistancia(TRIG_D, ECHO_D); break;
    }
  }

  // 2. SEGURIDAD DE LÍNEA (Prioridad alta)
  bool sL_I = (digitalRead(LINEA_I) == LOW);
  bool sL_D = (digitalRead(LINEA_D) == LOW);
  
  if ((sL_I || sL_D) && estadoActual != ESCAPANDO) {
    estadoActual = ESCAPANDO;
    tLinea = millis();
  }

  // 3. MÁQUINA DE ESTADOS
  switch (estadoActual) {
    case INICIO_RUSH:
      motores(VEL_MAX, VEL_MAX);
      if (millis() - tEstado > 350) {
        estadoActual = BUSCANDO;
        tBusqueda = millis();
      }
      break;

    case BUSCANDO:
      if (millis() - tBusqueda < 600) {
        motores(VEL_BUSQUEDA * memoriaDireccion, -VEL_BUSQUEDA * memoriaDireccion);
      } else if (millis() - tBusqueda < 950) {
        motores(VEL_BUSQUEDA, VEL_BUSQUEDA);
      } else {
        tBusqueda = millis();
      }
      
      if (dC < DIST_VISION || dI < DIST_VISION || dD < DIST_VISION) {
        tUltimoAvistamiento = millis();
        
        if (dI < DIST_VISION) memoriaDireccion = -1;
        if (dD < DIST_VISION) memoriaDireccion = 1;

        if (dC < DIST_ATAQUE || dI < DIST_ATAQUE || dD < DIST_ATAQUE) {
          confirmacion++;
          if (confirmacion >= 2) estadoActual = ATACANDO;
        } else {
          confirmacion = 0;
        }
      } else {
        if (millis() - tUltimoAvistamiento > 2800) {
          estadoActual = GIRANDO180;
          tEstado = millis();
        }
      }
      break;

    case ATACANDO:
      if (dI < DIST_ATAQUE) memoriaDireccion = -1;
      if (dD < DIST_ATAQUE) memoriaDireccion = 1;

      if (dC < DIST_CONTACTO || dI < 8 || dD < 8) {
        motores(VEL_MAX, VEL_MAX);
      } else {
        // Restaurado: dI - dD para giro correcto hacia el oponente
        int error = dI - dD; 
        int ajuste = constrain(error * Kp, -70, 70);
        motores(VEL_ATAQUE_BASE + ajuste, VEL_ATAQUE_BASE - ajuste);
      }

      if (dC > DIST_ATAQUE && dI > DIST_ATAQUE && dD > DIST_ATAQUE) {
        if (millis() - tUltimoAvistamiento > 450) {
          estadoActual = BUSCANDO;
          tBusqueda = millis();
        }
      } else {
        tUltimoAvistamiento = millis();
      }
      break;

    case ESCAPANDO:
      if ((digitalRead(LINEA_I) == LOW || digitalRead(LINEA_D) == LOW) || (millis() - tLinea < 350)) {
        if (digitalRead(LINEA_I) == LOW) motores(-VEL_MAX, -VEL_BUSQUEDA);
        else motores(-VEL_BUSQUEDA, -VEL_MAX);
      } else {
        estadoActual = GIRANDO180;
        tEstado = millis();
      }
      break;

    case GIRANDO180:
      if (millis() - tEstado < 420) {
        motores(VEL_MAX * memoriaDireccion, -VEL_MAX * memoriaDireccion);
      } else {
        estadoActual = BUSCANDO;
        tBusqueda = millis();
        tUltimoAvistamiento = millis();
        confirmacion = 0; // Reinicio de confirmación al terminar giro
      }
      break;
  }
}

void motores(int vI, int vD) {
  vI = constrain(vI, -255, 255);
  vD = constrain(vD, -255, 255);

  if (vI >= 0) { 
    analogWrite(RPWM_IZQ, vI); 
    analogWrite(LPWM_IZQ, 0); 
  } else { 
    analogWrite(RPWM_IZQ, 0); 
    analogWrite(LPWM_IZQ, abs(vI)); 
  }

  if (vD >= 0) { 
    analogWrite(RPWM_DER, vD); 
    analogWrite(LPWM_DER, 0); 
  } else { 
    analogWrite(RPWM_DER, 0); 
    analogWrite(LPWM_DER, abs(vD)); 
  }
}

int leerDistancia(int trig, int echo) {
  digitalWrite(trig, LOW); 
  delayMicroseconds(2);
  digitalWrite(trig, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  long d = pulseIn(echo, HIGH, 3500); 
  return (d == 0) ? 999 : d / 58;
}