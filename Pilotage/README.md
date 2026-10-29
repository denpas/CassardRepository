# Pilotage du Cassard

Le pilotage est constitue de trois firmwares Arduino et d'une interface web
servie par le module central, un Arduino UNO R4 WiFi. Le central recoit les
commandes signees par WebSocket et les transmet par I2C au module concerne.

## Structure

```text
Pilotage/
  ModuleCentral/                 Sketch central (Arduino UNO R4 WiFi)
    ModuleCentral.ino
    CassardAuth.h                Verification des trames signees
    CassardWifi.exemple.h        Modele des reglages Wi-Fi
    web/                         Interface web, embarquee dans le firmware
  ModulePropulsion/              Sketch de propulsion (I2C 2)
  ModuleTourelle/                Sketch de tourelle (I2C 1)
  libraries/CassardControl/      Bibliotheque Arduino partagee
  Simulateur/                    Simulation de l'interface et des cartes, sans materiel
  tools/                         Generation de cle, des pages embarquees, tests
  Archive/Legacy/                Anciens prototypes non maintenus (dont PHPoC)
  docs/                          Protocole et deploiement
```

Les sketches actifs sont dans des dossiers du meme nom que leur fichier `.ino`.
`Archive/Legacy` contient les anciens essais PAP, servo et Yun, ainsi que la
version PHPoC du module central (Arduino UNO + shield PHPoC, remplacee par
l'UNO R4 WiFi) : images firmware et fichiers web devenus inutiles.

## Modules

| Module | Role | Interface |
| --- | --- | --- |
| Central (UNO R4 WiFi) | Interface web (port 80), WebSocket signe (port 81), validation et routage des trames | Wi-Fi, I2C maitre |
| Propulsion | Deux moteurs continus et servo de direction | I2C esclave 2 |
| Tourelle | Servos d'azimut et d'elevation | I2C esclave 1 |

Le protocole et les exemples de trames sont decrits dans
[docs/protocole.md](./docs/protocole.md). Les broches configurees par les
firmwares et les instructions de compilation/deploiement sont dans
[docs/montage-et-deploiement.md](./docs/montage-et-deploiement.md).
Le schema des composants et des flux est disponible dans
[docs/architecture.md](./docs/architecture.md).
La securisation du pilotage (mot de passe, trames signees, Wi-Fi, arret sur perte de liaison) et
sa mise en service sont decrites dans [docs/securite.md](./docs/securite.md).
Le [simulateur](./Simulateur/README.md) fait tourner l'interface reelle face a
une carte maitre et deux cartes Nano simulees, avec un tableau de bord de leurs
reactions : `python3 Pilotage/Simulateur/simulateur.py`.
Les ecrans de pilotage et leurs limites sont decrits dans
[docs/interface-pilotage.md](./docs/interface-pilotage.md); une video de
simulation est disponible dans [docs/interface-pilotage-demo.mp4](./docs/interface-pilotage-demo.mp4).
Le dossier de commande 16:9 au format PowerPoint est disponible dans
[docs/Dossier commande.pptx](./docs/Dossier%20commande.pptx).

## Compiler

Environnement verifie (macOS Apple Silicon) :

```sh
# arduino-cli officiel (https://downloads.arduino.cc/arduino-cli/), place dans le PATH
softwareupdate --install-rosetta --agree-to-license   # Apple Silicon : avr-gcc est x86_64
arduino-cli core update-index
arduino-cli core install arduino:avr            # modules Nano (1.8.8)
arduino-cli core install arduino:renesas_uno    # module central UNO R4 WiFi (1.6.0)
arduino-cli lib update-index && arduino-cli lib install "WebSockets@2.7.2"
```

`CassardControl` est une bibliotheque locale (`libraries/CassardControl`) : chaque
sketch inclut d'abord `<CassardControl.h>`, sans quoi Arduino ne la detecte pas.
Ses parties servo et tampon circulaire ne sont compilees que sur AVR (modules
Nano); le central n'utilise que `Protocol.h`.

Le module central a besoin de trois fichiers generes, non versionnes :

```sh
python3 Pilotage/tools/generer-cle-pilotage.py    # ModuleCentral/CassardSecret.h (cle de pilotage)
cp Pilotage/ModuleCentral/CassardWifi.exemple.h Pilotage/ModuleCentral/CassardWifi.h   # puis le completer
python3 Pilotage/tools/generer-pages-web.py       # ModuleCentral/PagesWeb.h (interface compressee)
```

Relancer `generer-pages-web.py` apres toute modification de `ModuleCentral/web` :
l'interface est embarquee dans le firmware du central. Depuis la racine du depot :

```sh
arduino-cli compile --fqbn arduino:avr:nano --libraries Pilotage/libraries Pilotage/ModulePropulsion
arduino-cli compile --fqbn arduino:avr:nano --libraries Pilotage/libraries Pilotage/ModuleTourelle
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --libraries Pilotage/libraries Pilotage/ModuleCentral
```

Ajouter `--upload --port <port>` pour televerser (`arduino-cli board list`
donne le port). Un seul televersement du central suffit : pages web comprises.

Occupation memoire mesuree (arduino:avr 1.8.8, arduino:renesas_uno 1.6.0,
WebSockets 2.7.2) :

| Sketch | Carte | Flash | RAM globale (reste) |
| --- | --- | --- | --- |
| ModuleCentral | UNO R4 WiFi | 130 848 o (49 %), dont 44 Ko de pages | 11 080 o (21 688 o) |
| ModulePropulsion | Nano | 12 870 o (41 %) | 1 292 o (756 o) |
| ModuleTourelle | Nano | 12 352 o (40 %) | 1 297 o (751 o) |

## Limitations connues

- L'interface actuelle ne recoit pas de retour d'etat ou d'accuse I2C des
  modules.
- Les sorties fumee et son de la tourelle sont reservees mais non implementees.
- Les limites logicielles de servo (0-180 degres en azimut, 0-80 en elevation)
  doivent etre confirmees sur le mecanisme physique avant utilisation.
