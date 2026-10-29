/*
 * SERVO-I2C NANO - Firmware de base  (Rev 1.1 - 2026-10-27)
 * Carte : SERVO-I2C NANO v1.1  /  Arduino Nano (ATmega328P, 16 MHz, 5 V)
 *
 * Rôle : esclave I2C qui pilote 2 servos.
 *   - Servo 1 : broche A0  -> connecteur J2
 *   - Servo 2 : broche A1  -> connecteur J3
 *   - I2C     : A4 = SDA, A5 = SCL -> connecteur J4
 *   - LED ACT : A3 (D3) - flash à chaque commande transmise à un servo
 *
 * >>> VOTRE PROTOCOLE <<<
 *   Tout ce qui est propre à votre protocole est regroupé dans
 *   protocole.h / protocole.cpp :
 *     - protocoleTraiterTrame()  : appelée dans loop() pour chaque trame reçue
 *     - protocolePreparerReponse(): appelée quand le maître lit (requestFrom)
 *   Le fichier fourni contient un protocole d'EXEMPLE à remplacer.
 *
 * Bonnes pratiques respectées :
 *   - les callbacks I2C (sous interruption) ne font que copier les octets ;
 *     le traitement a lieu dans loop()
 *   - le tampon Wire de l'AVR est limité à 32 octets par transaction
 *   - sécurité optionnelle : coupure des servos si plus de trame valide
 */
#include <Wire.h>
#include <Servo.h>
#include "config.h"
#include "protocole.h"

Servo servos[NB_SERVOS];
static const uint8_t SERVO_PINS[NB_SERVOS] = { PIN_SERVO1, PIN_SERVO2 };

// ---- tampon de réception (rempli sous interruption) ----
static volatile uint8_t rxBuf[TRAME_MAX];
static volatile uint8_t rxLen = 0;
static volatile bool    rxPret = false;
static volatile bool    rxPerdue = false;

// ---- tampon de réponse (préparé dans loop, envoyé sous interruption) ----
static volatile uint8_t txBuf[TRAME_MAX];
static volatile uint8_t txLen = 0;

static uint32_t derniereTrameMs = 0;
static bool servosActifs = false;
static uint32_t ledActDebutMs = 0;
static bool ledActAllumee = false;

// Allume la LED d'activité (et la LED L du Nano) pour LED_ACT_MS ms, sans bloquer
static void ledActivite() {
  digitalWrite(PIN_LED_ACT, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  ledActDebutMs = millis();
  ledActAllumee = true;
}

// ------------------------------------------------------------------ servos
void servoActiver(uint8_t idx) {
  if (idx >= NB_SERVOS) return;
  if (!servos[idx].attached()) servos[idx].attach(SERVO_PINS[idx], SERVO_US_MIN, SERVO_US_MAX);
  servosActifs = true;
}

void servoEcrireAngle(uint8_t idx, uint16_t angleDeg) {
  if (idx >= NB_SERVOS) return;
  if (angleDeg > 180) angleDeg = 180;
  servoActiver(idx);
  servos[idx].write(angleDeg);
  ledActivite();
}

void servoEcrireMicros(uint8_t idx, uint16_t us) {
  if (idx >= NB_SERVOS) return;
  if (us < SERVO_US_MIN) us = SERVO_US_MIN;
  if (us > SERVO_US_MAX) us = SERVO_US_MAX;
  servoActiver(idx);
  servos[idx].writeMicroseconds(us);
  ledActivite();
}

void servoRelacher(uint8_t idx) {          // plus d'impulsion : servo libre
  if (idx >= NB_SERVOS) return;
  servos[idx].detach();
}

uint16_t servoLireMicros(uint8_t idx) {
  if (idx >= NB_SERVOS || !servos[idx].attached()) return 0;
  return servos[idx].readMicroseconds();
}

// --------------------------------------------------------- callbacks I2C
void onI2cReceive(int nbOctets) {
  (void)nbOctets;
  if (rxPret) {                        // trame précédente pas encore traitée
    while (Wire.available()) Wire.read();
    rxPerdue = true;
    return;
  }
  uint8_t i = 0;
  while (Wire.available()) {
    uint8_t b = Wire.read();
    if (i < TRAME_MAX) rxBuf[i++] = b;
  }
  rxLen = i;
  rxPret = (i > 0);
}

void onI2cRequest() {
  if (txLen == 0) { Wire.write((uint8_t)0x00); return; }
  Wire.write((const uint8_t*)txBuf, txLen);
}

// API utilisée par protocole.cpp pour préparer la réponse
void reponseDefinir(const uint8_t* data, uint8_t len) {
  if (len > TRAME_MAX) len = TRAME_MAX;
  noInterrupts();
  for (uint8_t i = 0; i < len; i++) txBuf[i] = data[i];
  txLen = len;
  interrupts();
}

bool trameAEtePerdue() { bool p = rxPerdue; rxPerdue = false; return p; }

// ------------------------------------------------------------------ setup
void setup() {
#if DEBUG_SERIE
  Serial.begin(115200);
  Serial.println(F("SERVO-I2C NANO - demarrage"));
#endif
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_LED_ACT, OUTPUT);
  digitalWrite(PIN_LED_ACT, LOW);
  Wire.begin(ADRESSE_I2C);             // mode esclave
  Wire.onReceive(onI2cReceive);
  Wire.onRequest(onI2cRequest);
  // Pas d'attach() au démarrage : les servos restent libres tant qu'aucun
  // ordre n'est reçu (évite un mouvement brusque à la mise sous tension).
  protocoleInit();
  derniereTrameMs = millis();
}

// ------------------------------------------------------------------- loop
void loop() {
  if (rxPret) {
    uint8_t local[TRAME_MAX];
    uint8_t len;
    noInterrupts();
    len = rxLen;
    for (uint8_t i = 0; i < len; i++) local[i] = rxBuf[i];
    rxPret = false;
    interrupts();

    if (protocoleTraiterTrame(local, len)) {
      derniereTrameMs = millis();
    }
#if DEBUG_SERIE
    Serial.print(F("RX ")); Serial.print(len); Serial.print(F(" o :"));
    for (uint8_t i = 0; i < len; i++) { Serial.print(' '); Serial.print(local[i], HEX); }
    Serial.println();
#endif
  }

  if (ledActAllumee && (millis() - ledActDebutMs >= LED_ACT_MS)) {
    digitalWrite(PIN_LED_ACT, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    ledActAllumee = false;
  }

#if TIMEOUT_SECURITE_MS > 0
  if (servosActifs && (millis() - derniereTrameMs > TIMEOUT_SECURITE_MS)) {
    for (uint8_t i = 0; i < NB_SERVOS; i++) servoRelacher(i);
    servosActifs = false;
    protocoleSurTimeout();
  }
#endif
}
