#!/usr/bin/env python3
"""Génère ModuleCentral/CassardSecret.h à partir du mot de passe de pilotage.

La clé est dérivée exactement comme dans le navigateur (_cassard-link.js) :
PBKDF2-HMAC-SHA256, sel "CASSARD-D614-pilotage", 10 000 itérations, 32 octets.
Le fichier généré n'est pas versionné (.gitignore).
"""
import getpass
import hashlib
import pathlib
import sys

SALT = b"CASSARD-D614-pilotage"
ITERATIONS = 10000
MIN_LENGTH = 12
OUTPUT = pathlib.Path(__file__).resolve().parent.parent / "ModuleCentral" / "CassardSecret.h"


def main():
    password = getpass.getpass("Mot de passe de pilotage : ")
    if len(password) < MIN_LENGTH:
        sys.exit(f"Refusé : au moins {MIN_LENGTH} caractères (une phrase de passe est conseillée).")
    if getpass.getpass("Confirmer : ") != password:
        sys.exit("Les deux saisies diffèrent.")

    key = hashlib.pbkdf2_hmac("sha256", password.encode("utf-8"), SALT, ITERATIONS, 32)
    values = ", ".join(f"0x{byte:02x}" for byte in key)
    OUTPUT.write_text(
        "// Généré par tools/generer-cle-pilotage.py — NE PAS VERSIONNER.\n"
        "#ifndef CASSARD_SECRET_H\n"
        "#define CASSARD_SECRET_H\n\n"
        "#include \"CassardAuth.h\"\n\n"
        "// En mémoire flash : CassardAuth.h la lit avec pgm_read_byte.\n"
        f"static const uint8_t CassardPilotKey[32] PROGMEM = {{ {values} }};\n\n"
        "#endif\n",
        encoding="utf-8",
    )
    OUTPUT.chmod(0o600)
    print(f"Clé écrite dans {OUTPUT}. Recompiler et téléverser ModuleCentral.")


if __name__ == "__main__":
    main()
