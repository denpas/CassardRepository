# Pilotage du Cassard

Le pilotage est constitue de trois firmwares Arduino et d'une interface web sur
le module central. Le central recoit les commandes WebSocket et les transmet par
I2C au module concerne.

## Structure

```text
Pilotage/
  ModuleCentral/                 Sketch central et site PHPoC
    ModuleCentral.ino
    web/                         Fichiers a deployer sur PHPoC
    firmware/                    Images firmware PHPoC conservees
  ModulePropulsion/              Sketch de propulsion (I2C 2)
  ModuleTourelle/                Sketch de tourelle (I2C 1)
  libraries/CassardControl/      Bibliotheque Arduino partagee
  tools/                         Generation de cle et tests d'authentification
  Archive/Legacy/                Anciens prototypes non maintenus
  docs/                          Protocole et deploiement
```

Les sketches actifs sont dans des dossiers du meme nom que leur fichier `.ino`.
`Archive/Legacy` contient les anciens essais PAP, servo et Yun; ils ne font pas
partie des trois firmwares actifs.

## Modules

| Module | Role | Interface |
| --- | --- | --- |
| Central | WebSocket/PHPoC, validation et routage des trames | I2C maitre |
| Propulsion | Deux moteurs continus et servo de direction | I2C esclave 2 |
| Tourelle | Servos d'azimut et d'elevation | I2C esclave 1 |

Le protocole et les exemples de trames sont decrits dans
[docs/protocole.md](./docs/protocole.md). Les broches configurees par les
firmwares et les instructions de compilation/deploiement sont dans
[docs/montage-et-deploiement.md](./docs/montage-et-deploiement.md).
Le schema des composants et des flux est disponible dans
[docs/architecture.md](./docs/architecture.md).
La securisation du pilotage (mot de passe, trames signees, Wi-Fi et PHPoC) et
sa mise en service sont decrites dans [docs/securite.md](./docs/securite.md).
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
arduino-cli core update-index && arduino-cli core install arduino:avr   # 1.8.8
arduino-cli lib update-index && arduino-cli lib install "PHPoC@1.5.1"
```

`CassardControl` est une bibliotheque locale (`libraries/CassardControl`) : chaque
sketch inclut d'abord `<CassardControl.h>`, sans quoi Arduino ne la detecte pas.
Depuis la racine du depot, avec Arduino CLI:

```sh
arduino-cli compile --fqbn arduino:avr:nano --libraries Pilotage/libraries Pilotage/ModulePropulsion
arduino-cli compile --fqbn arduino:avr:nano --libraries Pilotage/libraries Pilotage/ModuleTourelle
```

Compiler le module central avec le profil correspondant exactement a la carte et
au materiel PHPoC utilises, par exemple:

```sh
arduino-cli compile --fqbn arduino:avr:uno --libraries Pilotage/libraries Pilotage/ModuleCentral
```

Le central requiert la bibliotheque `Phpoc` fournie pour le materiel PHPoC et
le fichier `ModuleCentral/CassardSecret.h`, genere (hors Git) par
`python3 Pilotage/tools/generer-cle-pilotage.py`.
Verifier que la cible choisie correspond a la carte reelle: les commandes ci-
dessus indiquent des profils d'exemple, pas une detection automatique du
materiel.

Occupation memoire mesuree (arduino:avr 1.8.8, PHPoC 1.5.1) :

| Sketch | Carte | Flash | RAM globale (reste) |
| --- | --- | --- | --- |
| ModuleCentral | Uno | 27 268 o (84 %) | 1 345 o (703 o) |
| ModuleCentral | Mega 2560 | 28 728 o (11 %) | 1 564 o (6 628 o) |
| ModulePropulsion | Nano | 12 870 o (41 %) | 1 292 o (756 o) |
| ModuleTourelle | Nano | 12 352 o (40 %) | 1 297 o (751 o) |

Sur Uno, la verification d'une trame utilise environ 490 octets de pile au
plus profond (mesure `-fstack-usage`) : eviter d'ajouter des variables globales
ou des `String` au module central sans remesurer. Televerser chaque sketch sur sa carte puis deployer le contenu de
`ModuleCentral/web` sur le serveur web PHPoC.

## Limitations connues

- L'interface actuelle ne recoit pas de retour d'etat ou d'accuse I2C des
  modules.
- Les sorties fumee et son de la tourelle sont reservees mais non implementees.
- Les limites logicielles de servo (0-180 degres en azimut, 0-80 en elevation)
  doivent etre confirmees sur le mecanisme physique avant utilisation.
