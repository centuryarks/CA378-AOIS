/*
 * imx378.c - imx378 sensor driver
 *
 * Copyright (c) 2017, CenturyArks All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __IMX378_MODE_TABLES__
#define __IMX378_MODE_TABLES__

#include <media/camera_common.h>


#define IMX378_TABLE_WAIT_MS                    0
#define IMX378_TABLE_END                        1
#define IMX378_WAIT_MS                          1

#define IMX378_EXPOSURE_HIGH                    (0x0202)
#define IMX378_EXPOSURE_LOW                     (0x0203)
#define IMX378_ANALOG_GAIN_HIGH                 (0x0204)
#define IMX378_ANALOG_GAIN_LOW                  (0x0205)
#define IMX378_DIG_GAIN_GR_HIGH                 (0x020E)
#define IMX378_DIG_GAIN_GR_LOW                  (0x020F)
#define IMX378_DIG_GAIN_R_HIGH                  (0x0210)
#define IMX378_DIG_GAIN_R_LOW                   (0x0211)
#define IMX378_DIG_GAIN_B_HIGH                  (0x0212)
#define IMX378_DIG_GAIN_B_LOW                   (0x0213)
#define IMX378_DIG_GAIN_GB_HIGH                 (0x0214)
#define IMX378_DIG_GAIN_GB_LOW                  (0x0215)
#define IMX378_FRAME_LENGTH_HIGH                (0x0340)
#define IMX378_FRAME_LENGTH_LOW                 (0x0341)
#define IMX378_LINE_LENGTH_HIGH                 (0x0342)
#define IMX378_LINE_LENGTH_LOW                  (0x0343)
#define IMX378_MANUAL_DATA_PEDESTAL_EN          (0x3030)
#define IMX378_MANUAL_DATA_PEDESTAL_VALUE_HIGH  (0x3032)
#define IMX378_MANUAL_DATA_PEDESTAL_VALUE_LOW   (0x3033)
#define IMX378_POWER_SAVE_MODE                  (0x3F50)
#define IMX378_POWER_SAVE_HIGH                  (0x3F56)
#define IMX378_POWER_SAVE_LOW                   (0x3F57)
#define IMX378_DPGA_USE_GLOBAL_GAIN             (0x3FF9)

#define IMX378_DATA_RATE_21                     (2088*2)        // 2.1Gbps x2
#define IMX378_DATA_RATE_15                     (1476*2)        // 1.5Gbps x2
#define IMX378_PLL_MULTI_21                     (2088*3/18)     // PLL
#define IMX378_PLL_MULTI_15                     (1476*3/18)     // PLL
#define IMX378_POWER_SAVE_RATE_21               (18/835)        // 18Mhz/835Mpix
#define IMX378_POWER_SAVE_RATE_15               (18/590)        // 18Mhz/590Mpix
#define IMX378_DEFAULT_COARSE_STORAGE_TIME      (0x03E8)        // 1000
#define IMX378_DEFAULT_ANALOG_GAIN              (0x0000)        // ISO100
#define IMX378_DEFAULT_PEDESTAL_VALUE           (0x40)          // 64
#define IMX378_DEFAULT_DIG_GAIN_GR              (100*256/100)   // GR: 1.00
#define IMX378_DEFAULT_DIG_GAIN_R               (213*256/100)   // R:  2.13
#define IMX378_DEFAULT_DIG_GAIN_B               (193*256/100)   // G:  1.93
#define IMX378_DEFAULT_DIG_GAIN_GB              (100*256/100)   // GB: 1.00
#define IMX378_FARAME_H_4K3K                    (4350*2)
#define IMX378_FARAME_V_4K3K                    (3200)
#define IMX378_FARAME_H_4K2K                    (4100*2)
#define IMX378_FARAME_V_4K2K                    (2400)
#define IMX378_FARAME_H_FHD                     (2050*2)
#define IMX378_FARAME_V_FHD                     (1200)
#define IMX378_FARAME_H_VGA                     (1500*2)
#define IMX378_FARAME_V_VGA                     (820)

#define imx378_reg struct reg_8

/* Coarse Storage Time (Exposure) */
#define COARSE_STORAGE_TIME \
        {IMX378_EXPOSURE_HIGH, (((IMX378_DEFAULT_COARSE_STORAGE_TIME) >> 8) & 0xFF)},\
        {IMX378_EXPOSURE_LOW,  (((IMX378_DEFAULT_COARSE_STORAGE_TIME)     ) & 0xFF)},

/* Analog Gain */
#define ANALOG_GAIN \
        {IMX378_ANALOG_GAIN_HIGH, (((IMX378_DEFAULT_ANALOG_GAIN) >> 8) & 0xFF)},\
        {IMX378_ANALOG_GAIN_LOW,  (((IMX378_DEFAULT_ANALOG_GAIN)     ) & 0xFF)},

