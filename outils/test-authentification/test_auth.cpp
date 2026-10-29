#include <cstdio>
#include <cstring>
#include "CassardAuth.h"
int fails = 0;
void check(bool ok, const char* name) { printf("%s %s\n", ok ? "OK  " : "FAIL", name); if (!ok) fails++; }
int main(int, char** argv) {
  // SHA-256("abc") et message de 2 blocs (FIPS 180-2)
  CassardSha256 s; uint8_t d[32]; char hex[65];
  cassardSha256Begin(s); cassardSha256Update(s, (const uint8_t*)"abc", 3); cassardSha256End(s, d); cassardToHex(d, 32, hex);
  check(!strcmp(hex, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"), "sha256 abc");
  const char* m2 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  cassardSha256Begin(s); cassardSha256Update(s, (const uint8_t*)m2, strlen(m2)); cassardSha256End(s, d); cassardToHex(d, 32, hex);
  check(!strcmp(hex, "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"), "sha256 two blocks");
  // Trame signée par Python (argv) avec clé 00..1f, nonce fixe
  uint8_t key[32]; for (int i = 0; i < 32; i++) key[i] = i;
  const char* nonce = "00112233445566778899aabbccddeeff";
  char payload[24]; uint32_t last = 0;
  check(cassardVerifyFrame(key, nonce, last, argv[1], payload, sizeof payload) && !strcmp(payload, "2,12,50,255") && last == 7, "trame python acceptée");
  check(!cassardVerifyFrame(key, nonce, last, argv[1], payload, sizeof payload), "rejeu refusé");
  last = 0; char bad[80]; strcpy(bad, argv[1]); bad[3] = '9';
  check(!cassardVerifyFrame(key, nonce, last, bad, payload, sizeof payload), "charge modifiée refusée");
  check(!cassardVerifyFrame(key, "ffffffffffffffffffffffffffffffff", last, argv[1], payload, sizeof payload), "autre nonce refusé");
  check(!cassardVerifyFrame(key, nonce, last, "2,12,50,255", payload, sizeof payload), "trame non signée refusée");
  check(!cassardVerifyFrame(key, nonce, last, "2,12,50,255|99999999999|0123456789abcdef", payload, sizeof payload), "séquence trop longue refusée");
  check(!cassardVerifyFrame(key, nonce, last, "2,12,50,255|4294967296|0123456789abcdef", payload, sizeof payload), "dépassement 32 bits refusé");
  check(!cassardVerifyFrame(key, nonce, last, "|1|0123456789abcdef", payload, sizeof payload), "charge vide refusée");
  check(cassardVerifyFrame(key, nonce, last, argv[2], payload, sizeof payload) && !strcmp(payload, "PING"), "PING signé par JS accepté");
  return fails;
}
