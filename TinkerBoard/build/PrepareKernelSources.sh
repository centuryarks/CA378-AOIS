#!/bin/sh

sudo apt update
sudo apt upgrade -y
sudo apt install -y git-core build-essential bc libssl-dev device-tree-compiler
sudo apt install -y qt4-default libqtgui4 v4l-utils ufraw
sudo apt install -y zlib1g-dev libjpeg-dev mplayer

git clone --depth 1 https://github.com/TinkerBoard/debian_kernel.git -b 2.0.8