/* Digital Gain (White Blance) */
#define DIGITAL_GAIN \
        {IMX378_DPGA_USE_GLOBAL_GAIN, 0x00},\
        {IMX378_DIG_GAIN_GR_HIGH, (((IMX378_DEFAULT_DIG_GAIN_GR) >> 8) & 0xFF)},\
        {IMX378_DIG_GAIN_GR_LOW,  (((IMX378_DEFAULT_DIG_GAIN_GR)     ) & 0xFF)},\
        {IMX378_DIG_GAIN_R_HIGH,  (((IMX378_DEFAULT_DIG_GAIN_R)  >> 8) & 0xFF)},\
        {IMX378_DIG_GAIN_R_LOW,   (((IMX378_DEFAULT_DIG_GAIN_R)      ) & 0xFF)},\
        {IMX378_DIG_GAIN_B_HIGH,  (((IMX378_DEFAULT_DIG_GAIN_B)  >> 8) & 0xFF)},\
        {IMX378_DIG_GAIN_B_LOW,   (((IMX378_DEFAULT_DIG_GAIN_B)      ) & 0xFF)},\
        {IMX378_DIG_GAIN_GB_HIGH, (((IMX378_DEFAULT_DIG_GAIN_GB) >> 8) & 0xFF)},\
        {IMX378_DIG_GAIN_GB_LOW,  (((IMX378_DEFAULT_DIG_GAIN_GB)     ) & 0xFF)},

/* Pedestal (Black Level) */
#define PEDESTAL \
        {IMX378_MANUAL_DATA_PEDESTAL_EN, 0x01},\
        {IMX378_MANUAL_DATA_PEDESTAL_VALUE_HIGH, (((IMX378_DEFAULT_PEDESTAL_VALUE) >> 8) & 0xFF)},\
        {IMX378_MANUAL_DATA_PEDESTAL_VALUE_LOW,  (((IMX378_DEFAULT_PEDESTAL_VALUE)     ) & 0xFF)},

/* External clock frequency (18MHz) */
#define EXTERNAL_CLOCK \
        {0x0136, 0x12},\
        {0x0137, 0x00},

/* MIPI RAW10 */
#define MIPI_RAW10 \
        {0x0112, 0x0A},\
        {0x0113, 0x0A},

/* MIPI 2-lane */
#define MIPI_2LANE \
        {0x0114, 0x01},

/* Clock Setting */
#define CLOCK_SETTING_RAW10_21 \
        {0x0301, 0x05},\
        {0x0303, 0x02},\
        {0x0305, 0x03},\
        {0x0306, (((IMX378_PLL_MULTI_21) >> 8) & 0xFF)},\
        {0x0307, (((IMX378_PLL_MULTI_21)     ) & 0xFF)},\
        {0x0309, 0x0A},\
        {0x030B, 0x01},\
        {0x030D, 0x02},\
        {0x030E, (((IMX378_PLL_MULTI_21) >> 8) & 0xFF)},\
        {0x030F, (((IMX378_PLL_MULTI_21)     ) & 0xFF)},\
        {0x0310, 0x00},
#define CLOCK_SETTING_RAW10_15 \
        {0x0301, 0x05},\
        {0x0303, 0x02},\
        {0x0305, 0x03},\
        {0x0306, (((IMX378_PLL_MULTI_15) >> 8) & 0xFF)},\
        {0x0307, (((IMX378_PLL_MULTI_15)     ) & 0xFF)},\
        {0x0309, 0x0A},\
        {0x030B, 0x01},\
        {0x030D, 0x02},\
        {0x030E, (((IMX378_PLL_MULTI_15) >> 8) & 0xFF)},\
        {0x030F, (((IMX378_PLL_MULTI_15)     ) & 0xFF)},\
        {0x0310, 0x00},

/* Output Data Rate */
#define OUTPUT_DATA_RATE_21 \
        {0x0820, (((IMX378_DATA_RATE_21) >> 8) & 0xFF)},\
        {0x0821, (((IMX378_DATA_RATE_21)     ) & 0xFF)},\
        {0x0822, 0x00},\
        {0x0823, 0x00},
#define OUTPUT_DATA_RATE_15 \
        {0x0820, (((IMX378_DATA_RATE_15) >> 8) & 0xFF)},\
        {0x0821, (((IMX378_DATA_RATE_15)     ) & 0xFF)},\
        {0x0822, 0x00},\
        {0x0823, 0x00},

/* Output Data Select Setting */
#define OUTPUT_DATA_SELECT \
        {0x3E20, 0x01},\
        {0x3E37, 0x01},

