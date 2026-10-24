#include <Wire.h>
#include "SPI.h"
#include "Phpoc.h"
#include <CassardControl/Protocol.h>

PhpocServer server(80);
const byte MaxCommandLength = 5;

void setup()
{
  Wire.begin();
  Phpoc.begin(PF_LOG_SPI | PF_LOG_NET);
  server.beginWebSocket("_Cassard");
}

bool parseByteField(const String& frame, unsigned int& position, byte& value)
{
  const unsigned int length = frame.length();
  if (position >= length || frame[position] < '0' || frame[position] > '9')
    return false;

  unsigned int parsed = 0;
  while (position < length && frame[position] >= '0' && frame[position] <= '9')
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
    return fieldCount == 5 && command[2] <= 180 && command[3] <= 60 ? 5 : 0;

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

bool parseCommand(String frame, byte* command, byte& fieldCount)
{
  frame.trim();
  if (frame.length() > 19)
    return false;

  fieldCount = 0;
  unsigned int position = 0;

  while (position < frame.length())
  {
    if (fieldCount >= MaxCommandLength ||
        !parseByteField(frame, position, command[fieldCount]))
      return false;
    fieldCount++;

    if (position == frame.length())
      break;
    if (frame[position] != ',')
      return false;
    position++;
    if (position == frame.length())
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

void loop()
{
  PhpocClient client = server.available();
  if (!client)
    return;

  byte command[MaxCommandLength];
  byte fieldCount;
  String frame = client.readLine();
  if (parseCommand(frame, command, fieldCount))
    forwardCommand(command, fieldCount);
}
