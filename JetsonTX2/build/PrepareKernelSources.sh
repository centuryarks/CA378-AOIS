#!/bin/bash
# Prepare kernel
sudo ./scripts/removeAllKernelSources.sh
sudo ./scripts/getKernelSources.sh
sudo rm -r /usr/src/public_release/