/* Global Setting Header */
#define GLOBAL_SETTING_HEADER \
        {0xE000, 0x00},\
        {0x4AE9, 0x18},\
        {0x4AEA, 0x08},\
        {0xF61C, 0x04},\
        {0xF61E, 0x04},\
        {0x4AE9, 0x21},\
        {0x4AEA, 0x80},\
        {0x38A8, 0x1F},\
        {0x38A9, 0xFF},\
        {0x38AA, 0x1F},\
        {0x38AB, 0xFF},\
        {0x55D4, 0x00},\
        {0x55D5, 0x00},\
        {0x55D6, 0x07},\
        {0x55D7, 0xFF},\
        {0x55E8, 0x07},\
        {0x55E9, 0xFF},\
        {0x55EA, 0x00},\
        {0x55EB, 0x00},\
        {0x574C, 0x07},\
        {0x574D, 0xFF},\
        {0x574E, 0x00},\
        {0x574F, 0x00},\
        {0x5754, 0x00},\
        {0x5755, 0x00},\
        {0x5756, 0x07},\
        {0x5757, 0xFF},\
        {0x5973, 0x04},\
        {0x5974, 0x01},\
        {0x5D13, 0xC3},\
        {0x5D14, 0x58},\
        {0x5D15, 0xA3},\
        {0x5D16, 0x1D},\
        {0x5D17, 0x65},\
        {0x5D18, 0x8C},\
        {0x5D1A, 0x06},\
        {0x5D1B, 0xA9},\
        {0x5D1C, 0x45},\
        {0x5D1D, 0x3A},\
        {0x5D1E, 0xAB},\
        {0x5D1F, 0x15},\
        {0x5D21, 0x0E},\
        {0x5D22, 0x52},\
        {0x5D23, 0xAA},\
        {0x5D24, 0x7D},\
        {0x5D25, 0x57},\
        {0x5D26, 0xA8},\
        {0x5D37, 0x5A},\
        {0x5D38, 0x5A},\
        {0x5D77, 0x7F},\
        {0x7B75, 0x0E},\
        {0x7B76, 0x0B},\
        {0x7B77, 0x08},\
        {0x7B78, 0x0A},\
        {0x7B79, 0x47},\
        {0x7B7C, 0x00},\
        {0x7B7D, 0x00},\
        {0x8D1F, 0x00},\
        {0x8D27, 0x00},\
        {0x9004, 0x03},\
        {0x9200, 0x50},\
        {0x9201, 0x6C},\
        {0x9202, 0x71},\
        {0x9203, 0x00},\
        {0x9204, 0x71},\
        {0x9205, 0x01},\
        {0x9371, 0x6A},\
        {0x9373, 0x6A},\
        {0x9375, 0x64},\
        {0x991A, 0x00},\
        {0x996B, 0x8C},\
        {0x996C, 0x64},\
        {0x996D, 0x50},\
        {0x9A4C, 0x0D},\
        {0x9A4D, 0x0D},\
        {0xA001, 0x0A},\
        {0xA003, 0x0A},\
        {0xA005, 0x0A},\
        {0xA006, 0x01},\
        {0xA007, 0xC0},\
        {0xA009, 0xC0},

/* Global Setting Footer */
#define GLOBAL_SETTING_FOOTER \
        {0xF61C, 0x04},\
        {0xF61E, 0x04},\
        {0x4AE9, 0x18},\
        {0x4AEA, 0x08},\
        {0x4AE9, 0x21},\
        {0x4AEA, 0x80},

