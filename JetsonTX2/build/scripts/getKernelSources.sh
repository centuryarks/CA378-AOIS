#!/bin/bash
apt-add-repository universe
apt-get update
# apt-get install qt5-default pkg-config -y
cd /usr/src
wget -N https://developer.download.nvidia.com/embedded/L4T/r28_Release_v2.1/public_sources.tbz2
sudo tar -xvf public_sources.tbz2 public_release/kernel_src.tbz2
tar -xvf public_release/kernel_src.tbz2
# Space is tight; get rid of the compressed kernel source
rm -r public_release
cd kernel/kernel-4.4
# zcat /proc/config.gz > .config
# Ready to configure kernel
# make xconfig

