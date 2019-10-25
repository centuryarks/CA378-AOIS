#!/bin/sh

cp -r debian_kernel_cp/arch debian_kernel
cp -r debian_kernel_cp/drivers debian_kernel
sudo sed -i -e "s/CONFIG_VIDEO_IMX219=y/CONFIG_VIDEO_IMX219=y\nCONFIG_VIDEO_IMX378=y/g" debian_kernel/arch/arm/configs/miniarm-rk3288_defconfig


cd debian_kernel
make ARCH=arm miniarm-rk3288_defconfig
make ARCH=arm -j6 zImage
sudo make ARCH=arm -j6 modules
make ARCH=arm dtbs

sudo make ARCH=arm modules_install
sudo cp -v arch/arm/boot/zImage /boot
sudo cp -v arch/arm/boot/dts/rk3288-miniarm.dtb /boot