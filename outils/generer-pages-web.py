#!/usr/bin/env python3
"""Génère firmware/ModuleCentral/PagesWeb.h à partir de web/.

Chaque fichier est compressé en gzip et embarqué dans la flash de l'Arduino
UNO R4 WiFi, qui le sert tel quel (Content-Encoding: gzip). Relancer ce script
après toute modification de l'application, puis recompiler ModuleCentral.
Le fichier généré n'est pas versionné (.gitignore).
"""
import gzip
import pathlib

ICI = pathlib.Path(__file__).resolve().parent
WEB = ICI.parent / "web"
SORTIE = ICI.parent / "firmware" / "ModuleCentral" / "PagesWeb.h"
TYPES = {
    ".html": "text/html; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".js": "application/javascript; charset=utf-8",
    ".ttf": "font/ttf",
    ".png": "image/png",
    ".svg": "image/svg+xml",
}


def main():
    fichiers = sorted(p for p in WEB.iterdir() if p.is_file() and not p.name.startswith("."))
    lignes = [
        "// Généré par outils/generer-pages-web.py depuis web/ — NE PAS MODIFIER.",
        "#ifndef CASSARD_PAGES_WEB_H",
        "#define CASSARD_PAGES_WEB_H",
        "",
        "#include <stdint.h>",
        "",
        "struct CassardWebFile",
        "{",
        "  const char* path;",
        "  const char* contentType;",
        "  const uint8_t* data;",
        "  uint32_t length;",
        "};",
        "",
    ]
    table = []
    total = 0
    for numero, fichier in enumerate(fichiers):
        if fichier.suffix not in TYPES:
            raise SystemExit(f"Type de fichier non pris en charge : {fichier.name}")
        donnees = gzip.compress(fichier.read_bytes(), compresslevel=9, mtime=0)
        total += len(donnees)
        octets = ", ".join(f"0x{octet:02x}" for octet in donnees)
        lignes.append(f"// {fichier.name} : {fichier.stat().st_size} octets, {len(donnees)} compressés")
        lignes.append(f"static const uint8_t CassardWebData{numero}[] = {{ {octets} }};")
        table.append(f'  {{ "/{fichier.name}", "{TYPES[fichier.suffix]}", CassardWebData{numero}, sizeof(CassardWebData{numero}) }},')
    lignes += ["", "static const CassardWebFile CassardWebFiles[] = {"] + table + [
        "};",
        "static const uint8_t CassardWebFileCount = sizeof(CassardWebFiles) / sizeof(CassardWebFiles[0]);",
        "",
        "#endif",
        "",
    ]
    SORTIE.write_text("\n".join(lignes), encoding="utf-8")
    print(f"{len(fichiers)} fichiers, {total} octets compressés -> {SORTIE}")


if __name__ == "__main__":
    main()
