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

Installer Arduino CLI ou Arduino IDE, le coeur AVR, la bibliotheque PHPoC
necessaire au module central et les dependances disponibles dans
`libraries/CassardControl`. Depuis la racine du depot, avec Arduino CLI:

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
materiel. Televerser chaque sketch sur sa carte puis deployer le contenu de
`ModuleCentral/web` sur le serveur web PHPoC.

## Limitations connues

- L'interface actuelle ne recoit pas de retour d'etat ou d'accuse I2C des
  modules.
- Les sorties fumee et son de la tourelle sont reservees mais non implementees.
- Les limites logicielles de servo (0-180 degres en azimut, 0-60 en elevation)
  doivent etre confirmees sur le mecanisme physique avant utilisation.
