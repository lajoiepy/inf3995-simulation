#!/usr/bin/env bash
set -eo pipefail
source /opt/ros/jazzy/setup.bash
export GZ_SIM_RESOURCE_PATH="/app/models:${GZ_SIM_RESOURCE_PATH:-}"
export ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-100}"
export GZ_PARTITION="${GZ_PARTITION:-inf3995_simulation}"
exec "$@"
