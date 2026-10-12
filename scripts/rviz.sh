#!/usr/bin/env bash
set -eo pipefail
cd -- "$(dirname -- "$0")/.."
ROBOT="${1:-go2_1}"
case "$ROBOT" in go2_1|go2_2) ;; *) printf 'Choisir go2_1 ou go2_2.\n' >&2; exit 2 ;; esac
python3 scripts/rviz_frames.py "$ROBOT" --ros-args -p use_sim_time:=true &
FRAME_PID=$!
cleanup() { kill -INT "$FRAME_PID" 2>/dev/null || true; wait "$FRAME_PID" 2>/dev/null || true; }
trap cleanup EXIT
rviz2 -d "config/$ROBOT.rviz" --ros-args -p use_sim_time:=true
