#!/usr/bin/env python3
"""Commandes clavier pour essayer la simulation."""
import select
import os
import sys
import termios
import time
import tty

import rclpy
from geometry_msgs.msg import Twist

robot = sys.argv[1] if len(sys.argv) > 1 else 'go2_1'
if robot not in ('go2_1', 'go2_2'):
    raise SystemExit('Choisir go2_1 ou go2_2.')
if not sys.stdin.isatty():
    raise SystemExit('Lancer cette commande dans un terminal interactif.')

rclpy.init()
node = rclpy.create_node('clavier_' + robot)
publisher = node.create_publisher(Twist, '/' + robot + '/cmd_vel', 10)
mapping = {
    'w': (1.25, 0.0, 0.0), 's': (-1.25, 0.0, 0.0),
    'a': (0.0, 0.75, 0.0), 'd': (0.0, -0.75, 0.0),
    'q': (0.0, 0.0, 1.75), 'e': (0.0, 0.0, -1.75),
}
print(f'{robot} : W/S avancer/reculer, A/D déplacement latéral, Q/E tourner. '
      'Maintenir une touche ; ESPACE arrêter ; X quitter. '
      'Attendre 2 secondes simulées pour la mise en posture.')
settings = termios.tcgetattr(sys.stdin)
command = Twist()
last_key = 0.0
try:
    tty.setcbreak(sys.stdin.fileno())
    while rclpy.ok():
        if select.select([sys.stdin], [], [], 0.05)[0]:
            key = os.read(sys.stdin.fileno(), 1).decode(errors='ignore').lower()
            if key in ('x', '\x03'):
                break
            command = Twist()
            if key in mapping:
                command.linear.x, command.linear.y, command.angular.z = mapping[key]
                last_key = time.monotonic()
        if time.monotonic() - last_key > 0.3:
            command = Twist()
        publisher.publish(command)
        rclpy.spin_once(node, timeout_sec=0)
finally:
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, settings)
    if rclpy.ok():
        publisher.publish(Twist())
    node.destroy_node()
    if rclpy.ok():
        rclpy.shutdown()
