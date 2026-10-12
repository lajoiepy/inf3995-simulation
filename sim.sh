#!/usr/bin/env bash
set -eo pipefail
cd -- "$(dirname -- "$0")"
COMPOSE=(docker compose -f compose.yaml)
if [[ "${SIM_GPU:-}" == nvidia ]]; then
  COMPOSE+=(-f compose.nvidia.yaml)
fi
prepare_gui() {
  bash scripts/prepare_gui.sh
  export SIM_XAUTH="$PWD/.runtime/xauth"
}
ACTION="${1:-help}"
[[ $# -eq 0 ]] || shift
case "$ACTION" in
  build) "${COMPOSE[@]}" build server ;;
  start) prepare_gui; "${COMPOSE[@]}" up -d server bridge gui ;;
  headless) "${COMPOSE[@]}" up -d server bridge ;;
  view) prepare_gui; "${COMPOSE[@]}" up -d gui ;;
  overview) prepare_gui; export GUI_CONFIG=config/gui-overview.config; "${COMPOSE[@]}" up -d gui ;;
  drive) "${COMPOSE[@]}" run --rm --no-deps tools python3 scripts/teleop.py "${1:-go2_1}" ;;
  rviz) prepare_gui; "${COMPOSE[@]}" run --rm --no-deps gui bash scripts/rviz.sh "${1:-go2_1}" ;;
  ros) "${COMPOSE[@]}" run --rm --no-deps tools ros2 "$@" ;;
  logs) "${COMPOSE[@]}" logs --tail 100 "$@" ;;
  stop) "${COMPOSE[@]}" --profile gui --profile tools down --remove-orphans ;;
  help) printf 'Usage : bash sim.sh {build|start|headless|view|overview|drive [go2_1|go2_2]|rviz [go2_1|go2_2]|ros ...|logs|stop}\n' ;;
  *) printf 'Commande inconnue : %s. Voir : bash sim.sh help\n' "$ACTION" >&2; exit 2 ;;
esac
