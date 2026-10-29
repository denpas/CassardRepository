/*
 * SERVO-I2C - Maître Arduino UNO R4 WiFi  (Rev 1.2 - 2026-10-29)
 * Remplace l'ensemble Arduino UNO + shield PHPOC.
 *
 * Rôle :
 *   - se connecte au WiFi (2,4 GHz) ;
 *   - sert une page de commande sur http://<ip>/ ;
 *   - reçoit les ordres par WebSocket sur ws://<ip>:81/ ;
 *   - transmet chaque ordre à la carte SERVO-I2C NANO par I2C (connecteur J4).
 *
 * Bibliothèques :
 *   - WiFiS3, Wire, Arduino_LED_Matrix : fournies avec le paquet de cartes
 *     « Arduino UNO R4 Boards » ;
 *   - WebSockets de Markus Sattler (Links2004), version 2.5.1 ou plus récente,
 *     à installer depuis le gestionnaire de bibliothèques.
 *
 * Commandes WebSocket (texte, une par message) :
 *   S1=90      servo 1 à 90°   (S2=... pour le servo 2, 0 à 180)
 *   U1=1500    servo 1 à 1500 µs (U2=...)
 *   R1 / R2    relâcher le servo
 *   ?          lire l'état de la carte
 * Réponses : "OK ...", "ERR ..." ou "ETAT fw=.. us1=.. us2=..".
 */
#include <WiFiS3.h>
#include <WebSocketsServer.h>
#include "config.h"
#include "arduino_secrets.h"
#include "carte_servo.h"
#include "page_web.h"
#if MATRICE_ACTIVITE
#include <Arduino_LED_Matrix.h>
ArduinoLEDMatrix matrice;
// Motifs 8x12 : point fixe = en attente, cadre = commande transmise
static const uint32_t MOTIF_ATTENTE[3] = { 0x00000000, 0x00600600, 0x00000000 };
static const uint32_t MOTIF_ACTIVITE[3] = { 0xFFF80180, 0x18018018, 0x01801FFF };
static uint32_t matriceDebutMs = 0;
static bool matriceActive = false;
#endif

WiFiServer serveurHttp(PORT_HTTP);
WebSocketsServer serveurWs(PORT_WEBSOCKET, "", WS_SOUS_PROTOCOLE);

// ------------------------------------------------------------ utilitaires
static void signalerActivite() {
#if MATRICE_ACTIVITE
  matrice.loadFrame(MOTIF_ACTIVITE);
  matriceDebutMs = millis();
  matriceActive = true;
#endif
}

static void repondre(uint8_t client, const String& texte) {
  String t = texte;
  serveurWs.sendTXT(client, t);
  Serial.println(texte);
}

static void connecterWifi() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Module WiFi absent : verifier la carte UNO R4 WiFi"));
    while (true) delay(1000);
  }
#if IP_FIXE
  WiFi.config(IPAddress(IP_ADRESSE), IPAddress(IP_DNS), IPAddress(IP_PASSERELLE), IPAddress(IP_MASQUE));
#endif
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Connexion a ")); Serial.println(SECRET_SSID);
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    for (uint8_t i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) delay(500);
  }
  // L'adresse IP peut rester à 0.0.0.0 quelques instants après la connexion
  for (uint8_t i = 0; i < 20 && WiFi.localIP() == IPAddress(0, 0, 0, 0); i++) delay(250);
  Serial.print(F("Connecte. Page de commande : http://"));
  Serial.println(WiFi.localIP());
}