/* Image Quality Adjustment Setting */
#define IMAGE_QUALITY_SETTING \
        {0x3D8A, 0x01},\
        {0x4421, 0x08},\
        {0x7B3B, 0x01},\
        {0x7B4C, 0x00},\
        {0x9905, 0x00},\
        {0x9907, 0x00},\
        {0x9909, 0x00},\
        {0x990B, 0x00},\
        {0x9944, 0x3C},\
        {0x9947, 0x3C},\
        {0x994A, 0x8C},\
        {0x994B, 0x50},\
        {0x994C, 0x1B},\
        {0x994D, 0x8C},\
        {0x994E, 0x50},\
        {0x994F, 0x1B},\
        {0x9950, 0x8C},\
        {0x9951, 0x1B},\
        {0x9952, 0x0A},\
        {0x9953, 0x8C},\
        {0x9954, 0x1B},\
        {0x9955, 0x0A},\
        {0x9A13, 0x04},\
        {0x9A14, 0x04},\
        {0x9A19, 0x00},\
        {0x9A1C, 0x04},\
        {0x9A1D, 0x04},\
        {0x9A26, 0x05},\
        {0x9A27, 0x05},\
        {0x9A2C, 0x01},\
        {0x9A2D, 0x03},\
        {0x9A2F, 0x05},\
        {0x9A30, 0x05},\
        {0x9A41, 0x00},\
        {0x9A46, 0x00},\
        {0x9A47, 0x00},\
        {0x9C17, 0x35},\
        {0x9C1D, 0x31},\
        {0x9C29, 0x50},\
        {0x9C3B, 0x2F},\
        {0x9C41, 0x6B},\
        {0x9C47, 0x2D},\
        {0x9C4D, 0x40},\
        {0x9C6B, 0x00},\
        {0x9C71, 0xC8},\
        {0x9C73, 0x32},\
        {0x9C75, 0x04},\
        {0x9C7D, 0x2D},\
        {0x9C83, 0x40},\
        {0x9C94, 0x3F},\
        {0x9C95, 0x3F},\
        {0x9C96, 0x3F},\
        {0x9C97, 0x00},\
        {0x9C98, 0x00},\
        {0x9C99, 0x00},\
        {0x9C9A, 0x3F},\
        {0x9C9B, 0x3F},\
        {0x9C9C, 0x3F},\
        {0x9CA0, 0x0F},\
        {0x9CA1, 0x0F},\
        {0x9CA2, 0x0F},\
        {0x9CA3, 0x00},\
        {0x9CA4, 0x00},\
        {0x9CA5, 0x00},\
        {0x9CA6, 0x1E},\
        {0x9CA7, 0x1E},\
        {0x9CA8, 0x1E},\
        {0x9CA9, 0x00},\
        {0x9CAA, 0x00},\
        {0x9CAB, 0x00},\
        {0x9CAC, 0x09},\
        {0x9CAD, 0x09},\
        {0x9CAE, 0x09},\
        {0x9CBD, 0x50},\
        {0x9CBF, 0x50},\
        {0x9CC1, 0x50},\
        {0x9CC3, 0x40},\
        {0x9CC5, 0x40},\
        {0x9CC7, 0x40},\
        {0x9CC9, 0x0A},\
        {0x9CCB, 0x0A},\
        {0x9CCD, 0x0A},\
        {0x9D17, 0x35},\
        {0x9D1D, 0x31},\
        {0x9D29, 0x50},\
        {0x9D3B, 0x2F},\
        {0x9D41, 0x6B},\
        {0x9D47, 0x42},\
        {0x9D4D, 0x5A},\
        {0x9D6B, 0x00},\
        {0x9D71, 0xC8},\
        {0x9D73, 0x32},\
        {0x9D75, 0x04},\
        {0x9D7D, 0x42},\
        {0x9D83, 0x5A},\
        {0x9D94, 0x3F},\
        {0x9D95, 0x3F},\
        {0x9D96, 0x3F},\
        {0x9D97, 0x00},\
        {0x9D98, 0x00},\
        {0x9D99, 0x00},\
        {0x9D9A, 0x3F},\
        {0x9D9B, 0x3F},\
        {0x9D9C, 0x3F},\
        {0x9D9D, 0x1F},\
        {0x9D9E, 0x1F},\
        {0x9D9F, 0x1F},\
        {0x9DA0, 0x0F},\
        {0x9DA1, 0x0F},\
        {0x9DA2, 0x0F},\
        {0x9DA3, 0x00},\
        {0x9DA4, 0x00},\
        {0x9DA5, 0x00},\
        {0x9DA6, 0x1E},\
        {0x9DA7, 0x1E},\
        {0x9DA8, 0x1E},\
        {0x9DA9, 0x00},\
        {0x9DAA, 0x00},\
        {0x9DAB, 0x00},\
        {0x9DAC, 0x09},\
        {0x9DAD, 0x09},\
        {0x9DAE, 0x09},\
        {0x9DC9, 0x0A},\
        {0x9DCB, 0x0A},\
        {0x9DCD, 0x0A},\
        {0x9E17, 0x35},\
        {0x9E1D, 0x31},\
        {0x9E29, 0x50},\
        {0x9E3B, 0x2F},\
        {0x9E41, 0x6B},\
        {0x9E47, 0x2D},\
        {0x9E4D, 0x40},\
        {0x9E6B, 0x00},\
        {0x9E71, 0xC8},\
        {0x9E73, 0x32},\
        {0x9E75, 0x04},\
        {0x9E94, 0x0F},\
        {0x9E95, 0x0F},\
        {0x9E96, 0x0F},\
        {0x9E97, 0x00},\
        {0x9E98, 0x00},\
        {0x9E99, 0x00},\
        {0x9EA0, 0x0F},\
        {0x9EA1, 0x0F},\
        {0x9EA2, 0x0F},\
        {0x9EA3, 0x00},\
        {0x9EA4, 0x00},\
        {0x9EA5, 0x00},\
        {0x9EA6, 0x3F},\
        {0x9EA7, 0x3F},\
        {0x9EA8, 0x3F},\
        {0x9EA9, 0x00},\
        {0x9EAA, 0x00},\
        {0x9EAB, 0x00},\
        {0x9EAC, 0x09},\
        {0x9EAD, 0x09},\
        {0x9EAE, 0x09},\
        {0x9EC9, 0x0A},\
        {0x9ECB, 0x0A},\
        {0x9ECD, 0x0A},\
        {0x9F17, 0x35},\
        {0x9F1D, 0x31},\
        {0x9F29, 0x50},\
        {0x9F3B, 0x2F},\
        {0x9F41, 0x6B},\
        {0x9F47, 0x42},\
        {0x9F4D, 0x5A},\
        {0x9F6B, 0x00},\
        {0x9F71, 0xC8},\
        {0x9F73, 0x32},\
        {0x9F75, 0x04},\
        {0x9F94, 0x0F},\
        {0x9F95, 0x0F},\
        {0x9F96, 0x0F},\
        {0x9F97, 0x00},\
        {0x9F98, 0x00},\
        {0x9F99, 0x00},\
        {0x9F9A, 0x2F},\
        {0x9F9B, 0x2F},\
        {0x9F9C, 0x2F},\
        {0x9F9D, 0x00},\
        {0x9F9E, 0x00},\
        {0x9F9F, 0x00},\
        {0x9FA0, 0x0F},\
        {0x9FA1, 0x0F},\
        {0x9FA2, 0x0F},\
        {0x9FA3, 0x00},\
        {0x9FA4, 0x00},\
        {0x9FA5, 0x00},\
        {0x9FA6, 0x1E},\
        {0x9FA7, 0x1E},\
        {0x9FA8, 0x1E},\
        {0x9FA9, 0x00},\
        {0x9FAA, 0x00},\
        {0x9FAB, 0x00},\
        {0x9FAC, 0x09},\
        {0x9FAD, 0x09},\
        {0x9FAE, 0x09},\
        {0x9FC9, 0x0A},\
        {0x9FCB, 0x0A},\
        {0x9FCD, 0x0A},\
        {0xA14B, 0xFF},\
        {0xA151, 0x0C},\
        {0xA153, 0x50},\
        {0xA155, 0x02},\
        {0xA157, 0x00},\
        {0xA1AD, 0xFF},\
        {0xA1B3, 0x0C},\
        {0xA1B5, 0x50},\
        {0xA1B9, 0x00},\
        {0xA24B, 0xFF},\
        {0xA257, 0x00},\
        {0xA2AD, 0xFF},\
        {0xA2B9, 0x00},\
        {0xB21F, 0x04},\
        {0xB35C, 0x00},\
        {0xB35E, 0x08},

