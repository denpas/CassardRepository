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
   donnee par la box), se connecter avec le mot de passe de pilotage (la
   liaison s'ouvre seule) puis verifier d'abord les axes sans charge mecanique et avec des
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

## Matrice de LED du module central

La matrice 8 x 12 du R4 WiFi n'est redessinee que lorsque son contenu change,
au plus toutes les 20 ms (un rendu coute environ 70 us) : l'affichage ne freine
jamais le pilotage.

| Lignes | Zone | Affichage |
| --- | --- | --- |
| 0-1 | Application | un carre 2x2 par client connecte a l'application (WebSocket), 4 au plus |
| 3-4 | Trames | un bloc 2x2 qui avance d'une colonne a chaque trame valide recue (sa vitesse suit le debit), visible 1 s apres la derniere |
| 2-5, colonne 11 | Erreur | trait vertical allume 2 s apres une trame refusee (signature, rejeu, trame non signee ou trop longue, commande invalide) |
| 6-7 | Wi-Fi | un carre 2x2 par appareil du Wi-Fi (application ouverte, ou page chargee depuis moins de 30 s), 4 au plus |
| 0, colonne 11 | Vie | pixel qui clignote a 1 Hz : un pixel fige signale un firmware bloque |

Le module Wi-Fi du R4 ne sait pas lister les appareils associes a son point
d'acces : la zone Wi-Fi compte les appareils (adresses IP) qui ont
l'application ouverte (liaison WebSocket active) ou qui ont charge une page
depuis moins de 30 s. Un appareil qui quitte le Wi-Fi disparait des que la carte
detecte la perte de sa liaison (environ 3 s, ou 30 s s'il n'avait charge
qu'une page). Un telephone associe au Wi-Fi sans ouvrir l'application n'y
apparait pas.

Mesure sur la carte : un tour de `loop()` dure environ 5 ms (190 tours/s),
dont l'essentiel en dialogue avec le module Wi-Fi ; le serveur de pages n'est
interroge que 10 fois par seconde, sauf pendant le chargement d'une page.

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
- Reseau « cassard » absent alors que la carte se dit en ecoute : changer de
  canal (`CASSARD_WIFI_CANAL` dans `CassardWifi.h`, 6 par defaut). Sur le
  canal 1, voisin d'autres reseaux, le point d'acces n'etait plus emis.
  Apres un televersement, le reseau reapparait en une quinzaine de secondes.
- Page introuvable apres modification de l'interface : regenerer `PagesWeb.h`
  puis recompiler et televerser le central.
