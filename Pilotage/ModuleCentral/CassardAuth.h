#ifndef CASSARD_AUTH_H
#define CASSARD_AUTH_H

// Authentification des trames WebSocket : SHA-256, HMAC-SHA256 et
// vérification "<charge>|<sequence>|<mac>" liée au nonce de session.
// Le mac est formé des 16 premiers caractères hexadécimaux (minuscules) de
// HMAC-SHA256(cle, nonceHex + "|" + sequence + "|" + charge).

#include <stdint.h>
#include <string.h>

#ifdef ARDUINO
#include <avr/pgmspace.h>
#define CASSARD_K_READ(index) pgm_read_dword(&CassardSha256K[index])
#else
#define PROGMEM
#define CASSARD_K_READ(index) CassardSha256K[index]
#endif

const uint8_t CassardKeyLength = 32;
const uint8_t CassardNonceLength = 16;
const uint8_t CassardMacHexLength = 16;

static const uint32_t CassardSha256K[64] PROGMEM = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

struct CassardSha256
{
  uint32_t state[8];
  uint8_t block[64];
  uint8_t blockLength;
  uint32_t totalLength;
};

static uint32_t cassardRotr(uint32_t value, uint8_t bits)
{
  return (value >> bits) | (value << (32 - bits));
}

static void cassardSha256Compress(CassardSha256& sha)
{
  uint32_t w[16];
  for (uint8_t i = 0; i < 16; i++)
    w[i] = ((uint32_t)sha.block[i * 4] << 24) | ((uint32_t)sha.block[i * 4 + 1] << 16) |
           ((uint32_t)sha.block[i * 4 + 2] << 8) | sha.block[i * 4 + 3];

  uint32_t a = sha.state[0], b = sha.state[1], c = sha.state[2], d = sha.state[3];
  uint32_t e = sha.state[4], f = sha.state[5], g = sha.state[6], h = sha.state[7];

  for (uint8_t i = 0; i < 64; i++)
  {
    // Le tableau w sert de fenêtre glissante de 16 mots pour économiser la RAM.
    if (i >= 16)
    {
      uint32_t w15 = w[(i + 1) & 15];
      uint32_t w2 = w[(i + 14) & 15];
      uint32_t s0 = cassardRotr(w15, 7) ^ cassardRotr(w15, 18) ^ (w15 >> 3);
      uint32_t s1 = cassardRotr(w2, 17) ^ cassardRotr(w2, 19) ^ (w2 >> 10);
      w[i & 15] += s0 + w[(i + 9) & 15] + s1;
    }
    uint32_t t1 = h + (cassardRotr(e, 6) ^ cassardRotr(e, 11) ^ cassardRotr(e, 25)) +
                  ((e & f) ^ (~e & g)) + CASSARD_K_READ(i) + w[i & 15];
    uint32_t t2 = (cassardRotr(a, 2) ^ cassardRotr(a, 13) ^ cassardRotr(a, 22)) +
                  ((a & b) ^ (a & c) ^ (b & c));
    h = g; g = f; f = e; e = d + t1;
    d = c; c = b; b = a; a = t1 + t2;
  }

  sha.state[0] += a; sha.state[1] += b; sha.state[2] += c; sha.state[3] += d;
  sha.state[4] += e; sha.state[5] += f; sha.state[6] += g; sha.state[7] += h;
}

static void cassardSha256Begin(CassardSha256& sha)
{
  static const uint32_t initial[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
  };
  memcpy(sha.state, initial, sizeof(initial));
  sha.blockLength = 0;
  sha.totalLength = 0;
}

static void cassardSha256Update(CassardSha256& sha, const uint8_t* data, uint16_t length)
{
  for (uint16_t i = 0; i < length; i++)
  {
    sha.block[sha.blockLength++] = data[i];
    if (sha.blockLength == 64)
    {
      cassardSha256Compress(sha);
      sha.blockLength = 0;
    }
  }
  sha.totalLength += length;
}

