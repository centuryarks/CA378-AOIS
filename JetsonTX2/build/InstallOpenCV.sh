#!/bin/bash
# Build and Install OpenCV
git clone https://github.com/jetsonhacks/buildOpenCVTX2.git
cd buildOpenCVTX2
./buildOpenCV.sh
cd ~/opencv/build
sudo make install
