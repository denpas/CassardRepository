import hmac, hashlib, sys
key = bytes(range(32)); nonce = "00112233445566778899aabbccddeeff"
def sign(seq, payload):
    return f"{payload}|{seq}|" + hmac.new(key, f"{nonce}|{seq}|{payload}".encode(), hashlib.sha256).hexdigest()[:16]
print(sign(7, "2,12,50,255"))
print(hashlib.pbkdf2_hmac("sha256", "phrase de passe élégante".encode(), b"CASSARD-D614-pilotage", 10000, 32).hex())
