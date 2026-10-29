# Documentation

## Pilotage

| Document | Contenu |
| --- | --- |
| [Architecture](./pilotage/architecture.md) | Composants, flux d'une commande, responsabilités de chaque carte |
| [Protocole](./pilotage/protocole.md) | Trames WebSocket et I2C, authentification, plages de valeurs |
| [Sécurité](./pilotage/securite.md) | Mot de passe, trames signées, Wi-Fi, arrêt sur perte de liaison, mise en service |
| [Interface de pilotage](./pilotage/interface-pilotage.md) | Écrans de l'application web |
| [Montage et déploiement](./pilotage/montage-et-deploiement.md) | Brochage, câblage du R4 WiFi, compilation, téléversement, dépannage |

## Carte SERVO-I2C NANO

| Document | Contenu |
| --- | --- |
| [Dossier technique (PDF)](./carte-servo-i2c/SERVO_I2C_NANO_Documentation.pdf) | Schéma, carte, fabrication, firmware, câblage du maître R4 WiFi |
| [Présentation](./carte-servo-i2c/README.txt) | Brochages, commande du PCB, vérifications effectuées |
| [Historique de conception](./carte-servo-i2c/RECAP_CONVERSATION.md) | Choix, révisions, prestataires et coûts |
| [Images](./carte-servo-i2c/img/) | Schéma (PDF, SVG, PNG), rendus de la carte, plan d'assemblage, câblage |

Les fichiers de fabrication et les scripts qui génèrent carte, Gerber et dossier
sont dans [materiel/carte-servo-i2c](../materiel/carte-servo-i2c/).

## Navire

- [Fiche technique de la frégate D614](./navire/D614%20Fiche%20technique.docx)
- Site de la maquette : http://freg.vsl-vaunage.fr/

## Propulsion

[propulsion/](./propulsion/) : calcul hélice / moteur brushless, simulation du
mode proportionnel (`SimulationModuleProportionnel.xlsx`), catalogues
d'hélices (Raboesch), notes et schémas sur le pas et le glissement d'une hélice.

## Matériel

- [Brochage de l'Arduino Nano (DuinoEDU)](./materiel/Brochage_Arduino_Nano_DuinoEDU.png)

## Historique

[historique/](./historique/) : tableurs de la première conception du protocole
(commandes SETUP et EXECUTION, identifiants, broches). Ils décrivent un
protocole plus riche que celui implémenté ; la référence actuelle est
[pilotage/protocole.md](./pilotage/protocole.md).
