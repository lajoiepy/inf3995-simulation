# Ressources et licences

Le monde, les maillages de terrain, les scripts et l'adaptateur Gazebo sont sous
licence MIT : voir [LICENSE](LICENSE). Les éléments tiers gardent leurs licences.

| Ressource | Source retenue | Licence locale |
|---|---|---|
| Géométrie Go2, convertie d'URDF en SDF ; maillages officiels inchangés | [Unitree RL Gym](https://github.com/unitreerobotics/unitree_rl_gym/tree/276801e46c5d433564f24658bac64f254b7d2d4b/resources/robots/go2) | [BSD 3-Clause](models/unitree_go2/LICENSE) |
| Algorithmes de locomotion adaptés à Go2, au temps simulé et aux pentes ; fonctions ROS 1, navigation et matériel retirées | [Unitree Guide](https://github.com/unitreerobotics/unitree_guide/tree/fdf4d23de6affe8ee38fb4d892f61053fa1fcbcb) | [BSD 3-Clause](controller/vendor/guide/LICENSE) |
| Solveur QuadProg++ inclus dans Unitree Guide | Luca Di Gaspero et contributeurs ; même révision que ci-dessus | [MIT](controller/vendor/guide/QUADPROG-LICENSE) |
| Configuration graphique Gazebo, adaptée pour la taille de fenêtre et la caméra | [Gazebo Sim 8.15.0](https://github.com/gazebosim/gz-sim/blob/gz-sim8_8.15.0/src/gui/gui.config) | [Apache 2.0](licenses/Apache-2.0.txt), [notice](licenses/gz-sim-LICENSE.txt) |

Les paquets Ubuntu, ROS et Gazebo installés dans l'image conservent leurs notices
sous `/usr/share/doc` et dans leurs répertoires d'installation.
