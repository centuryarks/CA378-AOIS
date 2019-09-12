#!/bin/bash

# Copy source
sudo cp -r ./kernel/ /usr/src/
sudo cp -r ./hardware/ /usr/src/

# Copy config
sudo rm /usr/src/kernel/kernel-4.9/.config
sudo sh -c "zcat ./config.gz > /usr/src/kernel/kernel-4.9/.config"
#gzip -d ./config.gz
#sudo cp ./config /usr/src/kernel/kernel-4.9/.config

# Build kernel and modules
sudo ./scripts/makeKernel.sh
sudo ./scripts/copyImage.sh
