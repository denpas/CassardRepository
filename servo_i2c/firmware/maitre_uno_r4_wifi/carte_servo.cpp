/*
 * carte_servo.cpp - trames I2C envoyées à la carte SERVO-I2C NANO
 *
 * Implémente le protocole d'EXEMPLE de la carte :
 *   [0x01, servo, angle]        angle 0..180°
 *   [0x02, servo, usL, usH]     impulsion en µs
 *   [0x03, servo]               relâcher
 *   [0x10] puis lecture 6 oct.  [0xA5, version, us1L, us1H, us2L, us2H]
 * >>> Remplacer le contenu des fonctions par VOTRE protocole. <<<
 *
 * Bus utilisé : Wire = broches SDA/SCL du connecteur (et A4/A5), logique 5 V.
 * NE PAS utiliser le connecteur Qwiic (Wire1, 3,3 V).
 */
#include <Wire.h>
#include "config.h"
#include "carte_servo.h"

void carteInit() {
  Wire.begin();                       // maître
  Wire.setClock(I2C_FREQUENCE);
}

bool cartePresente() {
  Wire.beginTransmission(ADRESSE_CARTE);
  return Wire.endTransmission() == 0;
}

static uint8_t envoyer(const uint8_t* trame, uint8_t len) {
  Wire.beginTransmission(ADRESSE_CARTE);
  Wire.write(trame, len);
  return Wire.endTransmission();
}

uint8_t carteAngle(uint8_t servo, uint8_t angleDeg) {
  uint8_t t[3] = { 0x01, servo, angleDeg };
  return envoyer(t, sizeof t);
}

uint8_t carteMicros(uint8_t servo, uint16_t us) {
  uint8_t t[4] = { 0x02, servo, (uint8_t)(us & 0xFF), (uint8_t)(us >> 8) };
  return envoyer(t, sizeof t);
}

uint8_t carteRelacher(uint8_t servo) {
  uint8_t t[2] = { 0x03, servo };
  return envoyer(t, sizeof t);
}

uint8_t carteLireEtat(uint8_t& version, uint16_t& us1, uint16_t& us2) {
  uint8_t sel = 0x10;
  uint8_t err = envoyer(&sel, 1);
  if (err) return err;
  delay(5);                                    // laisse la carte préparer la réponse
  if (Wire.requestFrom((uint8_t)ADRESSE_CARTE, (size_t)6) != 6) return 4;
  uint8_t r[6];
  for (uint8_t i = 0; i < 6; i++) r[i] = Wire.read();
  if (r[0] != 0xA5) return 4;
  version = r[1];
  us1 = (uint16_t)r[2] | ((uint16_t)r[3] << 8);
  us2 = (uint16_t)r[4] | ((uint16_t)r[5] << 8);
  return 0;
}

const char* carteTexteErreur(uint8_t code) {
  switch (code) {
    case 0:  return "ok";
    case 1:  return "trame trop longue";
    case 2:  return "carte absente (NACK adresse)";
    case 3:  return "donnee refusee (NACK)";
    case 5:  return "delai depasse sur le bus";
    default: return "erreur bus I2C";
  }
}
