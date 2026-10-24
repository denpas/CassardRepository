# Architecture du pilotage

## Vue des composants

```mermaid
flowchart LR
    operator["Operateur"]
    browser["Navigateur<br/>Interface de pilotage"]
    central["Module central<br/>PHPoC + ModuleCentral.ino<br/>WebSocket et maitre I2C"]
    propulsion["Module propulsion<br/>I2C esclave 2"]
    turret["Module tourelle<br/>I2C esclave 1"]
    propulsionActuators["Moteurs gauche/droit<br/>Servo de direction"]
    turretActuators["Servos azimut/elevation"]
    shared["Bibliotheque partagee<br/>CassardControl"]

    operator --> browser
    browser -->|"WebSocket /_Cassard<br/>trames texte"| central
    central -->|"I2C adresse 2<br/>commandes propulsion"| propulsion
    central -->|"I2C adresse 1<br/>position azimut/elevation"| turret
    propulsion --> propulsionActuators
    turret --> turretActuators
    shared -.-> central
    shared -.-> propulsion
    shared -.-> turret
```

Le navigateur ne commande pas directement les esclaves : il envoie des trames
texte au serveur WebSocket PHPoC du module central. Le central valide les champs,
convertit les valeurs en octets, puis choisit l'adresse I2C d'apres le numero du
module. Les deux cartes esclaves n'ont pas de liaison reseau directe.

## Flux d'une commande

```mermaid
sequenceDiagram
    actor Operateur
    participant Web as Interface web
    participant Central as Module central
    participant Bus as Bus I2C
    participant Esclave as Module propulsion ou tourelle
    participant Actionneur as Moteur ou servo

    Operateur->>Web: Modifie un controle
    Web->>Central: Trame texte sur WebSocket
    Central->>Central: Valide module, commande, longueur et valeurs
    Central->>Bus: Envoie les octets avec le module comme adresse
    Bus->>Esclave: Livraison a l'adresse I2C
    Esclave->>Esclave: Assemble et valide la trame terminee par 255
    Esclave->>Actionneur: Applique la consigne
```

## Responsabilites

- **Interface web** (`ModuleCentral/web`) : expose les controles de propulsion et
  de tourelle et emet les commandes via le WebSocket PHPoC.
- **Module central** (`ModuleCentral/ModuleCentral.ino`) : maitre I2C; parse et
  valide les trames entrantes puis les route vers l'adresse `1` ou `2`.
- **Module propulsion** (`ModulePropulsion`) : esclave `2`; commande les moteurs
  continus et le servo de direction.
- **Module tourelle** (`ModuleTourelle`) : esclave `1`; commande les servos
  d'azimut et d'elevation.
- **CassardControl** (`libraries/CassardControl`) : code partage des commandes,
  du protocole et des controles bas niveau.

Le detail des codes, parametres et longueurs de trame est dans
[protocole.md](./protocole.md). Les raccordements et les etapes de mise en
service sont dans [montage-et-deploiement.md](./montage-et-deploiement.md).

## Sens des communications

Le chemin fonctionnel documente est unidirectionnel : de l'interface vers le
central puis vers l'esclave concerne. Le protocole I2C utilise actuellement
n'envoie pas d'accuse de reception applicatif aux pages web; celles-ci ne
connaissent donc pas l'etat reel d'un actionneur.
