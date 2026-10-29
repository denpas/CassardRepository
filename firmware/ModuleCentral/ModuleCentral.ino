// Module central du Cassard — Arduino UNO R4 WiFi.
//
// Sert l'interface web (port 80, pages compressées en flash dans PagesWeb.h)
// et le WebSocket de pilotage (port 81). Chaque trame signée est vérifiée
// (CassardAuth.h), validée, puis transmise par I2C au module concerné
// (adresse 1 tourelle, 2 propulsion).
//
// Bibliothèques : paquet « Arduino UNO R4 Boards » (WiFiS3, Wire,
// Arduino_LED_Matrix) et WebSockets de Markus Sattler (Links2004) >= 2.7.

#include <CassardControl.h>
#include <Wire.h>
#include <WiFiS3.h>
#include <WebSocketsServer.h>
#include <Arduino_LED_Matrix.h>
#include <CassardControl/Protocol.h>
#include "CassardAuth.h"

#if !__has_include("CassardSecret.h")
#error "Cle de pilotage absente : lancer python3 outils/generer-cle-pilotage.py"
#endif
#include "CassardSecret.h"
#if !__has_include("CassardWifi.h")
#error "Reglages Wi-Fi absents : copier CassardWifi.exemple.h en CassardWifi.h et le completer"
#endif
#include "CassardWifi.h"
#if !__has_include("PagesWeb.h")
#error "Pages web absentes : lancer python3 outils/generer-pages-web.py"
#endif
#include "PagesWeb.h"

#ifndef CASSARD_WIFI_CANAL
#define CASSARD_WIFI_CANAL 6
#endif

#if CASSARD_WIFI_POINT_ACCES
static_assert(sizeof(CASSARD_WIFI_MOT_DE_PASSE) - 1 >= 8, "WPA2 : mot de passe Wi-Fi d'au moins 8 caracteres");
#endif

const uint16_t HttpPort = 80;
const uint16_t WebSocketPort = 81;
const uint8_t NoClient = 0xFF;
const byte MaxCommandLength = 5;
const byte MaxFrameLength = 64;
const byte MaxPayloadLength = 24;

WiFiServer httpServer(HttpPort);
WebSocketsServer webSocket(WebSocketPort, "", "cassard");
ArduinoLEDMatrix matrix;

// Session de pilotage : un seul poste à la fois, lié à son client WebSocket.
uint8_t sessionClient = NoClient;
char sessionNonce[CassardNonceLength * 2 + 1];
uint32_t lastSequence = 0;
uint8_t entropyPool[32];

// Matrice de LED (8 lignes x 12 colonnes). Elle n'est redessinée que si son
// contenu change, au plus toutes les 20 ms (un rendu coûte environ 70 us) :
// l'affichage ne doit jamais freiner le pilotage.
//  lignes 0-1             : un carré 2x2 par client de l'application (WebSocket), 4 au plus ;
//  lignes 6-7             : un carré 2x2 par appareil du Wi-Fi, 4 au plus. Le module Wi-Fi
//                           ne sait pas lister les appareils associés : on compte ceux
//                           qui ont chargé une page depuis moins de 5 minutes ;
//  lignes 3-4             : un bloc 2x2 qui avance d'une colonne (0 à 9) à chaque trame
//                           valide, visible pendant 1 s après la dernière ;
//  colonne 11, lignes 2-5 : trame refusée dans les 2 dernières secondes ;
//  ligne 0, colonne 11    : témoin de vie, clignote à 1 Hz.
const byte SquareColumns[4] = { 0, 3, 6, 9 };
const byte MaxWifiDevices = 4;
const uint32_t WifiDeviceTimeoutMs = 300000;
uint8_t matrixPixels[8][12];
bool matrixDirty = true;
bool sessionReady = false;      // mot de passe validé (PING signé accepté)
byte trafficStep = 0;
bool trafficSeen = false;
uint32_t lastFrameMs = 0;
bool errorSeen = false;
uint32_t lastErrorMs = 0;
uint32_t wifiDeviceIp[MaxWifiDevices];
uint32_t wifiDeviceSeenMs[MaxWifiDevices];

bool parseByteField(const char* frame, byte& position, byte& value)
{
  if (frame[position] < '0' || frame[position] > '9')
    return false;

  unsigned int parsed = 0;
  while (frame[position] >= '0' && frame[position] <= '9')
  {
    parsed = parsed * 10 + frame[position] - '0';
    if (parsed > 255)
      return false;
    position++;
  }

  value = (byte)parsed;
  return true;
}

