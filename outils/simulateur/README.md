# Simulateur du pilotage

Fait tourner la vraie application web (`web/`) face à une carte
maître UNO R4 WiFi et à deux cartes Nano simulées, sans aucun matériel. Un
tableau de bord montre en direct les réactions de chaque carte.

```sh
python3 outils/simulateur/simulateur.py
```

Puis ouvrir http://localhost:8080/simulation.html (mot de passe de démo
`cassard demo 2026`). Python 3.9 ou plus récent, sans bibliothèque à installer.

## Ce qui est simulé

| Élément | Comportement reproduit | Source |
| --- | --- | --- |
| Carte maître UNO R4 WiFi | Poignée de main `AUTH` / `NONCE`, vérification HMAC et séquence, session liée au poste, `AUTH,REPRISE`, arrêt de sécurité à chaque nouvelle session et sur perte de liaison (~3 s), validation des trames, matrice de LED en trois zones (liaison, trafic, erreur) calculée comme `renderMatrix()`, messages du moniteur série | `firmware/ModuleCentral/ModuleCentral.ino` |
| Nano propulsion (I2C 2) | Mélange proportionnel (division entière comme en C), PWM `\|puissance\| × 190 / 100`, moteurs en miroir (DIR), servo de barre `90 ± angle`, LED OK / erreur | `firmware/ModulePropulsion/PropulsionCONTControler.h` |
| Nano tourelle (I2C 1) | `GoToPos` : servos d'azimut (0–180) et d'élévation (0–80) | `firmware/ModuleTourelle/TourelleControler.h` |

Le tableau de bord affiche l'application dans un iPhone 16e, la matrice et le
moniteur série du R4, les échanges WebSocket (signatures HMAC en violet), les
trames du bus I2C et l'état des deux cartes Nano. Le bouton **Couper le Wi-Fi
du téléphone** simule une sortie de portée : la carte ne reçoit plus rien et
déclenche l'arrêt de sécurité au bout de ~3 s, comme son ping WebSocket.

## Options

| Option | Défaut | Rôle |
| --- | --- | --- |
| `--port` | 8080 | port HTTP (80 sur la carte) |
| `--port-websocket` | 8181 | port WebSocket (81 sur la carte, réservé à l'administrateur sur macOS) |
| `--mot-de-passe` | `cassard demo 2026` | mot de passe de pilotage simulé |
| `--hote` | `127.0.0.1` | `0.0.0.0` pour tester depuis l'iPhone sur le même Wi-Fi (le simulateur devient alors joignable par tout le réseau local) |

Les pages sont servies telles qu'elles sont dans le dépôt ; seul le port
WebSocket de `_cassard-link.js` est remplacé à la volée. Une modification de
l'interface se voit donc en rechargeant la page, sans régénérer `PagesWeb.h`.

## Limites

Le simulateur reproduit la logique des firmwares, pas le matériel : ni le Wi-Fi
réel, ni les délais du bus I2C, ni la mécanique (vitesse des servos, inertie des
moteurs) ne sont simulés. Il ne remplace pas un essai sur les cartes. Il faut
le mettre à jour quand le comportement des firmwares change.