/* Visible Size */
#define VISIBLE_SIZE_4K3K \
        {0x0344, 0x00},\
        {0x0345, 0x00},\
        {0x0346, 0x00},\
        {0x0347, 0x00},\
        {0x0348, 0x0F},\
        {0x0349, 0xD7},\
        {0x034A, 0x0B},\
        {0x034B, 0xDF},
#define VISIBLE_SIZE_4K2K \
        {0x0344, 0x00},\
        {0x0345, 0x00},\
        {0x0346, 0x01},\
        {0x0347, 0x78},\
        {0x0348, 0x0F},\
        {0x0349, 0xD7},\
        {0x034A, 0x0A},\
        {0x034B, 0x67},
#define VISIBLE_SIZE_FHD \
        {0x0344, 0x00},\
        {0x0345, 0x00},\
        {0x0346, 0x01},\
        {0x0347, 0x78},\
        {0x0348, 0x0F},\
        {0x0349, 0xD7},\
        {0x034A, 0x0A},\
        {0x034B, 0x67},
#define VISIBLE_SIZE_VGA \
        {0x0344, 0x03},\
        {0x0345, 0xAC},\
        {0x0346, 0x02},\
        {0x0347, 0xC0},\
        {0x0348, 0x0C},\
        {0x0349, 0x2B},\
        {0x034A, 0x09},\
        {0x034B, 0x1F},

/* Mode Setting */
#define MODE_SETTING_4K3K \
        {0x0220, 0x00},\
        {0x0221, 0x11},\
        {0x0381, 0x01},\
        {0x0383, 0x01},\
        {0x0385, 0x01},\
        {0x0387, 0x01},\
        {0x0900, 0x00},\
        {0x0901, 0x11},\
        {0x0902, 0x02},\
        {0x3140, 0x02},\
        {0x3C00, 0x00},\
        {0x3C01, 0x03},\
        {0x3C02, 0xDC},\
        {0x3F0D, 0x00},\
        {0x5748, 0x07},\
        {0x5749, 0xFF},\
        {0x574A, 0x00},\
        {0x574B, 0x00},\
        {0x7B53, 0x01},\
        {0x9369, 0x5A},\
        {0x936B, 0x55},\
        {0x936D, 0x28},\
        {0x9304, 0x03},\
        {0x9305, 0x00},\
        {0x9E9A, 0x2F},\
        {0x9E9B, 0x2F},\
        {0x9E9C, 0x2F},\
        {0x9E9D, 0x00},\
        {0x9E9E, 0x00},\
        {0x9E9F, 0x00},\
        {0xA2A9, 0x60},\
        {0xA2B7, 0x00},
#define MODE_SETTING_4K2K \
        {0x0220, 0x00},\
        {0x0221, 0x11},\
        {0x0381, 0x01},\
        {0x0383, 0x01},\
        {0x0385, 0x01},\
        {0x0387, 0x01},\
        {0x0900, 0x00},\
        {0x0901, 0x11},\
        {0x0902, 0x02},\
        {0x3140, 0x02},\
        {0x3C00, 0x00},\
        {0x3C01, 0x03},\
        {0x3C02, 0xDC},\
        {0x3F0D, 0x00},\
        {0x5748, 0x07},\
        {0x5749, 0xFF},\
        {0x574A, 0x00},\
        {0x574B, 0x00},\
        {0x7B53, 0x01},\
        {0x9369, 0x5A},\
        {0x936B, 0x55},\
        {0x936D, 0x28},\
        {0x9304, 0x03},\
        {0x9305, 0x00},\
        {0x9E9A, 0x2F},\
        {0x9E9B, 0x2F},\
        {0x9E9C, 0x2F},\
        {0x9E9D, 0x00},\
        {0x9E9E, 0x00},\
        {0x9E9F, 0x00},\
        {0xA2A9, 0x60},\
        {0xA2B7, 0x00},