byte expectedCommandLength(const byte* command, byte fieldCount)
{
  if (fieldCount < 3 || command[fieldCount - 1] != END_OF_TRAME)
    return 0;

  if (command[0] == MODULE_TOURELLE &&
      command[1] == TOURELLE_SETAZIMUTELEVATION)
    return fieldCount == 5 && command[2] <= 180 && command[3] <= 80 ? 5 : 0;

  if (command[0] == MODULE_PROPULSION)
  {
    if (command[1] == PROPULSION_STOPENGINE ||
        command[1] == PROPULSION_GOSTRAIGHT)
      return fieldCount == 3 ? 3 : 0;
    if (command[1] == PROPULSION_FORWARDENGINE ||
        command[1] == PROPULSION_REVERSEENGINE)
      return fieldCount == 4 && command[2] <= 100 ? 4 : 0;
    if (command[1] == PROPULSION_TURNRIGHT ||
        command[1] == PROPULSION_TURNLEFT)
      return fieldCount == 4 && command[2] <= 90 ? 4 : 0;
    if (command[1] == PROPULSION_SETPROPORTIONALMODE)
      return fieldCount == 4 && command[2] <= 5 ? 4 : 0;
  }

  return 0;
}

bool parseCommand(const char* frame, byte* command, byte& fieldCount)
{
  if (strlen(frame) > 19)
    return false;

  fieldCount = 0;
  byte position = 0;

  while (frame[position])
  {
    if (fieldCount >= MaxCommandLength ||
        !parseByteField(frame, position, command[fieldCount]))
      return false;
    fieldCount++;

    if (!frame[position])
      break;
    if (frame[position] != ',')
      return false;
    position++;
    if (!frame[position])
      return false;
  }

  if (fieldCount == 0 ||
      (command[0] != MODULE_TOURELLE && command[0] != MODULE_PROPULSION) ||
      expectedCommandLength(command, fieldCount) != fieldCount)
    return false;

  return true;
}

void forwardCommand(const byte* command, byte fieldCount)
{
  Wire.beginTransmission(command[0]);
  for (byte field = 0; field < fieldCount; field++)
    Wire.write(command[field]);
  Wire.endTransmission();
}

void sendSafeStop()
{
  const byte stopEngine[] = { MODULE_PROPULSION, PROPULSION_STOPENGINE, END_OF_TRAME };
  const byte goStraight[] = { MODULE_PROPULSION, PROPULSION_GOSTRAIGHT, END_OF_TRAME };
  forwardCommand(stopEngine, sizeof(stopEngine));
  forwardCommand(goStraight, sizeof(goStraight));
}

// Mélange le bruit des entrées analogiques libres (A4/A5 servent à l'I2C)
// et la gigue de micros() dans un réservoir haché.
void newSessionNonce()
{
  CassardSha256 sha;
  cassardSha256Begin(sha);
  cassardSha256Update(sha, entropyPool, sizeof(entropyPool));
  for (byte round = 0; round < 32; round++)
  {
    uint16_t samples[5] = { (uint16_t)analogRead(A0), (uint16_t)analogRead(A1),
                            (uint16_t)analogRead(A2), (uint16_t)analogRead(A3),
                            (uint16_t)micros() };
    cassardSha256Update(sha, (const uint8_t*)samples, sizeof(samples));
  }
  uint32_t now = micros();
  cassardSha256Update(sha, (const uint8_t*)&now, sizeof(now));
  cassardSha256End(sha, entropyPool);
  cassardToHex(entropyPool, CassardNonceLength, sessionNonce);
  lastSequence = 0;
}

void reply(uint8_t client, const char* message)
{
  webSocket.sendTXT(client, message);
}

void noteFrame()
{
  trafficStep = (trafficStep + 1) % 9;
  trafficSeen = true;
  lastFrameMs = millis();
  matrixDirty = true;
}

void noteError()
{
  errorSeen = true;
  lastErrorMs = millis();
  matrixDirty = true;
}

// Appareil vu sur le Wi-Fi (page chargée) : on garde les 4 plus récents.
void noteWifiDevice(uint32_t ip, uint32_t now)
{
  byte slot = 0;
  for (byte i = 0; i < MaxWifiDevices; i++)
  {
    if (wifiDeviceIp[i] == ip)
    {
      slot = i;
      break;
    }
    if (wifiDeviceSeenMs[i] < wifiDeviceSeenMs[slot])
      slot = i;
  }
  if (wifiDeviceIp[slot] != ip)
    matrixDirty = true;
  wifiDeviceIp[slot] = ip;
  wifiDeviceSeenMs[slot] = now;
}