static void cassardSha256End(CassardSha256& sha, uint8_t* digest)
{
  uint32_t bitLength = sha.totalLength * 8;
  uint8_t padding = 0x80;
  cassardSha256Update(sha, &padding, 1);
  padding = 0;
  while (sha.blockLength != 56)
    cassardSha256Update(sha, &padding, 1);
  uint8_t lengthBytes[8] = { 0, 0, 0, 0,
                             (uint8_t)(bitLength >> 24), (uint8_t)(bitLength >> 16),
                             (uint8_t)(bitLength >> 8), (uint8_t)bitLength };
  cassardSha256Update(sha, lengthBytes, 8);
  for (uint8_t i = 0; i < 8; i++)
  {
    digest[i * 4] = sha.state[i] >> 24;
    digest[i * 4 + 1] = sha.state[i] >> 16;
    digest[i * 4 + 2] = sha.state[i] >> 8;
    digest[i * 4 + 3] = sha.state[i];
  }
}

static void cassardHmacPad(CassardSha256& sha, const uint8_t* key, uint8_t pad)
{
  uint8_t padded[64];
  for (uint8_t i = 0; i < 64; i++)
    padded[i] = (i < CassardKeyLength ? key[i] : 0) ^ pad;
  cassardSha256Begin(sha);
  cassardSha256Update(sha, padded, 64);
}

// HMAC-SHA256 d'un message en trois morceaux (nonce, séquence, charge).
static void cassardHmac(const uint8_t* key, const char* const* parts, uint8_t partCount, uint8_t* mac)
{
  CassardSha256 sha;
  uint8_t inner[32];
  cassardHmacPad(sha, key, 0x36);
  for (uint8_t i = 0; i < partCount; i++)
    cassardSha256Update(sha, (const uint8_t*)parts[i], strlen(parts[i]));
  cassardSha256End(sha, inner);
  cassardHmacPad(sha, key, 0x5c);
  cassardSha256Update(sha, inner, 32);
  cassardSha256End(sha, mac);
}

static void cassardToHex(const uint8_t* data, uint8_t length, char* hex)
{
  static const char digits[] = "0123456789abcdef";
  for (uint8_t i = 0; i < length; i++)
  {
    hex[i * 2] = digits[data[i] >> 4];
    hex[i * 2 + 1] = digits[data[i] & 15];
  }
  hex[length * 2] = 0;
}

// Vérifie "<charge>|<sequence>|<mac>". En cas de succès, renvoie true,
// recopie la charge dans payload et met à jour lastSequence.
static bool cassardVerifyFrame(const uint8_t* key, const char* nonceHex, uint32_t& lastSequence,
                               const char* frame, char* payload, uint8_t payloadSize)
{
  const char* firstBar = strchr(frame, '|');
  if (!firstBar)
    return false;
  const char* secondBar = strchr(firstBar + 1, '|');
  if (!secondBar || strchr(secondBar + 1, '|'))
    return false;

  uint8_t payloadLength = firstBar - frame;
  uint8_t sequenceLength = secondBar - firstBar - 1;
  if (payloadLength == 0 || payloadLength >= payloadSize ||
      sequenceLength == 0 || sequenceLength > 10 ||
      strlen(secondBar + 1) != CassardMacHexLength)
    return false;

  uint32_t sequence = 0;
  for (const char* digit = firstBar + 1; digit < secondBar; digit++)
  {
    if (*digit < '0' || *digit > '9')
      return false;
    uint32_t next = sequence * 10 + (*digit - '0');
    if (next / 10 != sequence)
      return false;
    sequence = next;
  }
  if (sequence <= lastSequence)
    return false;

  char sequenceText[11];
  memcpy(sequenceText, firstBar + 1, sequenceLength);
  sequenceText[sequenceLength] = 0;
  memcpy(payload, frame, payloadLength);
  payload[payloadLength] = 0;

  const char* parts[5] = { nonceHex, "|", sequenceText, "|", payload };
  uint8_t mac[32];
  char expected[CassardMacHexLength + 1];
  cassardHmac(key, parts, 5, mac);
  cassardToHex(mac, CassardMacHexLength / 2, expected);

  // Comparaison en temps constant.
  uint8_t difference = 0;
  for (uint8_t i = 0; i < CassardMacHexLength; i++)
    difference |= expected[i] ^ secondBar[1 + i];
  if (difference != 0)
    return false;

  lastSequence = sequence;
  return true;
}

#endif
