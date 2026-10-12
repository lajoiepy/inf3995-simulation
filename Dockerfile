FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG ROS_APT_VERSION=1.3.0
ENV LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8
SHELL ["/bin/bash", "-eo", "pipefail", "-c"]

RUN apt-get update && apt-get install -y --no-install-recommends \
      locales software-properties-common curl ca-certificates \
      python3 build-essential cmake libeigen3-dev && \
    locale-gen en_US.UTF-8 && add-apt-repository -y universe && \
    curl -fL -o /tmp/ros2-apt-source.deb \
      "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_VERSION}/ros2-apt-source_${ROS_APT_VERSION}.noble_all.deb" && \
    dpkg -i /tmp/ros2-apt-source.deb && apt-get update && \
    apt-get install -y --no-install-recommends \
      ros-jazzy-ros-base ros-jazzy-ros-gz ros-jazzy-rviz2 ros-jazzy-tf2-ros && \
    rm -rf /var/lib/apt/lists/* /tmp/ros2-apt-source.deb

COPY controller /tmp/controller
RUN source /opt/ros/jazzy/setup.bash && \
    cmake -S /tmp/controller -B /tmp/build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/inf3995 && \
    cmake --build /tmp/build -j4 && cmake --install /tmp/build && \
    rm -rf /tmp/controller /tmp/build

ENV GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/inf3995/lib
WORKDIR /app
COPY worlds worlds
COPY models models
COPY config config
COPY scripts scripts
COPY LICENSE NOTICE.md ./
COPY licenses licenses
COPY controller/vendor/guide/LICENSE controller/vendor/guide/LICENSE
COPY controller/vendor/guide/QUADPROG-LICENSE controller/vendor/guide/QUADPROG-LICENSE
USER ubuntu
ENTRYPOINT ["bash", "/app/scripts/entrypoint.sh"]
CMD ["bash", "scripts/launch.sh", "server"]
