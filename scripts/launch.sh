#!/usr/bin/env bash
set -eo pipefail
cd -- "$(dirname -- "$0")/.."
case "${1:-server}" in
  server) exec gz sim -r -s --headless-rendering worlds/tunnels.sdf ;;
  bridge) exec ros2 run ros_gz_bridge parameter_bridge --ros-args -p config_file:="$PWD/config/bridge.yaml" ;;
  gui) exec gz sim -g --gui-config "${GUI_CONFIG:-config/gui.config}" ;;
  *) printf 'Mode inconnu : %s\n' "$1" >&2; exit 2 ;;
esac
