# Sécurité du pilotage

## Menace visée

Le module central reçoit les commandes sur `ws://<hôte>/_Cassard`. Sans
protection, toute personne connectée au même réseau Wi-Fi peut ouvrir ce
WebSocket et piloter le navire, ou rejouer des trames captées. Une simple page
de connexion ne suffit pas : PHPoC ne gère pas de sessions, et le serveur
WebSocket de l'Arduino ne distingue pas les clients, si bien qu'un attaquant
peut contourner les pages HTML et parler directement au WebSocket. La
vérification se fait donc **sur l'Arduino, trame par trame**.

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
5. **Un seul poste à la fois** : chaque `AUTH` invalide la session précédente.
   L'Arduino envoie alors l'arrêt moteur et le recentrage du gouvernail avant
   d'ouvrir la nouvelle session ; l'ancien poste affiche « Session reprise par
   un autre poste ».

Le code est dans `ModuleCentral/CassardAuth.h` (Arduino) et
`ModuleCentral/web/_cassard-link.js` (navigateur).

## Mise en service

1. **Choisir un mot de passe de pilotage** d'au moins 12 caractères ; une
   phrase de passe de 4 ou 5 mots est conseillée (voir « Limites »).
2. **Générer la clé** depuis la racine du dépôt :

   ```sh
   python3 Pilotage/tools/generer-cle-pilotage.py
   ```

   Le script écrit `Pilotage/ModuleCentral/CassardSecret.h`, exclu de Git. Sans
   ce fichier, le module central ne compile pas.
3. **Recompiler et téléverser `ModuleCentral`**, puis copier le contenu de
   `ModuleCentral/web` sur PHPoC (dont `_cassard-link.js`).
4. **Protéger le Wi-Fi** : point d'accès en WPA2 avec une clé forte, distincte
   du mot de passe de pilotage. C'est la première barrière.
5. **Protéger PHPoC** : dans PHPoC Debugger, définir les mots de passe des
   comptes `admin` et `setup` (8 à 32 caractères). Sans cela, la page
   « Configuration réseau » (`setup_info.php`) et l'accès au module restent
   ouverts. PHPoC ne permet pas de récupérer un mot de passe oublié : il faut
   alors réinitialiser le module, ce qui efface réglages et fichiers.
6. **Essai** : se connecter avec un mauvais mot de passe (l'interface doit
   afficher « Mot de passe refusé » et le navire ne doit pas réagir), puis avec
   le bon, hélice hors de l'eau.

Pour changer le mot de passe : relancer le script, recompiler, téléverser.

## Vérifier les implémentations

```sh
Pilotage/tools/test-authentification/lancer.sh
```

Le script compile `CassardAuth.h` pour le Mac et vérifie que l'Arduino, le
navigateur et le générateur Python calculent les mêmes clés et signatures
(vecteurs FIPS 180-2 et RFC 4231 compris), et que le rejeu, la falsification
et les trames non signées sont refusés.

## Limites connues

- **Pas de chiffrement** : PHPoC sert la page en HTTP et le WebSocket en `ws://`.
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
- **Non vérifié sur le matériel** : le code n'a pas été compilé pour AVR (chaîne
  Arduino absente lors de l'écriture). Vérifier à la compilation l'occupation
  mémoire du module central (SHA-256 ajoute environ 2 Ko de flash). Vérifier
  aussi que `client.print()` renvoie bien les réponses au navigateur (le
  manuel PHPoC ne le documente pas explicitement).