byte wifiDeviceCount(uint32_t now)
{
  byte count = 0;
  for (byte i = 0; i < MaxWifiDevices; i++)
    if (wifiDeviceIp[i] != 0 && now - wifiDeviceSeenMs[i] < WifiDeviceTimeoutMs)
      count++;
  return count;
}

void drawSquares(byte firstRow, byte count)
{
  for (byte i = 0; i < count && i < 4; i++)
    matrixPixels[firstRow][SquareColumns[i]] = matrixPixels[firstRow][SquareColumns[i] + 1] =
      matrixPixels[firstRow + 1][SquareColumns[i]] = matrixPixels[firstRow + 1][SquareColumns[i] + 1] = 1;
}

void updateMatrix()
{
  static uint32_t lastRenderMs = 0;
  static byte shownClients = 255;
  static byte shownDevices = 255;
  static bool shownAlive = false;
  static bool shownTraffic = false;
  static bool shownError = false;

  uint32_t now = millis();
  if (now - lastRenderMs < 20)
    return;

  byte clients = webSocket.connectedClients();
  byte devices = wifiDeviceCount(now);
  bool alive = (now / 500) & 1;
  bool traffic = trafficSeen && now - lastFrameMs < 1000;
  bool error = errorSeen && now - lastErrorMs < 2000;
  if (!matrixDirty && clients == shownClients && devices == shownDevices && alive == shownAlive &&
      traffic == shownTraffic && error == shownError)
    return;

  memset(matrixPixels, 0, sizeof(matrixPixels));
  drawSquares(0, clients);
  drawSquares(6, devices);
  if (traffic)
    matrixPixels[3][trafficStep] = matrixPixels[3][trafficStep + 1] =
      matrixPixels[4][trafficStep] = matrixPixels[4][trafficStep + 1] = 1;
  if (error)
    for (byte row = 2; row <= 5; row++)
      matrixPixels[row][11] = 1;
  matrixPixels[0][11] = alive;
  matrix.renderBitmap(matrixPixels, 8, 12);

  shownClients = clients;
  shownDevices = devices;
  shownAlive = alive;
  shownTraffic = traffic;
  shownError = error;
  matrixDirty = false;
  lastRenderMs = now;
}

void endSession()
{
  sendSafeStop();
  sessionClient = NoClient;
  sessionReady = false;
}

void handleFrame(uint8_t client, const uint8_t* data, size_t length)
{
  while (length > 0 && (data[length - 1] == '\r' || data[length - 1] == '\n' || data[length - 1] == ' '))
    length--;
  if (length == 0)
    return;
  if (length > MaxFrameLength)
  {
    noteError();
    reply(client, "AUTH,ERR");
    return;
  }
  char frame[MaxFrameLength + 1];
  memcpy(frame, data, length);
  frame[length] = 0;

  // Nouvelle session : l'ancienne devient invalide, on met le navire en sécurité.
  if (strcmp(frame, "AUTH") == 0)
  {
    if (sessionClient != NoClient)
    {
      sendSafeStop();
      if (sessionClient != client)
        reply(sessionClient, "AUTH,REPRISE");
    }
    newSessionNonce();
    sessionClient = client;
    sessionReady = false;
    char message[6 + sizeof(sessionNonce)];
    strcpy(message, "NONCE,");
    strcat(message, sessionNonce);
    reply(client, message);
    return;
  }

  char payload[MaxPayloadLength];
  if (client != sessionClient ||
      !cassardVerifyFrame(CassardPilotKey, sessionNonce, lastSequence, frame, payload, sizeof(payload)))
  {
    noteError();
    reply(client, "AUTH,ERR");
    return;
  }

  if (strcmp(payload, "PING") == 0)
  {
    sessionReady = true;
    noteFrame();
    reply(client, "AUTH,OK");
    return;
  }

  byte command[MaxCommandLength];
  byte fieldCount;
  if (parseCommand(payload, command, fieldCount))
  {
    forwardCommand(command, fieldCount);
    noteFrame();
  }
  else
    noteError();   // signée mais invalide (commande, longueur ou valeur)
}

void onWebSocketEvent(uint8_t client, WStype_t type, uint8_t* data, size_t length)
{
  switch (type)
  {
    case WStype_TEXT:
      handleFrame(client, data, length);
      break;
    case WStype_DISCONNECTED:
      // Liaison du pilote perdue (fermeture, ou plus de réponse au ping :
      // navire hors de portée) : arrêt moteur et barre au centre.
      if (client == sessionClient)
      {
        Serial.println(F("Liaison du pilote perdue : arret de securite"));
        endSession();
      }
      break;
    default:
      break;
  }
}

