# Sécurité du pilotage

## Menace visée

Le module central (Arduino UNO R4 WiFi) reçoit les commandes sur
`ws://<hôte>:81/`. Sans protection, toute personne connectée au même réseau
Wi-Fi peut ouvrir ce WebSocket et piloter le navire, ou rejouer des trames
captées. Une simple page de connexion ne suffit pas : un attaquant peut
contourner les pages HTML et parler directement au WebSocket. La vérification
se fait donc **sur l'Arduino, trame par trame**.

## Mécanisme

1. **Clé de pilotage** : dérivée du mot de passe par PBKDF2-HMAC-SHA256 (sel
   `CASSARD-D614-pilotage`, 10 000 itérations, 32 octets). Le navigateur la
   calcule à la connexion et la garde dans `sessionStorage` (effacée à la
   fermeture de l'onglet ou par « Verrouiller »). L'Arduino ne contient que la
   clé dérivée, jamais le mot de passe. **Le mot de passe ne circule pas sur le
   réseau.**
2. **Poignée de main** : le navigateur envoie `AUTH`; l'Arduino tire un nonce
   de 128 bits (bruit des entrées A0–A3 et gigue d'horloge, haché en SHA-256) et
   répond `NONCE,<32 hex>`.
3. **Trames signées** : `<charge>|<séquence>|<mac>`, où `mac` est formé des 16
   premiers caractères hexadécimaux de
   `HMAC-SHA256(clé, nonce + "|" + séquence + "|" + charge)`. La séquence doit
   croître strictement : une trame rejouée, modifiée, signée pour un autre nonce
   ou non signée est refusée (`AUTH,ERR`) et n'atteint pas le bus I2C.
4. **Vérification du mot de passe** : la première trame signée est `PING`;
   l'Arduino répond `AUTH,OK` (liaison ouverte) ou `AUTH,ERR` (« Mot de passe
   refusé » dans l'interface).
5. **Un seul poste à la fois** : la session est liée au client WebSocket qui
   l'a ouverte ; les trames d'un autre client sont refusées. Chaque `AUTH`
   invalide la session précédente : l'Arduino envoie l'arrêt moteur et le
   recentrage du gouvernail avant d'ouvrir la nouvelle, et l'ancien poste reçoit
   `AUTH,REPRISE` (« Session reprise par un autre poste »).
6. **Arrêt sur perte de liaison** : l'Arduino envoie un ping WebSocket chaque
   seconde. Si le poste de pilotage ferme la liaison ou ne répond plus (navire
   hors de portée, iPhone en veille), la session est close en 3 s environ, avec
   arrêt moteur et barre au centre.

Le code est dans `firmware/ModuleCentral/CassardAuth.h` (Arduino) et
`web/_cassard-link.js` (navigateur).

## Mise en service

1. **Choisir un mot de passe de pilotage** d'au moins 12 caractères ; une
   phrase de passe de 4 ou 5 mots est conseillée (voir « Limites »).
2. **Générer la clé** depuis la racine du dépôt :

   ```sh
   python3 outils/generer-cle-pilotage.py
   ```

   Le script écrit `firmware/ModuleCentral/CassardSecret.h`, exclu de Git. Sans
   ce fichier, le module central ne compile pas.
3. **Protéger le Wi-Fi** : dans `firmware/ModuleCentral/CassardWifi.h` (copie non
   versionnée de `CassardWifi.exemple.h`), le point d'accès du navire est en
   WPA2 ; choisir une clé forte, distincte du mot de passe de pilotage. C'est la
   première barrière. La compilation échoue si la clé fait moins de 8
   caractères.
4. **Générer les pages, recompiler et téléverser `ModuleCentral`** : les pages
   sont embarquées dans le firmware, il n'y a rien d'autre à copier ni de compte
   d'administration à protéger sur la carte.
5. **Essai** : se connecter avec un mauvais mot de passe (l'interface doit
   afficher « Mot de passe refusé » et le navire ne doit pas réagir), puis avec
   le bon, hélice hors de l'eau ; enfin couper le Wi-Fi de l'iPhone moteur en
   marche : il doit s'arrêter en quelques secondes.

Pour changer le mot de passe : relancer le script, recompiler, téléverser.

## Vérifier les implémentations

```sh
outils/test-authentification/lancer.sh
```

Le script compile `CassardAuth.h` pour le Mac et vérifie que l'Arduino, le
navigateur et le générateur Python calculent les mêmes clés et signatures
(vecteurs FIPS 180-2 et RFC 4231 compris), et que le rejeu, la falsification
et les trames non signées sont refusés.

## Limites connues

- **Pas de chiffrement** : la carte sert la page en HTTP et le WebSocket en `ws://`.
  Les commandes restent lisibles sur le réseau ; elles ne sont pas falsifiables.
  La confidentialité repose sur le WPA2 du Wi-Fi.
- **Attaque hors ligne** : quelqu'un qui capte une trame signée peut tester des
  mots de passe hors ligne. PBKDF2 ralentit chaque essai, mais seul un mot de
  passe long protège réellement.
- **Déni de service** : un intrus présent sur le Wi-Fi peut envoyer `AUTH` et
  interrompre la session en cours. Le navire s'arrête alors (comportement sûr),
  mais il ne peut pas être piloté par l'intrus.
- **Aléa du nonce** : l'Arduino n'a pas de générateur matériel ; le nonce mélange
  bruit analogique et gigue d'horloge. Cela suffit à empêcher le rejeu entre
  sessions, pas à résister à une analyse matérielle.
- **Compilé et simulé, pas encore essayé sur la carte** : le module central
  compile pour l'UNO R4 WiFi (49 % de flash, 21 Ko de RAM libre, voir le
  README). La poignée de main, la prise de main par un autre poste et l'arrêt
  sur déconnexion ont été vérifiés avec l'interface réelle face à un simulateur
  du central. Restent à valider sur la carte : le Wi-Fi, le délai effectif de
  détection d'une perte de liaison et le bus I2C.
