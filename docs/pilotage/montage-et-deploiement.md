# Montage et deploiement

## Broches configurees

Ces affectations sont celles des firmwares actuels; verifier le brochage et les
drivers de puissance reels avant de brancher les actionneurs.

### Propulsion

| Signal | Broche |
| --- | ---: |
| LED confirmation / erreur | 16 / 15 |
| Moteur gauche direction / PWM | 9 / 3 |
| Moteur droit direction / PWM | 12 / 5 |
| Servo de direction | 14 |

### Tourelle

| Signal | Broche |
| --- | ---: |
| LED confirmation / erreur | 6 / 5 |
| Servo azimut | 14 |
| Servo elevation | 13 |
| Fumee / son | reservees, non implementees |

Les modules doivent partager la masse et le bus I2C doit etre raccorde sur les
lignes SDA/SCL de chaque carte. Utiliser des resistances de tirage adaptees au
bus, respecter les niveaux logiques des cartes et ne pas alimenter les moteurs
depuis les broches Arduino.

## Module central : Arduino UNO R4 WiFi

| Signal | Broche R4 | Vers |
| --- | --- | --- |
| SDA | SDA (en-tete, ou A4) | SDA des modules propulsion et tourelle |
| SCL | SCL (en-tete, ou A5) | SCL des modules |
| GND | GND | masse commune |

Le R4 WiFi travaille en logique 5 V, comme les Nano : liaison I2C directe. Ne pas
utiliser le connecteur Qwiic (bus `Wire1`, 3,3 V). A0 a A3 restent libres : leur
bruit sert a l'alea du nonce de session.

## Compilation et televersement

1. Installer l'environnement decrit dans le [README](../../README.md#compiler-et-téléverser)
   (coeurs `arduino:avr` et `arduino:renesas_uno`, bibliotheque WebSockets).
2. Compiler et televerser `ModulePropulsion` et `ModuleTourelle` sur leurs Nano.
3. Generer la cle de pilotage (`python3 outils/generer-cle-pilotage.py`,
   voir [securite.md](./securite.md)) : `firmware/ModuleCentral/CassardSecret.h`.
4. Creer `firmware/ModuleCentral/CassardWifi.h` a partir de `CassardWifi.exemple.h` :
   point d'acces (`CASSARD_WIFI_POINT_ACCES 1`, adresse 192.168.4.1, pour le
   bassin) ou client d'une box en 2,4 GHz (`0`, adresse affichee sur le
   moniteur serie a 115 200 bauds). Mot de passe WPA2 d'au moins 8 caracteres.
5. Generer les pages embarquees : `python3 outils/generer-pages-web.py`.
6. Compiler et televerser `ModuleCentral` (FQBN `arduino:renesas_uno:unor4wifi`).
   Les pages font partie du firmware : il n'y a plus de fichiers a copier.
7. Rejoindre le Wi-Fi du navire, ouvrir `http://192.168.4.1/` (ou l'adresse
   donnee par la box), se connecter avec le mot de passe de pilotage, activer
   POWER puis verifier d'abord les axes sans charge mecanique et avec des
   limites physiques securisees : sens de rotation du servo d'azimut (90 = axe
   du navire, au-dela de 90 = tribord) et course d'elevation jusqu'a 80°.
8. Verifier l'arret de securite : fermer la page ou couper le Wi-Fi de l'iPhone
   pendant que le moteur tourne; il doit s'arreter en quelques secondes.

Exemples Arduino CLI depuis la racine du depot :

```sh
arduino-cli compile --fqbn arduino:avr:nano --libraries firmware/libraries firmware/ModulePropulsion
arduino-cli compile --fqbn arduino:avr:nano --libraries firmware/libraries firmware/ModuleTourelle
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --libraries firmware/libraries firmware/ModuleCentral
```

## Depannage rapide

- Une commande de tourelle doit etre `1,4,azimut,elevation,255` (azimut 0-180,
  l'interface envoie 30-150; elevation 0-80) et aboutir a l'adresse I2C 1.
- Toutes les trames passent signees (`<trame>|<sequence>|<mac>`); dans la
  console de diagnostic, saisir la trame seule, la page la signe. Si le temoin
  affiche « Mot de passe refuse », la cle du navigateur ne correspond pas a
  `CassardSecret.h` : verifier le mot de passe ou regenerer la cle et
  televerser a nouveau le central.
- Une commande de propulsion commence par `2` et aboutit a l'adresse I2C 2.
- Verifier SDA/SCL, masses communes, adresses esclaves et alimentation avant de
  diagnostiquer un probleme logiciel.
- Les firmwares ne renvoient pas encore de statut de commande a l'interface.
- Le moniteur serie du central (115 200 bauds) affiche le mode Wi-Fi, l'adresse
  de l'interface et les arrets de securite sur perte de liaison.
- Page introuvable apres modification de l'interface : regenerer `PagesWeb.h`
  puis recompiler et televerser le central.
