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

## Compilation et televersement

1. Installer le coeur Arduino AVR correspondant aux cartes esclaves.
2. Garder `Pilotage/libraries` comme chemin de bibliotheques pour les modules
   actifs; `CassardControl` est une bibliotheque Arduino locale.
3. Installer la bibliotheque PHPoC du fabricant pour le module central.
4. Compiler et televerser `ModulePropulsion` et `ModuleTourelle` sur leurs cartes.
5. Compiler `ModuleCentral` avec le profil de la carte PHPoC; televerser le
   firmware puis copier le contenu de `ModuleCentral/web` sur le serveur Web.
6. Depuis l'interface, connecter le WebSocket puis verifier d'abord les axes
   sans charge mecanique et avec des limites physiques securisees.

Exemples Arduino CLI depuis la racine du depot:

```sh
arduino-cli compile --fqbn arduino:avr:nano --libraries Pilotage/libraries Pilotage/ModulePropulsion
arduino-cli compile --fqbn arduino:avr:nano --libraries Pilotage/libraries Pilotage/ModuleTourelle
arduino-cli compile --fqbn arduino:avr:uno --libraries Pilotage/libraries Pilotage/ModuleCentral
```

Le dernier profil n'est valable que pour une carte Uno compatible avec le
materiel PHPoC installe. Remplacer le FQBN pour correspondre a la carte reelle.
Le central depend de `Phpoc.h`, et les compilations web-PHPoC peuvent demander
les outils specifiques du fabricant.

## Depannage rapide

- Une commande de tourelle doit etre `1,4,azimut,elevation,255` et aboutir a
  l'adresse I2C 1.
- Une commande de propulsion commence par `2` et aboutit a l'adresse I2C 2.
- Verifier SDA/SCL, masses communes, adresses esclaves et alimentation avant de
  diagnostiquer un probleme logiciel.
- Les firmwares ne renvoient pas encore de statut de commande a l'interface.
