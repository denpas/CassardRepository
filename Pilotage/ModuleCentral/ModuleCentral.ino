#include <CassardControl.h>
#include <Wire.h>
#include "SPI.h"
#include "Phpoc.h"
#include <CassardControl/Protocol.h>
#include "CassardAuth.h"
// Clé de pilotage générée par tools/generer-cle-pilotage.py (non versionnée).
#include "CassardSecret.h"

PhpocServer server(80);
const byte MaxCommandLength = 5;
const byte MaxFrameLength = 64;
const byte MaxPayloadLength = 24;

char sessionNonce[CassardNonceLength * 2 + 1];
bool sessionActive = false;
uint32_t lastSequence = 0;
uint8_t entropyPool[32];

void setup()
{
  Wire.begin();
  Phpoc.begin(PF_LOG_SPI | PF_LOG_NET);
  server.beginWebSocket("_Cassard");
}

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
  sessionActive = true;
}

void reply(PhpocClient& client, const char* message)
{
  client.print(message);
  client.print("\r\n");
}

void loop()
{
  PhpocClient client = server.available();
  if (!client)
    return;

  // readLine() sans argument tronque à 32 caractères (tampon de la bibliothèque
  // PHPoC) : on lit dans notre tampon. Une ligne trop longue est coupée par la
  // bibliothèque, qui jette la suite ; elle est alors refusée ci-dessous.
  char frame[MaxFrameLength + 3];
  int length = client.readLine((uint8_t*)frame, MaxFrameLength + 2);
  while (length > 0 && (frame[length - 1] == '\r' || frame[length - 1] == '\n' || frame[length - 1] == ' '))
    length--;
  frame[length > 0 ? length : 0] = 0;
  if (length <= 0)
    return;
  if (length > MaxFrameLength)
  {
    reply(client, "AUTH,ERR");
    return;
  }

  // Nouvelle session : l'ancienne devient invalide, on met le navire en sécurité.
  if (strcmp(frame, "AUTH") == 0)
  {
    if (sessionActive)
      sendSafeStop();
    newSessionNonce();
    client.print("NONCE,");
    reply(client, sessionNonce);
    return;
  }

  char payload[MaxPayloadLength];
  if (!sessionActive ||
      !cassardVerifyFrame(CassardPilotKey, sessionNonce, lastSequence, frame, payload, sizeof(payload)))
  {
    reply(client, "AUTH,ERR");
    return;
  }

  if (strcmp(payload, "PING") == 0)
  {
    reply(client, "AUTH,OK");
    return;
  }

  byte command[MaxCommandLength];
  byte fieldCount;
  if (parseCommand(payload, command, fieldCount))
    forwardCommand(command, fieldCount);
}