#define MODE_SETTING_FHD \
        {0x0220, 0x00},\
        {0x0221, 0x11},\
        {0x0381, 0x01},\
        {0x0383, 0x01},\
        {0x0385, 0x01},\
        {0x0387, 0x01},\
        {0x0900, 0x01},\
        {0x0901, 0x22},\
        {0x0902, 0x00},\
        {0x3140, 0x02},\
        {0x3C00, 0x00},\
        {0x3C01, 0x01},\
        {0x3C02, 0x9C},\
        {0x3F0D, 0x00},\
        {0x5748, 0x00},\
        {0x5749, 0x00},\
        {0x574A, 0x00},\
        {0x574B, 0xA4},\
        {0x7B53, 0x00},\
        {0x9369, 0x73},\
        {0x936B, 0x64},\
        {0x936D, 0x5F},\
        {0x9304, 0x03},\
        {0x9305, 0x80},\
        {0x9E9A, 0x2F},\
        {0x9E9B, 0x2F},\
        {0x9E9C, 0x2F},\
        {0x9E9D, 0x00},\
        {0x9E9E, 0x00},\
        {0x9E9F, 0x00},\
        {0xA2A9, 0x27},\
        {0xA2B7, 0x03},
#define MODE_SETTING_VGA \
        {0x0220, 0x00},\
        {0x0221, 0x11},\
        {0x0381, 0x01},\
        {0x0383, 0x01},\
        {0x0385, 0x01},\
        {0x0387, 0x01},\
        {0x0900, 0x01},\
        {0x0901, 0x22},\
        {0x0902, 0x00},\
        {0x3140, 0x02},\
        {0x3C00, 0x00},\
        {0x3C01, 0x01},\
        {0x3C02, 0x9C},\
        {0x3F0D, 0x00},\
        {0x5748, 0x00},\
        {0x5749, 0x00},\
        {0x574A, 0x00},\
        {0x574B, 0xA4},\
        {0x7B53, 0x00},\
        {0x9369, 0x73},\
        {0x936B, 0x64},\
        {0x936D, 0x5F},\
        {0x9304, 0x03},\
        {0x9305, 0x80},\
        {0x9E9A, 0x2F},\
        {0x9E9B, 0x2F},\
        {0x9E9C, 0x2F},\
        {0x9E9D, 0x00},\
        {0x9E9E, 0x00},\
        {0x9E9F, 0x00},\
        {0xA2A9, 0x27},\
        {0xA2B7, 0x03},

/* Digital Crop & Scaling */
#define CROP_SCALING_4K3K \
        {0x0401, 0x00},\
        {0x0404, 0x00},\
        {0x0405, 0x10},\
        {0x0408, 0x00},\
        {0x0409, 0x00},\
        {0x040A, 0x00},\
        {0x040B, 0x00},\
        {0x040C, 0x0F},\
        {0x040D, 0xD8},\
        {0x040E, 0x0B},\
        {0x040F, 0xE0},
#define CROP_SCALING_4K2K \
        {0x0401, 0x00},\
        {0x0404, 0x00},\
        {0x0405, 0x10},\
        {0x0408, 0x00},\
        {0x0409, 0x6C},\
        {0x040A, 0x00},\
        {0x040B, 0x40},\
        {0x040C, 0x0F},\
        {0x040D, 0x00},\
        {0x040E, 0x08},\
        {0x040F, 0x70},
#define CROP_SCALING_FHD \
        {0x0401, 0x00},\
        {0x0404, 0x00},\
        {0x0405, 0x10},\
        {0x0408, 0x00},\
        {0x0409, 0x36},\
        {0x040A, 0x00},\
        {0x040B, 0x20},\
        {0x040C, 0x07},\
        {0x040D, 0x80},\
        {0x040E, 0x04},\
        {0x040F, 0x38},
#define CROP_SCALING_VGA \
        {0x0401, 0x02},\
        {0x0404, 0x00},\
        {0x0405, 0x1B},\
        {0x0408, 0x00},\
        {0x0409, 0x02},\
        {0x040A, 0x00},\
        {0x040B, 0x02},\
        {0x040C, 0x04},\
        {0x040D, 0x3A},\
        {0x040E, 0x03},\
        {0x040F, 0x2C},

/* Output Crop */
#define OUTPUT_CROP_4K3K \
        {0x034C, 0x0F},\
        {0x034D, 0xD8},\
        {0x034E, 0x0B},\
        {0x034F, 0xE0},
#define OUTPUT_CROP_4K2K \
        {0x034C, 0x0F},\
        {0x034D, 0x00},\
        {0x034E, 0x08},\
        {0x034F, 0x70},
#define OUTPUT_CROP_FHD \
        {0x034C, 0x07},\
        {0x034D, 0x80},\
        {0x034E, 0x04},\
        {0x034F, 0x38},
#define OUTPUT_CROP_VGA \
        {0x034C, 0x02},\
        {0x034D, 0x80},\
        {0x034E, 0x01},\
        {0x034F, 0xE0},

static const imx378_reg imx378_start[] = {
        {0x0100, 0x01}, /* mode select streaming on */
        {IMX378_TABLE_WAIT_MS, IMX378_WAIT_MS},
        {IMX378_TABLE_END, 0x00}
};

