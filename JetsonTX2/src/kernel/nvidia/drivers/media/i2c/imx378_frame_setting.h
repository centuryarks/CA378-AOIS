/*
 * Copyright (c) 2017, CenturyArks All rights reserved.
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __IMX378_FRAME_SETTING__
#define __IMX378_FRAME_SETTING__

#define IMX378_FARAME_H_4K3K                    (8750)
#define IMX378_FARAME_V_4K3K                    (3200)
#define IMX378_FARAME_H_4K2K                    (12500)
#define IMX378_FARAME_V_4K2K                    (2240)
#define IMX378_FARAME_H_FHD                     (12444)
#define IMX378_FARAME_V_FHD                     (1125)
#define IMX378_FARAME_H_VGA                     (4000)
#define IMX378_FARAME_V_VGA                     (1050)
#define IMX378_FPS_4K3K                         (30)
#define IMX378_FPS_4K2K                         (30)
#define IMX378_FPS_FHD                          (60)
#define IMX378_FPS_VGA                          (200)
#define IMX378_DATA_RATE_4K3K                   (348*6/1*2)     // Datarate(Mbps) for 4K3K
#define IMX378_DATA_RATE_4K2K                   (238*6/1*2)     // Datarate(Mbps) for 4K2K
#define IMX378_DATA_RATE_FHD                    (252*6/2*2)     // Datarate(Mbps) for FHD
#define IMX378_DATA_RATE_VGA                    (280*6/2*2)     // Datarate(Mbps) for VGA
#define IMX378_PLL_MULTI_VIDEO                  (350)           // PLL multiplier for Internal Video Timing System Clock
#define IMX378_PLL_MULTI_4K3K                   (348)           // PLL multiplier for Internal Output Pixel System
#define IMX378_PLL_MULTI_4K2K                   (238)           // PLL multiplier for Internal Output Pixel System
#define IMX378_PLL_MULTI_FHD                    (252)           // PLL multiplier for Internal Output Pixel System
#define IMX378_PLL_MULTI_VGA                    (280)           // PLL multiplier for Internal Output Pixel System
#define IMX378_SYCK_DIV_4K3K                    (1)             // System Clock Divider
#define IMX378_SYCK_DIV_4K2K                    (1)             // System Clock Divider
#define IMX378_SYCK_DIV_FHD                     (2)             // System Clock Divider
#define IMX378_SYCK_DIV_VGA                     (2)             // System Clock Divider
#define IMX378_POWER_SAVE_RATE_4K3K             (IMX378_FARAME_H_4K3K*18/840+1)   // 18Mhz/840Mpix
#define IMX378_POWER_SAVE_RATE_4K2K             (IMX378_FARAME_H_4K2K*18/840+1)   // 18Mhz/840Mpix
#define IMX378_POWER_SAVE_RATE_FHD              (IMX378_FARAME_H_FHD*18/840+1)    // 18Mhz/840Mpix
#define IMX378_POWER_SAVE_RATE_VGA              (IMX378_FARAME_H_VGA*18/840+1)    // 18Mhz/840Mpix

#endif  /* __IMX378_FRAME_SETTING__ */
