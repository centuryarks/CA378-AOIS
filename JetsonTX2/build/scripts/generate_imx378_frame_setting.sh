#!/bin/bash

FNAME=$1
FPS_4K3K=$2
FPS_4K2K=$3
FPS_FHD=$4
FPS_VGA=$5

source ./scripts/generate_framerate.sh

touch ${FNAME}
echo '/*'>${FNAME}
echo ' * Copyright (c) 2017, CenturyArks All rights reserved.'>>${FNAME}
echo ' * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.'>>${FNAME}
echo ' *'>>${FNAME}
echo ' * This program is free software; you can redistribute it and/or modify'>>${FNAME}
echo ' * it under the terms of the GNU General Public License as published by'>>${FNAME}
echo ' * the Free Software Foundation; either version 2 of the License, or'>>${FNAME}
echo ' * (at your option) any later version.'>>${FNAME}
echo ' *'>>${FNAME}
echo ' * This program is distributed in the hope that it will be useful, but WITHOUT'>>${FNAME}
echo ' * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or'>>${FNAME}
echo ' * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for'>>${FNAME}
echo ' * more details.'>>${FNAME}
echo ' *'>>${FNAME}
echo ' * You should have received a copy of the GNU General Public License'>>${FNAME}
echo ' * along with this program.  If not, see <http://www.gnu.org/licenses/>.'>>${FNAME}
echo ' */'>>${FNAME}
echo>>${FNAME}
echo '#ifndef __IMX378_FRAME_SETTING__'>>${FNAME}
echo '#define __IMX378_FRAME_SETTING__'>>${FNAME}
echo>>${FNAME}
echo "#define IMX378_FARAME_H_4K3K                    (${LINE_LENGTH_4K3K_DUAL})">>${FNAME}
echo "#define IMX378_FARAME_V_4K3K                    (${FRAME_LENGTH[0]})">>${FNAME}
echo "#define IMX378_FARAME_H_4K2K                    (${LINE_LENGTH_4K2K_DUAL})">>${FNAME}
echo "#define IMX378_FARAME_V_4K2K                    (${FRAME_LENGTH[1]})">>${FNAME}
echo "#define IMX378_FARAME_H_FHD                     (${LINE_LENGTH_FHD_DUAL})">>${FNAME}
echo "#define IMX378_FARAME_V_FHD                     (${FRAME_LENGTH[2]})">>${FNAME}
echo "#define IMX378_FARAME_H_VGA                     (${LINE_LENGTH_VGA_DUAL})">>${FNAME}
echo "#define IMX378_FARAME_V_VGA                     (${FRAME_LENGTH[3]})">>${FNAME}
echo "#define IMX378_FPS_4K3K                         (${FPS[0]})">>${FNAME}
echo "#define IMX378_FPS_4K2K                         (${FPS[1]})">>${FNAME}
echo "#define IMX378_FPS_FHD                          (${FPS[2]})">>${FNAME}
echo "#define IMX378_FPS_VGA                          (${FPS[3]})">>${FNAME}
echo "#define IMX378_DATA_RATE_4K3K                   (${PLL_MULTI[0]}*6/${SYCK_DIV[0]}*2)     // Datarate(Mbps) for 4K3K">>${FNAME}
echo "#define IMX378_DATA_RATE_4K2K                   (${PLL_MULTI[1]}*6/${SYCK_DIV[1]}*2)     // Datarate(Mbps) for 4K2K">>${FNAME}
echo "#define IMX378_DATA_RATE_FHD                    (${PLL_MULTI[2]}*6/${SYCK_DIV[2]}*2)     // Datarate(Mbps) for FHD">>${FNAME}
echo "#define IMX378_DATA_RATE_VGA                    (${PLL_MULTI[3]}*6/${SYCK_DIV[3]}*2)     // Datarate(Mbps) for VGA">>${FNAME}
echo "#define IMX378_PLL_MULTI_VIDEO                  (350)           // PLL multiplier for Internal Video Timing System Clock">>${FNAME}
echo "#define IMX378_PLL_MULTI_4K3K                   (${PLL_MULTI[0]})           // PLL multiplier for Internal Output Pixel System">>${FNAME}
echo "#define IMX378_PLL_MULTI_4K2K                   (${PLL_MULTI[1]})           // PLL multiplier for Internal Output Pixel System">>${FNAME}
echo "#define IMX378_PLL_MULTI_FHD                    (${PLL_MULTI[2]})           // PLL multiplier for Internal Output Pixel System">>${FNAME}
echo "#define IMX378_PLL_MULTI_VGA                    (${PLL_MULTI[3]})           // PLL multiplier for Internal Output Pixel System">>${FNAME}
echo "#define IMX378_SYCK_DIV_4K3K                    (${SYCK_DIV[0]})             // System Clock Divider">>${FNAME}
echo "#define IMX378_SYCK_DIV_4K2K                    (${SYCK_DIV[1]})             // System Clock Divider">>${FNAME}
echo "#define IMX378_SYCK_DIV_FHD                     (${SYCK_DIV[2]})             // System Clock Divider">>${FNAME}
echo "#define IMX378_SYCK_DIV_VGA                     (${SYCK_DIV[3]})             // System Clock Divider">>${FNAME}
echo "#define IMX378_POWER_SAVE_RATE_4K3K             (IMX378_FARAME_H_4K3K*18/840+1)   // 18Mhz/840Mpix">>${FNAME}
echo "#define IMX378_POWER_SAVE_RATE_4K2K             (IMX378_FARAME_H_4K2K*18/840+1)   // 18Mhz/840Mpix">>${FNAME}
echo "#define IMX378_POWER_SAVE_RATE_FHD              (IMX378_FARAME_H_FHD*18/840+1)    // 18Mhz/840Mpix">>${FNAME}
echo "#define IMX378_POWER_SAVE_RATE_VGA              (IMX378_FARAME_H_VGA*18/840+1)    // 18Mhz/840Mpix">>${FNAME}
echo>>${FNAME}
echo '#endif  /* __IMX378_FRAME_SETTING__ */'>>${FNAME}