static const imx378_reg imx378_stop[] = {
        {IMX378_TABLE_WAIT_MS, IMX378_WAIT_MS},
        {0x0100, 0x00}, /* mode select streaming off */
        {IMX378_TABLE_END, 0x00}
};

static const imx378_reg tp_colorbars[] = {
        /* test pattern */
        {IMX378_TABLE_END, 0x00}
};

static struct reg_8 mode_4056X3040[] = {
        {IMX378_TABLE_WAIT_MS, 10},
        /* software reset */
        {0x0103, 0x01},
        EXTERNAL_CLOCK
        GLOBAL_SETTING_HEADER
        IMAGE_QUALITY_SETTING
        MIPI_RAW10
        MIPI_2LANE
        /* Frame Horizontal Clock Count */
        {IMX378_LINE_LENGTH_HIGH, (((IMX378_FARAME_H_4K3K) >> 8) & 0xFF)},
        {IMX378_LINE_LENGTH_LOW,  (((IMX378_FARAME_H_4K3K)     ) & 0xFF)},
        /* Frame Vertical Clock Count */
        {IMX378_FRAME_LENGTH_HIGH, (((IMX378_FARAME_V_4K3K) >> 8) & 0xFF)},
        {IMX378_FRAME_LENGTH_LOW,  (((IMX378_FARAME_V_4K3K)     ) & 0xFF)},
        VISIBLE_SIZE_4K3K
        MODE_SETTING_4K3K
        CROP_SCALING_4K3K
        OUTPUT_CROP_4K3K
        CLOCK_SETTING_RAW10_21
        OUTPUT_DATA_RATE_21
        OUTPUT_DATA_SELECT
        /* PowerSave Setting */
        {IMX378_POWER_SAVE_MODE, 0x00},
        {IMX378_POWER_SAVE_HIGH, (((IMX378_FARAME_H_4K3K*IMX378_POWER_SAVE_RATE_21+1) >> 8) & 0xFF)},
        {IMX378_POWER_SAVE_LOW,  (((IMX378_FARAME_H_4K3K*IMX378_POWER_SAVE_RATE_21+1)     ) & 0xFF)},
        COARSE_STORAGE_TIME
        ANALOG_GAIN
        DIGITAL_GAIN
        PEDESTAL
        GLOBAL_SETTING_FOOTER
        {IMX378_TABLE_WAIT_MS, IMX378_WAIT_MS},
        {IMX378_TABLE_END, 0x00}
};

static imx378_reg mode_3840X2160[] = {
        {IMX378_TABLE_WAIT_MS, 10},
        /* software reset */
        {0x0103, 0x01},
        EXTERNAL_CLOCK
        GLOBAL_SETTING_HEADER
        IMAGE_QUALITY_SETTING
        MIPI_RAW10
        MIPI_2LANE
        /* Frame Horizontal Clock Count */
        {IMX378_LINE_LENGTH_HIGH, (((IMX378_FARAME_H_4K2K) >> 8) & 0xFF)},
        {IMX378_LINE_LENGTH_LOW,  (((IMX378_FARAME_H_4K2K)     ) & 0xFF)},
        /* Frame Vertical Clock Count */
        {IMX378_FRAME_LENGTH_HIGH, (((IMX378_FARAME_V_4K2K) >> 8) & 0xFF)},
        {IMX378_FRAME_LENGTH_LOW,  (((IMX378_FARAME_V_4K2K)     ) & 0xFF)},
        VISIBLE_SIZE_4K2K
        MODE_SETTING_4K2K
        CROP_SCALING_4K2K
        OUTPUT_CROP_4K2K
        CLOCK_SETTING_RAW10_15
        OUTPUT_DATA_RATE_15
        OUTPUT_DATA_SELECT
        /* PowerSave Setting */
        {IMX378_POWER_SAVE_MODE, 0x00},
        {IMX378_POWER_SAVE_HIGH, (((IMX378_FARAME_H_4K2K*IMX378_POWER_SAVE_RATE_15+1) >> 8) & 0xFF)},
        {IMX378_POWER_SAVE_LOW,  (((IMX378_FARAME_H_4K2K*IMX378_POWER_SAVE_RATE_15+1)     ) & 0xFF)},
        COARSE_STORAGE_TIME
        ANALOG_GAIN
        DIGITAL_GAIN
        PEDESTAL
        GLOBAL_SETTING_FOOTER
        {IMX378_TABLE_WAIT_MS, IMX378_WAIT_MS},
        {IMX378_TABLE_END, 0x00}
};

