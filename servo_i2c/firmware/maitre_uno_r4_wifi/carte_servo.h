#pragma once
#include <Arduino.h>

// Interface vers la carte SERVO-I2C NANO.
// Ces fonctions construisent les trames I2C : c'est ICI qu'il faut
// reporter VOTRE protocole (le même que dans protocole.cpp côté carte).
// Valeur de retour : 0 = succès, sinon code d'erreur Wire (1 à 5).

void    carteInit();
bool    cartePresente();
uint8_t carteAngle(uint8_t servo, uint8_t angleDeg);       // servo = 0 ou 1
uint8_t carteMicros(uint8_t servo, uint16_t us);
uint8_t carteRelacher(uint8_t servo);
uint8_t carteLireEtat(uint8_t& version, uint16_t& us1, uint16_t& us2);
const char* carteTexteErreur(uint8_t code);
