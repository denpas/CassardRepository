SERVO-I2C NANO - Dossier rév. 1.2 - 29/10/2026 (carte rév. 1.1)
=========================================

Carte esclave I2C pilotant 2 servomoteurs, à base d'Arduino Nano.
Carte 37,5 x 49 mm, 2 couches, alimentation des servos séparée (jack DC 5,5x2,1).

CONTENU
  docs/SERVO_I2C_NANO_Documentation.pdf   Dossier technique complet (à lire en premier)
  docs/img/schematic.pdf|svg|png          Schéma électrique
  docs/img/pcb_top.png, pcb_bottom.png    Rendus de la carte
  docs/img/assembly_top.png               Plan d'assemblage

  fabrication/SERVO_I2C_NANO_gerber.zip   >>> FICHIER À ENVOYER AU FABRICANT <<<
  fabrication/gerber/                     Gerber RS-274X X2 + perçages Excellon (détail)
  fabrication/assembly/BOM_JLCPCB.csv     Nomenclature CMS (format JLCPCB)
  fabrication/assembly/CPL_JLCPCB.csv     Placement CMS (format JLCPCB)
  fabrication/assembly/BOM_complete.csv   Nomenclature complète (séparateur ;)

  firmware/servo_i2c_nano/                Sketch Arduino (esclave I2C, 2 servos)
                                          -> votre protocole se code dans protocole.cpp
  firmware/test_maitre/                   Sketch de test côté maître (Arduino 5 V quelconque)
  firmware/maitre_uno_r4_wifi/            Maître Arduino UNO R4 WiFi (remplace UNO + PHPOC) :
                                          page web http://<ip>/ + WebSocket ws://<ip>:81/ -> I2C
                                          -> renseigner arduino_secrets.h ; protocole dans carte_servo.cpp
  docs/img/cablage_uno_r4_wifi.png|pdf    Câblage du R4 WiFi vers J4

  sources/                                Scripts Python ayant généré carte, Gerber, doc
                                          (python3 board.py = DRC ; gerbers.py ; render.py ;
                                           schematic.py ; bom.py ; docs.py)

COMMANDE PCB (rappel)
  2 couches - FR-4 1,6 mm - cuivre 1 oz - HASL sans plomb - 37,5 x 49 mm
  3 trous oblongs métallisés (jack) inclus dans le fichier PTH (G85)

BROCHAGES
  J1 jack   : centre = + servos (4,8-8,4 V), manchon = GND
  J2/J3     : S + -   (servo 1 = A0, servo 2 = A1)
  J4 JST XH : 1 GND, 2 +5V (alimente le Nano), 3 SDA (A4), 4 SCL (A5)
  JP2       : fermer pour activer les pull-ups 4k7
  D3 (ACT)  : LED jaune sur A3, flashe à chaque commande transmise à un servo
  JP1       : fermer pour alimenter le Nano par J1 (J1 <= 5,5 V, sans +5V sur J4)

MAÎTRE UNO R4 WIFI
  R4 GND -> J4-1   R4 5V -> J4-2   R4 SDA -> J4-3   R4 SCL -> J4-4   (pas le Qwiic, 3,3 V)
  Fermer JP2 (pull-ups), laisser JP1 ouvert.
  Bibliothèques : paquet « Arduino UNO R4 Boards » + « WebSockets » (Links2004) >= 2.5.1
  Commandes WebSocket : S1=90  S2=45  U1=1500  R1  R2  ?

VÉRIFICATIONS EFFECTUÉES
  - DRC scripté : isolements >= 0,20 mm, cuivre/bord >= 0,40 mm, connectivité de tous les nets
  - Gerber et perçages relus avec un lecteur indépendant (gerbonara)
  - Firmwares carte et maître : contrôle syntaxique C++ ; compilation finale à faire dans l'IDE Arduino
  - Codes LCSC marqués « à vérifier » : à confirmer au moment de la commande
