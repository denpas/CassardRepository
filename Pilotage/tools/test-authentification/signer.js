const L = require(process.argv[2]);
const key = Array.from({length: 32}, (_, i) => i);
const nonce = "00112233445566778899aabbccddeeff";
if (process.argv[3] === "frame") console.log(L._signFrame(key, nonce, 7, "2,12,50,255"));
else if (process.argv[3] === "ping") console.log(L._signFrame(key, nonce, 1, "PING"));
else {
  const t = Date.now();
  console.log(L._toHex(L._pbkdf2(L._utf8("phrase de passe élégante"), L._utf8("CASSARD-D614-pilotage"), 10000)));
  console.error("pbkdf2 ms:", Date.now() - t);
  // RFC 4231 cas 2
  console.error("rfc4231:", L._toHex(L._hmac(L._utf8("Jefe"), L._utf8("what do ya want for nothing?"))) === "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843" ? "OK" : "FAIL");
}
