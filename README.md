# INF3995 — Simulation de tunnels avec deux Go2

Environnement Gazebo avec deux robots Unitree Go2, un LiDAR 2D, une caméra,
une IMU et l'odométrie pour chaque robot. Le monde comprend des embranchements,
des boucles, des impasses, 34 pentes et une chute de 1,6 m.
Les couloirs font environ 3 m de large ; l'ensemble occupe environ 86 × 38 m.
Aucune navigation autonome n'est incluse.

La simulation utilise **Ubuntu 24.04, ROS 2 Jazzy, Gazebo Harmonic et ros_gz**
dans Docker. Il n'est pas nécessaire d'installer ROS sur l'ordinateur.
Les modèles sont inclus : aucun téléchargement de décor au lancement.

## Prérequis

- Ubuntu avec un affichage X11 ou XWayland pour Gazebo et RViz.
- [Docker Engine et le plugin Compose](https://docs.docker.com/engine/install/ubuntu/),
  accessibles avec `docker` sans `sudo` (voir les étapes post-installation de Docker).
- `xauth` pour autoriser les fenêtres du conteneur : `sudo apt install xauth`.
- Internet pour la première construction de l'image.

## Démarrer

Depuis ce dossier :

```bash
bash sim.sh build
bash sim.sh start
```

La première construction installe les dépendances et compile le contrôleur.
`start` lance le serveur, le pont ROS et la fenêtre Gazebo en arrière-plan.
Attendre la mise en posture des robots pendant 2 secondes simulées.

Pour obtenir une vue d'ensemble :

```bash
bash sim.sh overview
```

Pour revenir à la vue rapprochée : `bash sim.sh view`.
Pour fonctionner sans fenêtre : `bash sim.sh headless`, à la place de `start`.

### GPU NVIDIA (facultatif)

Par défaut, le rendu se fait sur le processeur. Pour utiliser une carte NVIDIA,
installer le pilote et le [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html).
Puis, dans chaque terminal utilisé pour ces commandes :

```bash
export SIM_GPU=nvidia
bash sim.sh start
```

## Déplacer un robot pour essayer le monde

Dans un autre terminal, depuis ce même dossier :

```bash
bash sim.sh drive go2_1
```

| Touche | Action |
|---|---|
| W / S | Avancer / reculer à 1,25 m/s |
| A / D | Se déplacer latéralement à 0,75 m/s |
| Q / E | Tourner à 1,75 rad/s |
| Espace | Arrêter |
| X | Quitter |

Maintenir la touche ; la commande s'annule après 0,3 s sans répétition.
Utiliser `go2_2` pour commander le second robot. Ces commandes servent aux essais
manuels de la simulation. Le contrôleur ne contourne pas les obstacles ni la chute.

## Voir les capteurs

```bash
bash sim.sh rviz go2_1
```

RViz affiche le LiDAR dans la vue principale et l'image de la caméra dans son
panneau Image. Remplacer `go2_1` par `go2_2` pour voir le second robot.
Les transformations nécessaires au LiDAR sont publiées pendant cette commande.

Chaque robot expose les topics suivants, avec son propre préfixe :

| Topic (premier robot) | Type ROS 2 |
|---|---|
| `/go2_1/cmd_vel` | `geometry_msgs/msg/Twist` |
| `/go2_1/scan` | `sensor_msgs/msg/LaserScan` |
| `/go2_1/camera/image` | `sensor_msgs/msg/Image` |
| `/go2_1/imu` | `sensor_msgs/msg/Imu` |
| `/go2_1/odom` | `nav_msgs/msg/Odometry` |
| `/go2_1/joint_states` | `sensor_msgs/msg/JointState` |

L'horloge simulée est publiée sur `/clock`. Les nœuds étudiants doivent utiliser
`use_sim_time:=true`. Pour consulter un topic depuis le conteneur :

```bash
bash sim.sh ros topic list
bash sim.sh ros topic echo /go2_1/scan --once --qos-reliability best_effort
```

Pour publier une vitesse depuis ROS, exécuter cette commande puis `Ctrl+C` :

```bash
bash sim.sh ros topic pub --rate 20 /go2_1/cmd_vel geometry_msgs/msg/Twist \
  '{linear: {x: 0.5}, angular: {z: 0.0}}'
```

Le contrôleur annule une commande non renouvelée après 0,5 s simulée.
Il limite les consignes à 2 m/s en longitudinal, 1,5 m/s en latéral et 2,5 rad/s
en rotation ; la vitesse atteinte dépend du terrain et de la dynamique du robot.

## Arrêter et diagnostiquer

```bash
bash sim.sh logs
bash sim.sh stop
```

`stop` arrête aussi les conteneurs de commande et de visualisation de ce projet.
Si une fenêtre ne s'ouvre pas, vérifier que `echo "$DISPLAY"` indique un affichage
et que `xauth` est installé. L'autorisation X11 est limitée au cookie de la session,
stocké dans `.runtime/`, qui est exclu de Git.

## Fichiers à modifier

- `worlds/tunnels.sdf` : monde, collisions, robots et capteurs.
- `models/` : maillages des robots et des pentes.
- `controller/` : contrôleur de locomotion compilé comme plugin Gazebo.
- `config/bridge.yaml` : correspondances entre les topics Gazebo et ROS.
- `config/` : vues Gazebo et RViz.
- `scripts/` et `sim.sh` : lancement et outils d'essai.

Après une modification, exécuter `bash sim.sh stop`, `bash sim.sh build`, puis
`bash sim.sh start` : les fichiers sont intégrés à l'image Docker.
Les services Compose partagent leur réseau ; utiliser `bash sim.sh ros ...`
pour communiquer avec eux. La connexion de nœuds ROS exécutés directement sur
l'hôte ou sur une autre machine nécessite une configuration réseau supplémentaire.

## Portée de la simulation

La géométrie provient du modèle officiel **Go2**. Le contrôleur est une adaptation
pour cette simulation : il utilise la pose réelle du simulateur et les plans
de hauteur connus du terrain. L'odométrie représente aussi la vérité du simulateur.
Les capteurs ajoutés sont génériques et ne reproduisent pas tous les capteurs
du robot physique. RViz fournit une vue des capteurs et des axes du robot.

Ce dossier ne valide ni un Go2X physique, ni l'association Humble/Fortress.
Les sources et licences des éléments tiers sont dans [NOTICE.md](NOTICE.md).
Les fichiers propres au projet sont sous [licence MIT](LICENSE).
