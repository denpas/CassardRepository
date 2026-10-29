/*
 * Test côté MAÎTRE (autre Arduino 5 V) pour le protocole d'EXEMPLE.
 * Câblage : GND-GND, 5V-5V, A4-SDA, A5-SCL vers J4 de la carte.
 * Fermer JP2 sur la carte si aucun pull-up n'existe sur le bus.
 */
#include <Wire.h>
const uint8_t ADR = 0x40;

void setup() {
  Serial.begin(115200);
  Wire.begin();
}

void angle(uint8_t servo, uint8_t deg) {
  Wire.beginTransmission(ADR);
  Wire.write(0x01); Wire.write(servo); Wire.write(deg);
  uint8_t err = Wire.endTransmission();
  if (err) { Serial.print(F("Erreur I2C ")); Serial.println(err); }
}

void loop() {
  for (int a = 0; a <= 180; a += 30) { angle(0, a); angle(1, 180 - a); delay(400); }
  Wire.beginTransmission(ADR); Wire.write(0x10); Wire.endTransmission();
  Wire.requestFrom(ADR, (uint8_t)6);
  Serial.print(F("Etat :"));
  while (Wire.available()) { Serial.print(' '); Serial.print(Wire.read(), HEX); }
  Serial.println();
}
