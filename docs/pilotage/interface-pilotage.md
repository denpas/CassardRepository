# Interface de pilotage

Cette documentation décrit les écrans web du module central (Arduino UNO R4
WiFi). Les pages, en HTML statique, sont écrites dans `web/` (racine du dépôt),
compressées dans le firmware par `outils/generer-pages-web.py` et servies par la
carte sur le port 80 ; elles utilisent la feuille de style
commune `app.css`, ainsi que deux scripts partagés :

- `_cassard-link.js` : liaison WebSocket authentifiée et signature des trames
  (voir [securite.md](./securite.md)); fournit aussi le renvoi vers la page de
  connexion quand aucune clé de pilotage n'est présente.
- `_cassard-trim.js` : composant « trim » de radiocommande (levier strié,
  rainure graduée, boutons de pas) utilisé par les écrans Propulsion et Tourelle.

## Identité visuelle et navigation

Les écrans de commande **Propulsion & direction** et **Tourelle 100 mm**
reproduisent la face d'une radiocommande de modélisme : coque gris métal vissée,
poignées caoutchouc, sérigraphies, accents orange, trims et interrupteur
**POWER**. Ils sont conçus pour l'iPhone 16e en portrait (390 × 844 points,
zones de sécurité comprises) et restent centrés sur un écran plus large. Leur
en-tête sombre affiche le lien de retour au tableau de bord (logo CASSARD), le
nom du module et le témoin de connexion; la liaison s'ouvre et se ferme avec
l'interrupteur POWER.

Les autres écrans (connexion, tableau de bord, instruments, sons & lumières,
diagnostic) gardent la présentation en cartes claires sur barre bleu marine,
avec un bouton « Connecter » dans l'en-tête des pages qui utilisent la liaison.

## Écrans

### 1. Connexion — `web/index.html`

Page d'accueil avec le champ « Mot de passe de pilotage ». Le mot de passe
n'est pas envoyé : le navigateur en dérive la clé de signature des commandes et
la garde pour l'onglet en cours. Un mot de passe erroné est détecté à la
connexion au navire (« Mot de passe refusé »). Voir [securite.md](./securite.md).

### 2. Tableau de bord — `web/_CassardMenu.html`

Présente sous forme de cartes les accès à Propulsion & direction, Tourelle
100 mm, Instruments, Sons & lumières et à la Console de diagnostic. Le lien
« Verrouiller » efface la clé de pilotage de l'onglet. Les pages de commande
renvoient vers la connexion si aucune clé n'est présente. Les réglages Wi-Fi se
font dans le firmware (`CassardWifi.h`), plus depuis une page.

### 3. Propulsion & direction — `web/_CassardPropulsion.html`

L'écran reproduit la face d'une radiocommande de modélisme (type Spektrum DX6i)
et est mis en page pour l'iPhone 16e (390 × 844 points, zones de sécurité
comprises); sur un écran plus large, la radiocommande reste centrée.

- **Manche vu de dessus** : dessiné sur canvas (platine vissée, bague aluminium
  brossé à quatre repères, graduations tous les 5°, soufflet caoutchouc, pommeau
  moleté). Il se pilote au toucher, à la souris ou avec les flèches du clavier.
  L'axe avant / arrière règle la puissance de -100 à +100 %, l'axe bâbord /
  tribord commande la direction de -90 à +90°. Le puits est rond mais la course
  est étirée sur un carré : au bord du puits en diagonale, on obtient à la fois
  100 % de puissance et 90° de barre. Au relâchement, il revient au
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

Les commandes sont transmises au WebSocket du module central (port 81). Les
modifications combinées sont regroupées avant l'envoi pour limiter la cadence
des trames. Le relâchement du joystick envoie immédiatement les deux commandes
de sécurité. Une perte de connexion ne produit pas de commandes différées.

### 4. Tourelle 100 mm — `web/_CassardTourelle.html`

Même coque et mêmes trims que le pupitre propulsion, au format iPhone 16e
vertical (390 × 844), sans défilement : les deux vues se partagent la hauteur.

- **Vue de dessus** (en haut), redessinée en SVG d'après le plan de la
  tourelle vue de dessus; le contour de la coque a été rendu symétrique par
  rapport à l'axe du canon (le plan d'origine ne l'était pas), les détails
  intérieurs (trappons, aérations) sont conservés. L'avant du navire est en haut. On saisit la
  tourelle au doigt ou à la souris et on la fait pivoter autour de son axe,
  de 60° à bâbord à 60° à tribord. Un cadran gradué (tous les 5°, repères à 0,
  30 et 60°) et un index orange indiquent l'azimut.
- **Trim d'azimut** sous la vue de dessus : boutons BÂB / TRI (pas de 5°),
  levier à glisser, synchronisé avec la vue.
- **Vue de profil** (en bas), redessinée d'après le plan de profil (côté
  gauche), le canon étant une pièce séparée : on saisit le
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

### 5. Instruments — `web/_CassardInstruments.html`

La page affiche cinq indicateurs en tr/min et en pourcentage : moteur gauche,
arbre gauche, puissance, arbre droit et moteur droit. Leurs plages et zones
visuelles sont configurées sur les jauges de la page. Le protocole ne fournit
actuellement pas de retour d'état des modules vers le navigateur : ces
indicateurs ne constituent donc pas une télémétrie réelle tant qu'une source de
mesures n'est pas raccordée.

### 6. Sons & lumières — `web/_CassardSonsEtLumieres.html`

Présente l'écran du module et son état de connexion. Il indique que les
commandes ne sont pas disponibles dans la version actuelle; aucune commande de
son ou d'éclairage n'est implémentée dans cet écran.

### 7. Console de diagnostic — `web/_CassardDebug.html`

Permet de saisir une trame brute (par exemple `2,14,255`), de l'envoyer sur la
connexion active — la page la signe automatiquement — et d'effacer l'historique
des trames envoyées. L'écran signale si la connexion
n'est pas établie avant l'envoi.

## Connexion et limites fonctionnelles

La connexion est établie séparément depuis chaque écran de commande. Le point
WebSocket utilisé est `ws://<hôte>:81/`, avec le sous-protocole `cassard`, via
`_cassard-link.js` qui authentifie la liaison et signe chaque
trame. Le témoin ne passe à « Connecté » qu'après acceptation du mot de passe
par le navire; il ne confirme pas
l'exécution d'une commande par un actionneur. Les messages et plages de
commandes sont détaillés dans [protocole.md](./protocole.md).

## Simulation

Le [simulateur](../../outils/simulateur/README.md) fait tourner ces écrans face
à une carte maître et deux cartes Nano simulées, avec un tableau de bord de
leurs réactions, sans matériel : `python3 outils/simulateur/simulateur.py`.
