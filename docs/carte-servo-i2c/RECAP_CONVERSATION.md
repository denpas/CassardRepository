# SERVO-I2C NANO – Récapitulatif de la conversation (27 et 29/10/2026)

## 1. Demande initiale
Montage Arduino le plus petit possible pour piloter 2 servos à partir d'ordres reçus en I2C (protocole déjà défini par Pascal), avec une carte professionnelle prête à fabriquer : connecteurs servos, connecteur I2C, alimentation des servos indépendante de l'Arduino via un connecteur standard, documentation et dossier téléchargeable.

## 2. Choix du microcontrôleur
- Première piste : Seeed XIAO SAMD21 (21 x 17,5 mm), abandonnée à la demande de Pascal.
- Choix retenu : **Arduino Nano** (ATmega328P, 5 V), enfiché sur deux barrettes femelles 1x15 (remplaçable, reprogrammable en USB). Logique 5 V : pas d'adaptateur de niveau sur un bus I2C 5 V.

## 3. Conception de la carte (rév. 1.0 puis 1.1)
- 2 couches, 1,6 mm, plans de masse sur les deux faces, 2 trous de fixation M2,5.
- **J1** jack DC 5,5 x 2,1 mm (CUI PJ-002A), centre +, 4,8 à 8,4 V : alimente uniquement les servos.
- **Q1** AO3401A : protection contre l'inversion de polarité ; **C1** 470 µF réservoir ; **D2** LED verte « PWR ».
- **J2 / J3** servos (barrettes 1x3, ordre S / + / -), signaux sur A0 et A1 via 220 Ω.
- **J4** I2C JST XH 4 points : GND / +5V / SDA (A4) / SCL (A5). Le +5V du bus alimente le Nano.
- **JP2** : pull-ups I2C 4k7 (ouvert par défaut). **JP1** + D1 : option d'alimentation du Nano par J1 (≤ 5,5 V).
- Contrôle DRC scripté (isolements, bords, connectivité) : OK. Gerber relus avec un lecteur indépendant.

## 4. Modifications demandées (rév. 1.1)
- Inquiétude de Pascal : connecteurs trop proches du Nano (surélevé de 8,5 mm sur ses barrettes). **Bloc connecteurs décalé de 3,5 mm** : carte 37,5 x 49 mm, environ 4,2 mm de dégagement jusqu'aux fiches servo et 5 mm jusqu'au JST.
- Ajout d'une **LED d'activité jaune « ACT »** (D3 + R7 sur A3) : flash de 60 ms à chaque commande transmise à un servo (réglable via LED_ACT_MS).

## 5. Firmware
- Sketch Arduino : esclave I2C (adresse 0x40 par défaut), bibliothèques Wire et Servo.
- Le protocole de Pascal se code dans `protocole.cpp` (fonction `protocoleTraiterTrame`) ; un protocole d'exemple est fourni.
- Contrôle syntaxique effectué ; compilation finale à faire dans l'IDE Arduino (serveurs Arduino inaccessibles depuis l'environnement de travail).

## 6. Fabrication : prestataires et coûts
- Réforme douanière UE : depuis le 1/7/2026, taxe forfaitaire de 3 € par colis < 150 € + TVA ; frais de 2 € en France et 2 € de frais UE à partir du 1/11/2026. Choisir une livraison **DDP** chez JLCPCB pour tout payer à la commande.
- **JLCPCB** (Chine, DDP) : estimation ≈ 45 à 55 € pour 3 cartes assemblées livrées, + Nano achetés en France (≈ 5 € l'unité en compatible). Devis : https://cart.jlcpcb.com/quote/?from=FABS
- **AISLER** (Europe, zéro douane) : assemblage CMS + traversants dès 1 carte ; frais d'assemblage non publiés, ordre de grandeur estimé 100 à 200 € pour 3 cartes. Devis : https://aisler.net/products/assembly
- Recommandation : faire assembler la carte complète (sans le Nano), acheter les Nano en France, les enficher et téléverser le firmware.

## 7. Fichiers à déposer pour un devis
- `fabrication/SERVO_I2C_NANO_gerber.zip`
- `fabrication/assembly/BOM_JLCPCB.csv` et `CPL_JLCPCB.csv`
- Vérifier les codes LCSC marqués « à vérifier ».

## 8. Passage au maître Arduino UNO R4 WiFi (29/10/2026)
- Demande : remplacer l'Arduino UNO + shield PHPOC (maître I2C, commandé par WebSocket depuis une page web) par un **Arduino UNO R4 WiFi**. La carte servo (Nano, rév. 1.1) est inchangée.
- Le R4 WiFi est en logique 5 V : liaison I2C directe vers J4 (GND, 5V, SDA, SCL de l'en-tête, pas le Qwiic qui est en 3,3 V). Fermer JP2, JP1 ouvert.
- Nouveau firmware `firmware/maitre_uno_r4_wifi` : WiFiS3, page de commande sur le port 80, WebSocket sur le port 81 (bibliothèque WebSockets de Links2004 ≥ 2.5.1, sous-protocole « text.phpoc » accepté), commandes S1=90, U1=1500, R1, ?, trames I2C dans `carte_servo.cpp` à adapter au protocole de Pascal, matrice de LED du R4 en témoin d'activité.
- Migration d'une page PHPOC : remplacer ws://ip/remote_push par ws://ip:81/.
- Documentation passée en rév. 1.2 (section 10 + schéma de câblage).