// ------------------------------------------------- traitement des ordres
static void traiterCommande(uint8_t client, const String& brut) {
  String c = brut; c.trim(); c.toUpperCase();
  uint8_t err;

  if (c == "?") {
    uint8_t fw; uint16_t u1, u2;
    err = carteLireEtat(fw, u1, u2);
    if (err) { repondre(client, String("ERR etat : ") + carteTexteErreur(err)); return; }
    repondre(client, String("ETAT fw=") + (fw >> 4) + "." + (fw & 0x0F) + " us1=" + u1 + " us2=" + u2);
    return;
  }
  if (c.length() >= 2 && (c[1] == '1' || c[1] == '2')) {
    uint8_t servo = c[1] - '1';                       // 0 ou 1
    long val = (c.length() > 3 && c[2] == '=') ? c.substring(3).toInt() : -1;
    switch (c[0]) {
      case 'S':
        if (val < 0 || val > 180) break;
        err = carteAngle(servo, (uint8_t)val);
        if (!err) signalerActivite();
        repondre(client, err ? String("ERR ") + carteTexteErreur(err) : String("OK ") + c);
        return;
      case 'U':
        if (val < 500 || val > 2500) break;
        err = carteMicros(servo, (uint16_t)val);
        if (!err) signalerActivite();
        repondre(client, err ? String("ERR ") + carteTexteErreur(err) : String("OK ") + c);
        return;
      case 'R':
        err = carteRelacher(servo);
        repondre(client, err ? String("ERR ") + carteTexteErreur(err) : String("OK ") + c);
        return;
    }
  }
  repondre(client, String("ERR commande inconnue : ") + brut);
}

static void surEvenementWs(uint8_t client, WStype_t type, uint8_t* donnees, size_t longueur) {
  switch (type) {
    case WStype_CONNECTED:
      // remoteIP() n'existe pas dans la bibliothèque WebSockets pour l'UNO R4 (WiFiS3).
      Serial.print(F("WebSocket : client ")); Serial.print(client); Serial.println(F(" connecte"));
      break;
    case WStype_DISCONNECTED:
      Serial.print(F("WebSocket : client ")); Serial.print(client); Serial.println(F(" deconnecte"));
      break;
    case WStype_TEXT: {
      String cmd;
      cmd.reserve(longueur);
      for (size_t i = 0; i < longueur; i++) cmd += (char)donnees[i];
      traiterCommande(client, cmd);
      break;
    }
    default:
      break;
  }
}

// --------------------------------------------------- mini serveur HTTP
static void servirHttp() {
  WiFiClient c = serveurHttp.available();
  if (!c) return;
  String ligne = c.readStringUntil('\n');            // "GET / HTTP/1.1"
  while (c.connected()) {                            // ignorer les en-têtes
    String h = c.readStringUntil('\n');
    if (h.length() <= 1) break;
  }
  if (ligne.startsWith("GET / ") || ligne.startsWith("GET /index")) {
    c.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n\r\n"));
    const size_t n = sizeof(PAGE_WEB) - 1;
    for (size_t i = 0; i < n; i += 512) {
      size_t m = (n - i < 512) ? n - i : 512;
      c.write((const uint8_t*)PAGE_WEB + i, m);
    }
  } else {
    c.print(F("HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\nIntrouvable"));
  }
  c.flush();
  c.stop();
}

// ------------------------------------------------------------------ setup
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println(F("SERVO-I2C - maitre UNO R4 WiFi"));
#if MATRICE_ACTIVITE
  matrice.begin();
  matrice.loadFrame(MOTIF_ATTENTE);
#endif
  carteInit();
  Serial.println(cartePresente() ? F("Carte servo detectee sur le bus I2C")
                                 : F("ATTENTION : carte servo non detectee (cablage J4, JP2, alimentation)"));
  connecterWifi();
  serveurHttp.begin();
  serveurWs.begin();
  serveurWs.onEvent(surEvenementWs);
}

// ------------------------------------------------------------------- loop
void loop() {
  serveurWs.loop();
  servirHttp();
#if MATRICE_ACTIVITE
  if (matriceActive && millis() - matriceDebutMs > 80) {
    matrice.loadFrame(MOTIF_ATTENTE);
    matriceActive = false;
  }
#endif
  static uint32_t dernierControle = 0;
  if (millis() - dernierControle > 10000) {          // reconnexion WiFi si besoin
    dernierControle = millis();
    if (WiFi.status() != WL_CONNECTED) connecterWifi();
  }
}
