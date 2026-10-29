#pragma once
// ===================== Configuration de la carte =====================

// Adresse I2C 7 bits de la carte (0x08 à 0x77). À adapter à votre protocole.
#define ADRESSE_I2C          0x40

// Broches (fixées par le circuit imprimé - ne pas modifier)
#define NB_SERVOS            2
#define PIN_SERVO1           A0     // J2
#define PIN_SERVO2           A1     // J3
#define PIN_LED_ACT          A3     // LED jaune "ACT" (D3)

// Durée du flash de la LED d'activité à chaque commande transmise (ms)
#define LED_ACT_MS           60

// Plage d'impulsion des servos (µs). 1000-2000 = standard, 500-2500 = étendu.
#define SERVO_US_MIN         500
#define SERVO_US_MAX         2500

// Taille max d'une trame (limite matérielle Wire AVR = 32 octets)
#define TRAME_MAX            32

// Sécurité : relâche les servos si aucune trame valide pendant ce délai (ms).
// 0 = désactivé.
#define TIMEOUT_SECURITE_MS  0

// Traces sur le port série USB (115200 bauds). 0 en production.
#define DEBUG_SERIE          0
