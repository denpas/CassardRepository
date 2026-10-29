# Projet Cassard

Pilotage d'une maquette de la frégate anti-aérienne Cassard (D614) au 1/100.
Le pilote utilise une application web sur iPhone ; le module central, un
Arduino UNO R4 WiFi, sert cette application, vérifie chaque commande signée et
la transmet par I2C aux cartes Nano de propulsion et de tourelle.

## Plan du dépôt

```text
docs/                      Toute la documentation (index : docs/README.md)
web/                       Application web, embarquée dans le firmware du module central
firmware/                  Sketches Arduino
  ModuleCentral/           UNO R4 WiFi : Wi-Fi, pages web, WebSocket signé, maître I2C
  ModulePropulsion/        Nano, I2C 2 : deux moteurs et servo de barre
  ModuleTourelle/          Nano, I2C 1 : servos d'azimut et d'élévation
  carte-servo-i2c/         Carte SERVO-I2C NANO : firmware de la carte, maître R4 de démonstration, test
  libraries/CassardControl/  Bibliothèque partagée (protocole, servos, moteurs, LED)
materiel/carte-servo-i2c/  Fabrication de la carte (Gerber, nomenclatures) et scripts de génération
outils/                    Simulateur, génération de la clé et des pages, tests d'authentification
```

Chaque sketch est dans un dossier du même nom que son fichier `.ino`.

## Modules

| Module | Rôle | Interface |
| --- | --- | --- |
| Central (UNO R4 WiFi) | Application web (port 80), WebSocket signé (port 81), validation et routage des trames, arrêt de sécurité sur perte de liaison | Wi-Fi, I2C maître |
| Propulsion (Nano) | Deux moteurs continus et servo de direction | I2C esclave 2 |
| Tourelle (Nano) | Servos d'azimut et d'élévation | I2C esclave 1 |

## Démarrage rapide

Essayer l'application sans matériel, face à des cartes simulées :

```sh
python3 outils/simulateur/simulateur.py
```

puis ouvrir http://localhost:8080/simulation.html (mot de passe de démo
`cassard demo 2026`). Voir [outils/simulateur](./outils/simulateur/README.md).

## Compiler et téléverser

Environnement vérifié (macOS Apple Silicon) :

```sh
# arduino-cli officiel (https://downloads.arduino.cc/arduino-cli/), placé dans le PATH
softwareupdate --install-rosetta --agree-to-license   # Apple Silicon : avr-gcc est x86_64
arduino-cli core update-index
arduino-cli core install arduino:avr            # modules Nano (1.8.8)
arduino-cli core install arduino:renesas_uno    # module central UNO R4 WiFi (1.6.0)
arduino-cli lib update-index && arduino-cli lib install "WebSockets@2.7.2"
```

`CassardControl` est une bibliothèque locale : chaque sketch inclut d'abord
`<CassardControl.h>`, sans quoi Arduino ne la détecte pas. Ses parties servo et
tampon circulaire ne sont compilées que sur AVR (modules Nano) ; le central
n'utilise que `Protocol.h`.

Le module central a besoin de trois fichiers générés, non versionnés :

```sh
python3 outils/generer-cle-pilotage.py     # firmware/ModuleCentral/CassardSecret.h (clé de pilotage)
cp firmware/ModuleCentral/CassardWifi.exemple.h firmware/ModuleCentral/CassardWifi.h   # puis le compléter
python3 outils/generer-pages-web.py        # firmware/ModuleCentral/PagesWeb.h (web/ compressé)
```

Relancer `generer-pages-web.py` après toute modification de `web/` : l'application
est embarquée dans le firmware du central. Depuis la racine du dépôt :

```sh
arduino-cli compile --fqbn arduino:avr:nano --libraries firmware/libraries firmware/ModulePropulsion
arduino-cli compile --fqbn arduino:avr:nano --libraries firmware/libraries firmware/ModuleTourelle
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --libraries firmware/libraries firmware/ModuleCentral
```

Ajouter `--upload --port <port>` pour téléverser (`arduino-cli board list`
donne le port). Un seul téléversement du central suffit : pages web comprises.

Occupation mémoire mesurée (arduino:avr 1.8.8, arduino:renesas_uno 1.6.0,
WebSockets 2.7.2) :

| Sketch | Carte | Flash | RAM globale (reste) |
| --- | --- | --- | --- |
| ModuleCentral | UNO R4 WiFi | 131 128 o (50 %), dont 44 Ko de pages | 11 080 o (21 688 o) |
| ModulePropulsion | Nano | 12 870 o (41 %) | 1 292 o (756 o) |
| ModuleTourelle | Nano | 12 352 o (40 %) | 1 297 o (751 o) |

Vérifier les implémentations de l'authentification (C++, JavaScript, Python) :
`outils/test-authentification/lancer.sh`.

## Limitations connues

- L'application ne reçoit pas de retour d'état ou d'accusé I2C des modules.
- Les sorties fumée et son de la tourelle sont réservées mais non implémentées.
- Les limites logicielles de servo (0–180° en azimut, 0–80° en élévation)
  doivent être confirmées sur le mécanisme physique avant utilisation.
- Le firmware de la carte SERVO-I2C NANO utilise son propre protocole d'exemple
  (adresse 0x40), distinct de celui des modules de pilotage.
