# Protocole de commande

Les messages WebSocket sont des nombres decimaux separes par des virgules et
termines par CRLF. Le central valide tous les champs avant de les convertir en
octets et de les envoyer sur l'adresse I2C indiquee par le premier champ.
Chaque octet `255` termine une trame I2C.

## Authentification

Le WebSocket du module central (Arduino UNO R4 WiFi) est `ws://<hôte>:81/`,
sous-protocole `cassard`. Le central n'accepte que des trames signées. Le
navigateur envoie d'abord `AUTH\r\n`; le central répond `NONCE,<32 hex>`.
Chaque commande est ensuite transmise sous la forme :

```text
<trame>|<séquence>|<mac>\r\n
2,12,60,255|2|3f9c0d1e5a7b8c42\r\n
```

La première trame signée est `PING`; le central répond `AUTH,OK` ou
`AUTH,ERR`. Toute trame non signée, rejouée, falsifiée ou venant d'un autre
client que celui de la session reçoit `AUTH,ERR` et n'est pas transmise sur le
bus I2C. Quand un autre poste ouvre une session, l'ancien reçoit
`AUTH,REPRISE`. Le détail du calcul est dans
[securite.md](./securite.md). Les trames décrites ci-dessous correspondent à la
partie `<trame>`.

## Format

```text
module,commande,parametres...,255
```

Les trames supportees sont:

| Module/adresse | Code | Commande | Parametres | Taille I2C |
| --- | ---: | --- | --- | ---: |
| Tourelle / 1 | 4 | Definir azimut et elevation | azimut 0-180 (interface : 30-150, 90 = axe), elevation 0-80 | 5 octets |
| Propulsion / 2 | 12 | Marche avant | puissance 0-100 | 4 octets |
| Propulsion / 2 | 13 | Marche arriere | puissance 0-100 | 4 octets |
| Propulsion / 2 | 14 | Arret moteur | aucun | 3 octets |
| Propulsion / 2 | 15 | Braquer a droite | angle 0-90 | 4 octets |
| Propulsion / 2 | 16 | Braquer a gauche | angle 0-90 | 4 octets |
| Propulsion / 2 | 17 | Recentrer la direction | aucun | 3 octets |
| Propulsion / 2 | 18 | Regler le mode proportionnel | niveau 0-5 | 4 octets |

Exemples de trames (avant signature):

```text
1,4,120,25,255\r\n
2,12,60,255\r\n
2,14,255\r\n
```

Le central choisit l'adresse I2C selon le module, contrairement a l'ancien
comportement qui envoyait systematiquement toutes les commandes a l'adresse 2.
Les commandes de tourelle envoient les deux axes dans une seule trame afin de
conserver la position de l'axe qui n'est pas en cours de reglage.

Le caractere `255` est reserve a la fin de trame et ne peut pas servir de valeur
de parametre. Les commandes inconnues, champs non numeriques, champs manquants,
valeurs hors plage et longueurs incorrectes sont rejetees par le central. Les
esclaves verifient aussi leur adresse logique, la commande et la longueur.

## Melange de propulsion

Le niveau proportionnel (reactivite, 0 a 5, commande 18) ajoute aux deux
moteurs un ecart qui aide le navire a tourner, selon la formule du tableur
`docs/propulsion/SimulationModuleProportionnel.xlsx` :

```text
ecart          = angle x reactivite x 2 / puissance   (division entiere)
moteur gauche  = puissance + ecart                     borne a -100..100 %
moteur droit   = puissance - ecart
```

L'angle est positif a tribord, negatif a babord. L'ecart grandit quand la
vitesse baisse : a faible allure, le moteur interieur peut s'inverser et le
navire tourne presque sur place. Il change de signe avec la puissance, ce qui
gere la marche arriere. A puissance nulle, les deux moteurs sont arretes. Avec
la reactivite 0, les deux moteurs recoivent la meme puissance.

| Puissance | Barre | Reactivite | Gauche | Droit |
| ---: | ---: | ---: | ---: | ---: |
| 100 % | 90° tribord | 3 | 100 % | 95 % |
| 50 % | 90° tribord | 3 | 60 % | 40 % |
| 20 % | 90° tribord | 3 | 47 % | -7 % |
| -20 % | 90° tribord | 3 | -47 % | 7 % |
| 50 % | 90° tribord | 5 | 68 % | 32 % |

La PWM utilise la valeur absolue de chaque puissance (`|p| x 190 / 100`), le
signe choisit le sens du moteur. L'interface envoie la reactivite affichee a
chaque ouverture de liaison, pour que la carte de propulsion l'applique.
