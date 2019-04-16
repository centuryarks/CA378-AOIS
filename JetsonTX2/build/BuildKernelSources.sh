#!/bin/bash

# Connection setting
echo -n "What is the number of camera connections? : "
read ans
CAM_NUM=$ans
# FPS setting
echo -n "What is the number of framerate for 4056x3040 ? (30/24/20/15/12/10/6/5) : "
read ans
FPS_4K3K=$ans

echo -n "What is the number of framerate for 3840x2160 ? (30/24/20/15/12/10) : "
read ans
FPS_4K2K=$ans

echo -n "What is the number of framerate for 1920x1080 ? (120/96/80/60/48/40/30) : "
read ans
FPS_FHD=$ans

echo -n "What is the number of framerate for 640x480 ? (512/500/480/400/350/300/240/200/150/120/60) : "
read ans
FPS_VGA=$ans
./scripts/generate_tegra186-camera-imx378.sh ./dts/tegra186-camera-imx378-c03.dtsi ${FPS_4K3K} ${FPS_4K2K} ${FPS_FHD} ${FPS_VGA} ${CAM_NUM}
./scripts/generate_imx378_frame_setting.sh ./src/imx378_frame_setting.h ${FPS_4K3K} ${FPS_4K2K} ${FPS_FHD} ${FPS_VGA}
cp ./dts/tegra186-camera-imx378-c03.dtsi ./hardware/nvidia/platform/t18x/quill/kernel-dts/quill-modules/tegra186-camera-imx378-c03.dtsi
cp ./src/imx378_mode_tbls.h ./kernel/kernel-4.4/drivers/media/i2c/imx378_mode_tbls.h
cp ./src/imx378_frame_setting.h ./kernel/kernel-4.4/drivers/media/i2c/

# Copy source
cp -r ./kernel/ /usr/src/
cp -r ./hardware/ /usr/src/

# Copy config
sudo rm /usr/src/kernel/kernel-4.4/.config
zcat ./config.gz > /usr/src/kernel/kernel-4.4/.config

# Build kernel and modules
sudo ./scripts/makeKernel.sh
sudo ./scripts/copyImage.sh
