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

// Matrice de LED (8 x 12) : point = aucun pilote, cadre = pilote connecté,
// cadre pointé = commande transmise.
const uint32_t MatrixIdle[3] = { 0x00000000, 0x00600600, 0x00000000 };
const uint32_t MatrixPilot[3] = { 0xFFF80180, 0x18018018, 0x01801FFF };
const uint32_t MatrixCommand[3] = { 0xFFF80180, 0x18618618, 0x01801FFF };
uint32_t matrixFlashStart = 0;
bool matrixFlashing = false;

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

void endSession()
{
  sendSafeStop();
  sessionClient = NoClient;
  matrix.loadFrame(MatrixIdle);
}

void handleFrame(uint8_t client, const uint8_t* data, size_t length)
{
  while (length > 0 && (data[length - 1] == '\r' || data[length - 1] == '\n' || data[length - 1] == ' '))
    length--;
  if (length == 0)
    return;
  if (length > MaxFrameLength)
  {
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
    reply(client, "AUTH,ERR");
    return;
  }

  if (strcmp(payload, "PING") == 0)
  {
    reply(client, "AUTH,OK");
    matrix.loadFrame(MatrixPilot);
    return;
  }

  byte command[MaxCommandLength];
  byte fieldCount;
  if (parseCommand(payload, command, fieldCount))
  {
    forwardCommand(command, fieldCount);
    matrix.loadFrame(MatrixCommand);
    matrixFlashStart = millis();
    matrixFlashing = true;
  }
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

void serveHttp()
{
  WiFiClient client = httpServer.available();
  if (!client)
    return;

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
  while (WiFi.beginAP(CASSARD_WIFI_SSID, CASSARD_WIFI_MOT_DE_PASSE) != WL_AP_LISTENING)
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
  matrix.loadFrame(MatrixIdle);
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
  serveHttp();

  if (matrixFlashing && millis() - matrixFlashStart > 80)
  {
    matrix.loadFrame(sessionClient != NoClient ? MatrixPilot : MatrixIdle);
    matrixFlashing = false;
  }

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
