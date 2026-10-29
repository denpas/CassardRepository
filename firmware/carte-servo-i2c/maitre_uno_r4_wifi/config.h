#pragma once
// ============ Configuration du maître UNO R4 WiFi ============
// Les identifiants WiFi sont dans arduino_secrets.h

// Adresse IP fixe (recommandée pour retrouver la carte) : mettre 1 pour l'activer
#define IP_FIXE              0
#define IP_ADRESSE           192, 168, 1, 50
#define IP_PASSERELLE        192, 168, 1, 1
#define IP_MASQUE            255, 255, 255, 0
#define IP_DNS               192, 168, 1, 1

// Ports réseau
#define PORT_HTTP            80      // page de commande : http://<ip>/
#define PORT_WEBSOCKET       81      // WebSocket : ws://<ip>:81/

// Sous-protocole WebSocket renvoyé quand le client en demande un.
// "text.phpoc" permet de réutiliser une page écrite pour le shield PHPOC.
#define WS_SOUS_PROTOCOLE    "text.phpoc"

// Bus I2C vers la carte SERVO-I2C NANO (connecteur J4)
#define ADRESSE_CARTE        0x40    // doit correspondre à ADRESSE_I2C de la carte
#define I2C_FREQUENCE        100000  // 100 kHz (400000 possible si câble court)

// Retour visuel sur la matrice de LED du R4 (0 pour désactiver)
#define MATRICE_ACTIVITE     1
