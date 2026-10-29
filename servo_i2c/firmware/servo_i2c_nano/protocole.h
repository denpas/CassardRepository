#pragma once
#include <Arduino.h>

// ---------- À implémenter (protocole.cpp) ----------
void protocoleInit();
// Appelée dans loop() pour chaque trame écrite par le maître.
// Retourner true si la trame est valide (réarme le timeout de sécurité).
bool protocoleTraiterTrame(const uint8_t* trame, uint8_t len);
// Appelée si le timeout de sécurité expire (servos déjà relâchés).
void protocoleSurTimeout();

// ---------- Services fournis par le firmware ----------
void     servoEcrireAngle(uint8_t idx, uint16_t angleDeg);   // idx 0 = servo 1
void     servoEcrireMicros(uint8_t idx, uint16_t us);
void     servoRelacher(uint8_t idx);
uint16_t servoLireMicros(uint8_t idx);
void     reponseDefinir(const uint8_t* data, uint8_t len);   // prochaine lecture maître
bool     trameAEtePerdue();
