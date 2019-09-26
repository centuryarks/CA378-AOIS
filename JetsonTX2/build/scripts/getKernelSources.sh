#!/bin/bash
#apt-add-repository universe
apt-get update
apt-get install v4l-utils -y
apt-get install ufraw -y
apt-get install qt4-dev-tools qt4-qmlviewer -y
apt-get install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools -y
# apt-get install qt5-default pkg-config -y
cd /usr/src

# for JetPack4.2
wget -N https://developer.download.nvidia.com/embedded/L4T/r32_Release_v1.0/jax-tx2/BSP/JAX-TX2-public_sources.tbz2

# Extract the source
mv JAX-TX2-public_sources.tbz2 public_sources.tbz2
tar -xvf public_sources.tbz2 public_sources/kernel_src.tbz2
tar -xvf public_sources/kernel_src.tbz2
sudo chown `whoami` kernel/ -R
sudo chown `whoami` hardware/ -R

# Space is tight; get rid of the compressed kernel source
rm -r public_sources
cd kernel/kernel-4.9
