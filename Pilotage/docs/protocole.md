# Protocole de commande

Les messages WebSocket sont des nombres decimaux separes par des virgules et
termines par CRLF. Le central valide tous les champs avant de les convertir en
octets et de les envoyer sur l'adresse I2C indiquee par le premier champ.
Chaque octet `255` termine une trame I2C.

## Format

```text
module,commande,parametres...,255
```

Les trames supportees sont:

| Module/adresse | Code | Commande | Parametres | Taille I2C |
| --- | ---: | --- | --- | ---: |
| Tourelle / 1 | 4 | Definir azimut et elevation | azimut 0-180, elevation 0-60 | 5 octets |
| Propulsion / 2 | 12 | Marche avant | puissance 0-100 | 4 octets |
| Propulsion / 2 | 13 | Marche arriere | puissance 0-100 | 4 octets |
| Propulsion / 2 | 14 | Arret moteur | aucun | 3 octets |
| Propulsion / 2 | 15 | Braquer a droite | angle 0-90 | 4 octets |
| Propulsion / 2 | 16 | Braquer a gauche | angle 0-90 | 4 octets |
| Propulsion / 2 | 17 | Recentrer la direction | aucun | 3 octets |
| Propulsion / 2 | 18 | Regler le mode proportionnel | niveau 0-5 | 4 octets |

Exemples de messages WebSocket:

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

Le niveau proportionnel ajoute une correction de direction au niveau moteur.
Cette correction depend de l'angle et du niveau choisi et les deux puissances
resultantes sont bornees a -100..100 %. En marche arriere, le signe choisit le
sens du moteur et la PWM utilise la valeur absolue de la puissance.
