#!/usr/bin/env python3
"""Transformations RViz obtenues de la simulation et de la pose du LiDAR."""
from pathlib import Path
import sys
import xml.etree.ElementTree as ET

import rclpy
from geometry_msgs.msg import TransformStamped
from nav_msgs.msg import Odometry
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data
from tf2_ros import StaticTransformBroadcaster, TransformBroadcaster

robot = sys.argv[1]
if robot not in ('go2_1', 'go2_2'):
    raise SystemExit('Choisir go2_1 ou go2_2.')
rclpy.init()
node = Node(robot + '_rviz_frames')
dynamic = TransformBroadcaster(node)
static = StaticTransformBroadcaster(node)
root = Path(__file__).resolve().parents[1]
model = ET.parse(root / 'worlds/tunnels.sdf').find("./world/model[@name='" + robot + "']")
sensor = model.find(".//link[@name='base']/sensor[@name='lidar']")
x, y, z, roll, pitch, yaw = map(float, sensor.findtext('pose').split())
if (roll, pitch, yaw) != (0.0, 0.0, 0.0):
    raise SystemExit('Adapter la transformation RViz pour cette orientation du LiDAR.')
transform = TransformStamped()
transform.header.frame_id = robot + '/base'
transform.child_frame_id = sensor.findtext('gz_frame_id')
transform.transform.translation.x = x
transform.transform.translation.y = y
transform.transform.translation.z = z
transform.transform.rotation.w = 1.0
static.sendTransform(transform)


def receive(msg):
    transform = TransformStamped()
    transform.header = msg.header
    transform.child_frame_id = msg.child_frame_id
    transform.transform.translation.x = msg.pose.pose.position.x
    transform.transform.translation.y = msg.pose.pose.position.y
    transform.transform.translation.z = msg.pose.pose.position.z
    transform.transform.rotation = msg.pose.pose.orientation
    dynamic.sendTransform(transform)


subscription = node.create_subscription(
    Odometry, '/' + robot + '/odom', receive, qos_profile_sensor_data)
try:
    rclpy.spin(node)
except KeyboardInterrupt:
    pass
finally:
    node.destroy_node()
    if rclpy.ok():
        rclpy.shutdown()
