// Réglages Wi-Fi du module central (Arduino UNO R4 WiFi).
// Copier ce fichier en CassardWifi.h (non versionné) et le compléter.
#ifndef CASSARD_WIFI_H
#define CASSARD_WIFI_H

// 1 : la carte crée son propre réseau (point d'accès WPA2, adresse 192.168.4.1),
//     à utiliser au bassin ;
// 0 : la carte rejoint un réseau existant (box, en 2,4 GHz), adresse attribuée
//     par la box et affichée sur le moniteur série au démarrage.
#define CASSARD_WIFI_POINT_ACCES 1

#define CASSARD_WIFI_SSID "cassard"
// Mot de passe WPA2 : au moins 8 caractères, différent du mot de passe de pilotage.
#define CASSARD_WIFI_MOT_DE_PASSE "a-remplacer"

// Canal du point d'accès (2,4 GHz : 1 à 13). Choisir un canal peu encombré :
// sur le canal 1, voisin d'autres réseaux, le point d'accès n'était plus émis.
#define CASSARD_WIFI_CANAL 6

#endif
