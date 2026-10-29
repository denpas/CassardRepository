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

// Matrice de LED (8 lignes x 12 colonnes), trois zones fixes :
//  colonnes 0-7  : liaison - croix clignotante à 1 Hz (aucun pilote), pixel qui
//                  tourne sur le contour (poste connecté, mot de passe pas encore
//                  validé), coche fixe (pilote connecté) ;
//  colonnes 9-10 : trafic - bargraphe logarithmique des trames valides par
//                  seconde ; le pixel du haut s'éteint 30 ms à chaque trame ;
//  colonne 11    : erreur - « ! » allumé 2 s après une trame refusée, clignotant
//                  rapide si les erreurs persistent (au moins 2 dans la dernière
//                  seconde et plus de 10 % des trames).
const char* const IconCross[8] = { "X......X", ".X....X.", "..X..X..", "...XX...", "...XX...", "..X..X..", ".X....X.", "X......X" };
const char* const IconCheck[8] = { "........", ".......X", "......X.", ".....X..", "X...X...", ".X.X....", "..X.....", "........" };
const uint16_t TrafficLevels[8] = { 1, 2, 5, 10, 20, 50, 100, 200 };   // trames/s pour 1 à 8 LED
uint8_t matrixPixels[8][12];
bool sessionReady = false;      // mot de passe validé (PING signé accepté)
bool errorSeen = false;
uint32_t lastFrameMs = 0;
uint32_t lastErrorMs = 0;
uint32_t lastRateMs = 0;
uint16_t frameCount = 0;        // trames valides et refusées sur la seconde en cours
uint16_t errorCount = 0;
uint16_t framesPerSecond = 0;
uint16_t errorsPerSecond = 0;
uint8_t errorPercent = 0;

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
  frameCount++;
  lastFrameMs = millis();
}

void noteError()
{
  errorCount++;
  lastErrorMs = millis();
  errorSeen = true;
}

void drawIcon(const char* const icon[8])
{
  for (byte row = 0; row < 8; row++)
    for (byte col = 0; col < 8; col++)
      matrixPixels[row][col] = icon[row][col] == 'X';
}

void renderMatrix(uint32_t now)
{
  memset(matrixPixels, 0, sizeof(matrixPixels));

  if (sessionReady)
  {
    drawIcon(IconCheck);
    byte height = 0;
    while (height < 8 && framesPerSecond >= TrafficLevels[height])
      height++;
    for (byte row = 0; row < height; row++)
      matrixPixels[7 - row][9] = matrixPixels[7 - row][10] = 1;
    if (height > 0 && now - lastFrameMs < 30)
      matrixPixels[8 - height][9] = matrixPixels[8 - height][10] = 0;
  }
  else if (webSocket.connectedClients() > 0)
  {
    // Connexion en cours : un pixel parcourt les 28 LED du contour, un pas toutes les 40 ms.
    byte step = (now / 40) % 28;
    if (step < 7)
      matrixPixels[0][step] = 1;
    else if (step < 14)
      matrixPixels[step - 7][7] = 1;
    else if (step < 21)
      matrixPixels[7][7 - (step - 14)] = 1;
    else
      matrixPixels[7 - (step - 21)][0] = 1;
  }
  else if ((now / 500) & 1)
  {
    drawIcon(IconCross);   // clignote : prouve que la carte tourne
  }

  bool persistent = errorsPerSecond >= 2 && errorPercent > 10;
  if (errorSeen && now - lastErrorMs < 2000 && (!persistent || ((now / 125) & 1)))
    for (byte row = 0; row < 8; row++)
      if (row != 5)
        matrixPixels[row][11] = 1;

  matrix.renderBitmap(matrixPixels, 8, 12);
}

void updateMatrix()
{
  uint32_t now = millis();
  if (now - lastRateMs >= 1000)
  {
    uint16_t total = frameCount + errorCount;
    framesPerSecond = frameCount;
    errorsPerSecond = errorCount;
    errorPercent = total ? (uint32_t)errorCount * 100 / total : 0;
    frameCount = errorCount = 0;
    lastRateMs = now;
  }
  static uint32_t lastRenderMs = 0;
  if (now - lastRenderMs >= 10)
  {
    lastRenderMs = now;
    renderMatrix(now);
  }
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
  renderMatrix(millis());
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