static struct reg_8 mode_1920X1080[] = {
        {IMX378_TABLE_WAIT_MS, 10},
        /* software reset */
        {0x0103, 0x01},
        EXTERNAL_CLOCK
        GLOBAL_SETTING_HEADER
        IMAGE_QUALITY_SETTING
        MIPI_RAW10
        MIPI_2LANE
        /* Frame Horizontal Clock Count */
        {IMX378_LINE_LENGTH_HIGH, (((IMX378_FARAME_H_FHD) >> 8) & 0xFF)},
        {IMX378_LINE_LENGTH_LOW,  (((IMX378_FARAME_H_FHD)     ) & 0xFF)},
        /* Frame Vertical Clock Count */
        {IMX378_FRAME_LENGTH_HIGH, (((IMX378_FARAME_V_FHD) >> 8) & 0xFF)},
        {IMX378_FRAME_LENGTH_LOW,  (((IMX378_FARAME_V_FHD)     ) & 0xFF)},
        VISIBLE_SIZE_FHD
        MODE_SETTING_FHD
        CROP_SCALING_FHD
        OUTPUT_CROP_FHD
        CLOCK_SETTING_RAW10_15
        OUTPUT_DATA_RATE_15
        OUTPUT_DATA_SELECT
        /* PowerSave Setting */
        {IMX378_POWER_SAVE_MODE, 0x00},
        {IMX378_POWER_SAVE_HIGH, (((IMX378_FARAME_H_FHD*IMX378_POWER_SAVE_RATE_15+1) >> 8) & 0xFF)},
        {IMX378_POWER_SAVE_LOW,  (((IMX378_FARAME_H_FHD*IMX378_POWER_SAVE_RATE_15+1)     ) & 0xFF)},
        COARSE_STORAGE_TIME
        ANALOG_GAIN
        DIGITAL_GAIN
        PEDESTAL
        GLOBAL_SETTING_FOOTER
        {IMX378_TABLE_WAIT_MS, IMX378_WAIT_MS},
        {IMX378_TABLE_END, 0x00}
};

static struct reg_8 mode_640X480[] = {
        {IMX378_TABLE_WAIT_MS, 10},
        /* software reset */
        {0x0103, 0x01},
        EXTERNAL_CLOCK
        GLOBAL_SETTING_HEADER
        IMAGE_QUALITY_SETTING
        MIPI_RAW10
        MIPI_2LANE
        /* Frame Horizontal Clock Count */
        {IMX378_LINE_LENGTH_HIGH, (((IMX378_FARAME_H_VGA) >> 8) & 0xFF)},
        {IMX378_LINE_LENGTH_LOW,  (((IMX378_FARAME_H_VGA)     ) & 0xFF)},
        /* Frame Vertical Clock Count */
        {IMX378_FRAME_LENGTH_HIGH, (((IMX378_FARAME_V_VGA) >> 8) & 0xFF)},
        {IMX378_FRAME_LENGTH_LOW,  (((IMX378_FARAME_V_VGA)     ) & 0xFF)},
        VISIBLE_SIZE_VGA
        MODE_SETTING_VGA
        CROP_SCALING_VGA
        OUTPUT_CROP_VGA
        CLOCK_SETTING_RAW10_15
        OUTPUT_DATA_RATE_15
        OUTPUT_DATA_SELECT
        /* PowerSave Setting */
        {IMX378_POWER_SAVE_MODE, 0x00},
        {IMX378_POWER_SAVE_HIGH, (((IMX378_FARAME_H_VGA*IMX378_POWER_SAVE_RATE_15+1) >> 8) & 0xFF)},
        {IMX378_POWER_SAVE_LOW,  (((IMX378_FARAME_H_VGA*IMX378_POWER_SAVE_RATE_15+1)     ) & 0xFF)},
        COARSE_STORAGE_TIME
        ANALOG_GAIN
        DIGITAL_GAIN
        PEDESTAL
        GLOBAL_SETTING_FOOTER
        {IMX378_TABLE_WAIT_MS, IMX378_WAIT_MS},
        {IMX378_TABLE_END, 0x00}
};

enum {
        IMX378_MODE_4056X3040 = 0,
        IMX378_MODE_3840X2160,
        IMX378_MODE_1920X1080,
        IMX378_MODE_640X480,
        IMX378_MODE_START_STREAM,
        IMX378_MODE_STOP_STREAM,
        IMX378_MODE_TEST_PATTERN,
};

static const imx378_reg *mode_table[] = {
        [IMX378_MODE_4056X3040]         = mode_4056X3040,
        [IMX378_MODE_3840X2160]         = mode_3840X2160,
        [IMX378_MODE_1920X1080]         = mode_1920X1080,
        [IMX378_MODE_640X480]           = mode_640X480,
        [IMX378_MODE_START_STREAM]      = imx378_start,
        [IMX378_MODE_STOP_STREAM]       = imx378_stop,
        [IMX378_MODE_TEST_PATTERN]      = tp_colorbars,
};

static const int imx378_30_fr[] = {
        30,
};

static const int imx378_60_fr[] = {
        60,
};

static const int imx378_120_fr[] = {
        120,
};

static const int imx378_240_fr[] = {
        240,
};

static const int imx378_480_fr[] = {
        480,
};

static const struct camera_common_frmfmt imx378_frmfmt[] = {
        {{4056, 3040}, imx378_30_fr,  1, 0, IMX378_MODE_4056X3040},
        {{3840, 2160}, imx378_30_fr,  1, 0, IMX378_MODE_3840X2160},
        {{1920, 1080}, imx378_120_fr, 1, 0, IMX378_MODE_1920X1080},
        {{ 640,  480}, imx378_240_fr, 1, 0, IMX378_MODE_640X480},
};
#endif  /* __IMX378_MODE_TABLES__ */
