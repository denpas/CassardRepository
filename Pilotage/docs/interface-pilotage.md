# Interface de pilotage

Cette documentation décrit les écrans web du module central PHPoC. Les pages
sont servies depuis `ModuleCentral/web` et utilisent la feuille de style
commune `app.css`.

## Identité visuelle et navigation

L'interface utilise une barre bleu marine, des cartes claires, une accentuation
bleu acier et des boutons ambre. Le module Propulsion & direction adopte un
pupitre graphite inspiré d'une radiocommande physique, avec des commandes
contrastées ambre et bleu. La mise en page s'adapte aux écrans étroits.
L'en-tête des écrans de pilotage affiche le nom du module, un indicateur de
connexion WebSocket et les boutons de connexion et de retour au tableau de bord.

## Écrans

### 1. Connexion — `web/index.html`

Page d'accueil avec les champs Identifiant et Mot de passe et le bouton
« Accéder au tableau de bord ». Le formulaire transmet les données à
`_CassardMenu.php`. **Ce formulaire n'authentifie pas l'utilisateur** : le code
actuel ne vérifie ni l'identifiant ni le mot de passe; il enregistre seulement
l'identifiant fourni dans la session.

### 2. Tableau de bord — `web/_CassardMenu.php`

Présente sous forme de cartes les accès à Propulsion & direction, Tourelle
100 mm, Instruments, Sons & lumières et à la Console de diagnostic. Le lien
« Configuration réseau » ouvre la page `setup_info.php` fournie par PHPoC.
Lorsque PHPoC signale le mode `SETUP`, le tableau de bord affiche un avertissement
à la place des modules.

### 3. Propulsion & direction — `web/_CassardPropulsion.php`

L'écran reproduit la face d'une radiocommande de modélisme (type Spektrum DX6i)
et est mis en page pour l'iPhone 16e (390 × 844 points, zones de sécurité
comprises); sur un écran plus large, la radiocommande reste centrée.

- **Manche vu de dessus** : dessiné sur canvas (platine vissée, bague aluminium
  brossé à quatre repères, graduations tous les 5°, soufflet caoutchouc, pommeau
  moleté). Il se pilote au toucher, à la souris ou avec les flèches du clavier.
  L'axe avant / arrière règle la puissance de -100 à +100 %, l'axe bâbord /
  tribord commande la direction de -90 à +90°. Au relâchement, il revient au
  neutre et envoie l'arrêt moteur (commande 14) et le recentrage du gouvernail
  (commande 17).
- **Trim moteur vertical, à droite du manche** : levier strié dans une rainure
  graduée, boutons AV / AR (pas de 5 %); synchronisé avec le manche. Une valeur
  positive envoie la commande 12, une valeur négative la commande 13 et zéro
  l'arrêt (commande 14).
- **Trim direction horizontal, sous le manche** : boutons BÂB / TRI (pas de 5°);
  les valeurs négatives envoient le braquage à gauche (16), les valeurs positives
  celui à droite (15) et le centre le recentrage (17).
- **Interrupteur POWER** : ouvre ou ferme la liaison WebSocket; le voyant et les
  barres orange s'allument une fois connecté.
- **Écran LCD** : affiche puissance, direction et réactivité.
- **Molette** (à droite de l'écran) : réactivité moteur de 0 à 5, commande 18 du
  module 2.

Les commandes sont transmises au serveur WebSocket PHPoC `/_Cassard`. Les
modifications combinées sont regroupées avant l'envoi pour limiter la cadence
des trames. Le relâchement du joystick envoie immédiatement les deux commandes
de sécurité. Une perte de connexion ne produit pas de commandes différées.

### 4. Tourelle 100 mm — `web/_CassardTourelle.php`

Deux curseurs permettent de régler l'élévation de 0° à 60° et l'azimut de 0° à
180°. À chaque changement, la page envoie les deux coordonnées dans une trame
unique de commande 4 au module 1, afin de conserver la consigne de l'autre axe.

### 5. Instruments — `web/_CassardInstruments.php`

La page affiche cinq indicateurs en tr/min et en pourcentage : moteur gauche,
arbre gauche, puissance, arbre droit et moteur droit. Leurs plages et zones
visuelles sont configurées sur les jauges de la page. Le protocole ne fournit
actuellement pas de retour d'état des modules vers le navigateur : ces
indicateurs ne constituent donc pas une télémétrie réelle tant qu'une source de
mesures n'est pas raccordée.

### 6. Sons & lumières — `web/_CassardSonsEtLumieres.php`

Présente l'écran du module et son état de connexion. Il indique que les
commandes ne sont pas disponibles dans la version actuelle; aucune commande de
son ou d'éclairage n'est implémentée dans cet écran.

### 7. Console de diagnostic — `web/_CassardDebug.php`

Permet de saisir une trame brute, de l'envoyer sur la connexion WebSocket active
et d'effacer l'historique des trames envoyées. L'écran signale si la connexion
n'est pas établie avant l'envoi.

## Connexion et limites fonctionnelles

La connexion est établie séparément depuis chaque écran de commande. Le point
WebSocket utilisé est `ws://<hôte>/_Cassard`, avec le sous-protocole
`text.phpoc`. Un témoin signale l'état de la connexion; il ne confirme pas
l'exécution d'une commande par un actionneur. Les messages et plages de
commandes sont détaillés dans [protocole.md](./protocole.md).

## Dossier de commande

La présentation générale 16:9 est disponible dans
[Dossier commande.pptx](./Dossier%20commande.pptx). Elle
décrit l'architecture, les écrans, le pupitre joystick, les modules matériels,
le protocole et les étapes de mise en service.

Pour régénérer la présentation après modification de ces informations :

```sh
python3 Pilotage/docs/generate-dossier-commande.py
```

## Simulation vidéo

[Voir la simulation de l'interface (MP4)](./interface-pilotage-demo.mp4).
Cette vidéo est une maquette visuelle de parcours : elle montre les sept écrans,
dont le nouveau pupitre avec joystick 3D, curseur de puissance sous le joystick
et curseur de direction à sa droite. Les valeurs sont illustratives; la vidéo
n'ouvre aucun WebSocket et ne commande pas le bateau. Elle ne remplace pas un
essai sur le serveur PHPoC.

Pour régénérer la vidéo sur macOS avec Swift et AVFoundation :

```sh
swift docs/generate-interface-demo.swift
```

Le fichier est créé par défaut dans `Pilotage/docs/interface-pilotage-demo.mp4`.
