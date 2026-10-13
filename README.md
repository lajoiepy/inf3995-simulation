# INF3995 — Simulation de corridors avec deux Go2

## Le modèle

Le plus simple est d'utiliser le `worlds/tunnels.sdf` dans votre simulateur.

Les autres fichiers et scripts dans ce dépôt ne sont pas obligatoires, mais ils peuvent être utiles pour tester ce modèle.

## Démarrer

Depuis ce dossier :

```bash
bash sim.sh build
bash sim.sh start
```

Pour obtenir une vue d'ensemble :

```bash
bash sim.sh overview
```

Pour revenir à la vue rapprochée : `bash sim.sh view`.
Pour fonctionner sans fenêtre : `bash sim.sh headless`, à la place de `start`.

### GPU NVIDIA (facultatif)

Par défaut, le rendu se fait sur le CPU. Pour utiliser une carte NVIDIA,
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

## Voir les capteurs

```bash
bash sim.sh rviz go2_1
```

RViz affiche le LiDAR dans la vue principale et l'image de la caméra dans son
panneau Image. Remplacer `go2_1` par `go2_2` pour voir le second robot.
## Arrêter

```bash
bash sim.sh stop
```

## Compatibilité

**La configuration fournie a été validée avec Jazzy et Harmonic uniquement.**

Par contre, peu importe votre configuration, vous pouvez simplement utiliser l'environnement `worlds/tunnels.sdf` fourni dans ce dépôt et ajouter les robots et contrôleurs vous-mêmes.

### Si vous souhaitez utiliser le contrôleur fourni dans ce dépôt avec Humble

1. Elle peut être lancée sur un ordinateur où Humble est installé : Docker conserve
Jazzy à l'intérieur du conteneur. Cela ne valide pas les échanges entre des nœuds
Humble sur l'hôte et le pont Jazzy du conteneur.

2. Bien que [Humble est normalement associé à Gazebo Fortress](https://gazebosim.org/docs/fortress/ros_installation/).
Humble avec Harmonic est également possible avec le paquet alternatif
`ros-humble-ros-gzharmonic`, qui entre en conflit avec les paquets Humble `ros_gz`
standards. Cette option nécessiterait une image adaptée et des essais séparés.