const CassardWebFile* findWebFile(const String& path)
{
  for (uint8_t i = 0; i < CassardWebFileCount; i++)
    if (path == CassardWebFiles[i].path)
      return &CassardWebFiles[i];
  return nullptr;
}

// Renvoie true si une requête a été servie.
bool serveHttp()
{
  WiFiClient client = httpServer.available();
  if (!client)
    return false;
  noteWifiDevice((uint32_t)client.remoteIP(), millis());

  client.setTimeout(1000);
  String requestLine = client.readStringUntil('\n');
  while (client.connected())
  {
    String header = client.readStringUntil('\n');
    if (header.length() <= 1)
      break;
  }

  // "GET /chemin?requete HTTP/1.1"
  const CassardWebFile* file = nullptr;
  if (requestLine.startsWith("GET "))
  {
    int end = requestLine.indexOf(' ', 4);
    String path = requestLine.substring(4, end < 0 ? requestLine.length() : end);
    int query = path.indexOf('?');
    if (query >= 0)
      path = path.substring(0, query);
    file = findWebFile(path == "/" ? String("/index.html") : path);
  }

  if (!file)
  {
    client.print(F("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nIntrouvable"));
  }
  else
  {
    client.print(F("HTTP/1.1 200 OK\r\nContent-Type: "));
    client.print(file->contentType);
    client.print(F("\r\nContent-Encoding: gzip\r\nContent-Length: "));
    client.print(file->length);
    client.print(F("\r\nCache-Control: no-cache\r\nX-Content-Type-Options: nosniff\r\nConnection: close\r\n\r\n"));
    for (uint32_t offset = 0; offset < file->length; offset += 512)
    {
      uint32_t chunk = file->length - offset < 512 ? file->length - offset : 512;
      client.write(file->data + offset, chunk);
    }
  }
  client.flush();
  client.stop();
  return true;
}

void startWifi()
{
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println(F("Module Wi-Fi absent : verifier la carte UNO R4 WiFi"));
    while (true)
      delay(1000);
  }

#if CASSARD_WIFI_POINT_ACCES
  WiFi.config(IPAddress(192, 168, 4, 1));
  while (WiFi.beginAP(CASSARD_WIFI_SSID, CASSARD_WIFI_MOT_DE_PASSE, CASSARD_WIFI_CANAL) != WL_AP_LISTENING)
  {
    Serial.println(F("Point d'acces Wi-Fi : nouvel essai"));
    delay(1000);
  }
  Serial.print(F("Point d'acces Wi-Fi \"" CASSARD_WIFI_SSID "\" (WPA2)"));
#else
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("Connexion au Wi-Fi \"" CASSARD_WIFI_SSID "\""));
    WiFi.begin(CASSARD_WIFI_SSID, CASSARD_WIFI_MOT_DE_PASSE);
    for (byte attempt = 0; attempt < 20 && WiFi.status() != WL_CONNECTED; attempt++)
      delay(500);
  }
  // L'adresse IP peut rester à 0.0.0.0 quelques instants après la connexion.
  for (byte attempt = 0; attempt < 20 && WiFi.localIP() == IPAddress(0, 0, 0, 0); attempt++)
    delay(250);
  Serial.print(F("Connecte au Wi-Fi \"" CASSARD_WIFI_SSID "\""));
#endif
  Serial.print(F(" : interface sur http://"));
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  matrix.begin();
  updateMatrix();
  Wire.begin();
  newSessionNonce();
  startWifi();
  httpServer.begin();
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  // Ping toutes les secondes ; sans réponse deux fois de suite, le client est
  // déconnecté (environ 3 s), ce qui déclenche l'arrêt de sécurité.
  webSocket.enableHeartbeat(1000, 1000, 2);
}

void loop()
{
  webSocket.loop();

  // Chaque interrogation du serveur de pages coûte environ 5 ms (dialogue avec le
  // module Wi-Fi) : 10 fois par seconde suffisent, sauf pendant le chargement d'une
  // page (une quinzaine de fichiers), où l'on enchaîne les requêtes pendant 2 s.
  static uint32_t lastHttpPollMs = 0;
  static uint32_t httpBurstUntilMs = 0;
  uint32_t now = millis();
  if ((int32_t)(httpBurstUntilMs - now) > 0 || now - lastHttpPollMs >= 100)
  {
    lastHttpPollMs = now;
    if (serveHttp())
      httpBurstUntilMs = millis() + 2000;
  }

  updateMatrix();

#if !CASSARD_WIFI_POINT_ACCES
  static uint32_t lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 10000)
  {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED)
      startWifi();
  }
#endif
}
