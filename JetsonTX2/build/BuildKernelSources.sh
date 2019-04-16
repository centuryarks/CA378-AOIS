#!/bin/bash

# Connection setting
echo -n "What is the number of camera connections? : "
read ans
CAM_NUM=$ans
if [ ${CAM_NUM} = 1 ]; then
 cp ./dts/tegra186-camera-imx378-c03-1u.dtsi ./hardware/nvidia/platform/t18x/quill/kernel-dts/quill-modules/tegra186-camera-imx378-c03-cu.dtsi
elif [ ${CAM_NUM} = 2 ]; then
 cp ./dts/tegra186-camera-imx378-c03-2u.dtsi ./hardware/nvidia/platform/t18x/quill/kernel-dts/quill-modules/tegra186-camera-imx378-c03-cu.dtsi
elif [ ${CAM_NUM} = 3 ]; then
 cp ./dts/tegra186-camera-imx378-c03-3u.dtsi ./hardware/nvidia/platform/t18x/quill/kernel-dts/quill-modules/tegra186-camera-imx378-c03-cu.dtsi
elif [ ${CAM_NUM} = 4 ]; then
 cp ./dts/tegra186-camera-imx378-c03-4u.dtsi ./hardware/nvidia/platform/t18x/quill/kernel-dts/quill-modules/tegra186-camera-imx378-c03-cu.dtsi
elif [ ${CAM_NUM} = 5 ]; then
 cp ./dts/tegra186-camera-imx378-c03-5u.dtsi ./hardware/nvidia/platform/t18x/quill/kernel-dts/quill-modules/tegra186-camera-imx378-c03-cu.dtsi
else
 cp ./dts/tegra186-camera-imx378-c03-6u.dtsi ./hardware/nvidia/platform/t18x/quill/kernel-dts/quill-modules/tegra186-camera-imx378-c03-cu.dtsi
fi

# Copy source
cp -r ./kernel/ /usr/src/
cp -r ./hardware/ /usr/src/

# Copy config
zcat ./config.gz > /usr/src/kernel/kernel-4.4/.config

# Build kernel and modules
sudo ./scripts/makeKernel.sh
sudo ./scripts/copyImage.sh
