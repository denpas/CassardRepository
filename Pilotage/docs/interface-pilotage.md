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

Page d'accueil avec le champ « Mot de passe de pilotage ». Le mot de passe
n'est pas envoyé : le navigateur en dérive la clé de signature des commandes et
la garde pour l'onglet en cours. Un mot de passe erroné est détecté à la
connexion au navire (« Mot de passe refusé »). Voir [securite.md](./securite.md).

### 2. Tableau de bord — `web/_CassardMenu.php`

Présente sous forme de cartes les accès à Propulsion & direction, Tourelle
100 mm, Instruments, Sons & lumières et à la Console de diagnostic. Le lien
« Configuration réseau » ouvre la page `setup_info.php` fournie par PHPoC.
« Verrouiller » efface la clé de pilotage de l'onglet. Les pages de commande
renvoient vers la connexion si aucune clé n'est présente.
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

Même coque et mêmes trims que le pupitre propulsion, au format iPhone 16e
vertical (390 × 844), sans défilement : les deux vues se partagent la hauteur.

- **Vue de dessus** (en haut), redessinée en SVG d'après le plan
  `tourelle_vue_dessus.png` : l'avant du navire est en haut. On saisit la
  tourelle au doigt ou à la souris et on la fait pivoter autour de son axe,
  de 60° à bâbord à 60° à tribord. Un cadran gradué (tous les 5°, repères à 0,
  30 et 60°) et un index orange indiquent l'azimut.
- **Trim d'azimut** sous la vue de dessus : boutons BÂB / TRI (pas de 5°),
  levier à glisser, synchronisé avec la vue.
- **Vue de profil** (en bas), d'après `tourelle_vue_profil.png` : on saisit le
  canon et on l'incline autour de ses tourillons, de l'horizontale (0°) à 80°.
  Cadran gradué de 0 à 80° avec index orange.
- **Trim d'élévation** vertical à droite de la vue de profil : boutons HAUT /
  BAS (pas de 5°).
- **Interrupteur POWER** : ouvre la liaison authentifiée ; la position courante
  est envoyée dès que le mot de passe est accepté.

À chaque changement, la page envoie les deux coordonnées dans une trame
unique de commande 4 au module 1 (regroupées toutes les 80 ms). L'azimut
servo vaut `90 + rotation` (30 à 150, 90 = axe du navire, au-delà de 90 =
tribord) ; l'élévation servo vaut l'angle du canon (0 à 80). **Le sens de
rotation réel du servo d'azimut et la course mécanique jusqu'à 80° doivent être
vérifiés sur la maquette** avant usage.

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
`text.phpoc`, via `_cassard-link.js` qui authentifie la liaison et signe chaque
trame. Le témoin ne passe à « Connecté » qu'après acceptation du mot de passe
par le navire; il ne confirme pas
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
