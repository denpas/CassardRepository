/*
 * protocole.cpp - IMPLÉMENTATION DE VOTRE PROTOCOLE I2C
 *
 * Ce fichier contient un protocole d'EXEMPLE pour tester la carte.
 * Remplacez le contenu de protocoleTraiterTrame() par le décodage de
 * votre propre protocole, en utilisant les services de protocole.h.
 *
 * Protocole d'exemple (registre + données, little-endian) :
 *   Écriture [0x01, servo, angle]           -> angle 0..180°   (servo = 0 ou 1)
 *   Écriture [0x02, servo, usL, usH]        -> impulsion en µs
 *   Écriture [0x03, servo]                  -> relâcher le servo
 *   Écriture [0x10]  puis lecture 6 octets  -> [0xA5, version, us1L, us1H, us2L, us2H]
 */
#include "protocole.h"

static const uint8_t VERSION_FW = 0x10;   // 1.0

static void majReponseEtat() {
  uint16_t u1 = servoLireMicros(0), u2 = servoLireMicros(1);
  uint8_t r[6] = { 0xA5, VERSION_FW, (uint8_t)u1, (uint8_t)(u1 >> 8), (uint8_t)u2, (uint8_t)(u2 >> 8) };
  reponseDefinir(r, sizeof r);
}

void protocoleInit() {
  majReponseEtat();
}

bool protocoleTraiterTrame(const uint8_t* t, uint8_t len) {
  // ====== >>> REMPLACER PAR VOTRE PROTOCOLE <<< ======
  if (len < 1) return false;
  switch (t[0]) {
    case 0x01:
      if (len < 3 || t[1] > 1) return false;
      servoEcrireAngle(t[1], t[2]);
      break;
    case 0x02:
      if (len < 4 || t[1] > 1) return false;
      servoEcrireMicros(t[1], (uint16_t)t[2] | ((uint16_t)t[3] << 8));
      break;
    case 0x03:
      if (len < 2 || t[1] > 1) return false;
      servoRelacher(t[1]);
      break;
    case 0x10:
      break;                       // simple sélection du registre d'état
    default:
      return false;
  }
  majReponseEtat();
  return true;
  // ====================================================
}

void protocoleSurTimeout() {
  majReponseEtat();
}
