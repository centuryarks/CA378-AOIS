#!/bin/bash
apt-add-repository universe
apt-get update
apt-get install v4l-utils -y
apt-get install ufraw -y
apt-get install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools -y
# apt-get install qt5-default pkg-config -y
cd /usr/src
wget https://developer.nvidia.com/embedded/dlc/l4t-sources-32-1-jetson-nano -O Jetson-Nano-public_sources.tbz2
sudo tar -xvf Jetson-Nano-public_sources.tbz2 public_sources/kernel_src.tbz2
tar -xvf public_sources/kernel_src.tbz2
# Space is tight; get rid of the compressed kernel source
rm -r public_sources
cd kernel/kernel-4.9
# zcat /proc/config.gz > .config
# Ready to configure kernel
# make xconfig

