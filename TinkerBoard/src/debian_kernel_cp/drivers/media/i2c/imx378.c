/*
 * Driver for IMX378 CMOS Image Sensor from Sony
 *
 * Copyright (C) 2014, Andrew Chew <achew@nvidia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_graph.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-image-sizes.h>
#include <media/v4l2-mediabus.h>

/* IMX378 supported geometry */
#define IMX378_TABLE_END		0xffff
#define IMX378_ANALOGUE_GAIN_MULTIPLIER	256
#define IMX378_ANALOGUE_GAIN_MIN	(1 * IMX378_ANALOGUE_GAIN_MULTIPLIER)
#define IMX378_ANALOGUE_GAIN_MAX	(22 * IMX378_ANALOGUE_GAIN_MULTIPLIER)
#define IMX378_ANALOGUE_GAIN_DEFAULT	(16 * IMX378_ANALOGUE_GAIN_MULTIPLIER)

/* In dB*256 */
#define IMX378_DIGITAL_GAIN_MIN		256
#define IMX378_DIGITAL_GAIN_MAX		65535
#define IMX378_DIGITAL_GAIN_DEFAULT	256

#define IMX378_DIGITAL_EXPOSURE_MIN	0
#define IMX378_DIGITAL_EXPOSURE_MAX	65515
#define IMX378_DIGITAL_EXPOSURE_DEFAULT	800

#define IMX378_EXP_LINES_MARGIN		20

#define IMX378_WHITE_BALANCE_R		213
#define IMX378_WHITE_BALANCE_GR		100
#define IMX378_WHITE_BALANCE_B		193
#define IMX378_WHITE_BALANCE_GB		100

static const s64 link_freq_menu_items[] = {
	999000000,
};

struct imx378_reg {
	u16 addr;
	u8 val;
};

struct imx378_mode {
	u32 width;
	u32 height;
	u32 max_fps;
	u32 hts_def;
	u32 vts_def;
	const struct imx378_reg *reg_list;
	u8 binning_v; // 0 = no binning, 1 = 2x binning
	u8 binning_h;
};

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

/* MCLK:18MHz  4056x3040  15fps   MIPI LANE2 */
/* static const struct imx378_reg imx378_init_tab_4056_3040_15fps[] = { */
/* MCLK:18MHz  4032x3040  15fps   MIPI LANE2 */
static const struct imx378_reg imx378_init_tab_4032_3040_15fps[] = {
	{0x0136, 0x12},    // EXCK_FREQ[15:8]
	{0x0137, 0x00},    // EXCK_FREQ[7:0]
	/* Global Setting Header */
	GLOBAL_SETTING_HEADER
	/* Image Quality Adjustment Setting */
	IMAGE_QUALITY_SETTING
	/* MIPI setting */
	{0x0112, 0x0A},    // CSI_DT_FMT_H
	{0x0113, 0x0A},    // CSI_DT_FMT_L
	{0x0114, 0x01},    // CSI_LANE_MODE
	/* Frame Horizontal Clock Count */
	{0x0342, 0x44},    // LINE_LENGTH_PCK[15:8]
	{0x0343, 0x5C},    // LINE_LENGTH_PCK[7:0]
	/* Frame Vertical Clock Count */
	{0x0340, 0x0C},    // FRM_LENGTH_LINES[15:8]
	{0x0341, 0x80},    // FRM_LENGTH_LINES[7:0]
	/* Visible Size */
	{0x0344, 0x00},    // X_ADD_STA[12:8]
	{0x0345, 0x0c},    // X_ADD_STA[7:0] 0 original
	{0x0346, 0x00},    // Y_ADD_STA[12:8]
	{0x0347, 0x00},    // Y_ADD_STA[7:0]
	{0x0348, 0x0F},    // X_ADD_END[12:8]
	{0x0349, 0xD7},    // X_ADD_END[7:0] D7 original 4055
	{0x034A, 0x0B},    // Y_ADD_END[12:8]
	{0x034B, 0xDF},    // Y_ADD_END[7:0]
	/* Mode Setting */
	{0x0220, 0x00},    // HDR_MODE
	{0x0221, 0x11},    // HDR_RESO_REDU_V
	{0x0381, 0x01},    // X_EVN_INC
	{0x0383, 0x01},    // X_ODD_INC
	{0x0385, 0x01},    // Y_EVN_INC
	{0x0387, 0x01},    // Y_ODD_INC
	{0x0900, 0x00},    // BINNING_MODE
	{0x0901, 0x11},    // BINNING_TYPE_H
	{0x0902, 0x02},    // BINNING_WEIGHTING
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x03},
	{0x3C02, 0xDC},
	{0x3C03, 0x00},
	{0x3C04, 0x00},
	{0x3F0D, 0x00},
	{0x5748, 0x07},
	{0x5749, 0xFF},
	{0x574A, 0x00},
	{0x574B, 0x00},
	{0x7B53, 0x01},
	{0x9369, 0x5A},
	{0x936B, 0x55},
	{0x936D, 0x28},
	{0x9304, 0x03},
	{0x9305, 0x00},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x60},
	{0xA2B7, 0x00},
	/* Digital Crop & Scaling */
	{0x0401, 0x00},    // SCALE_MODE
	{0x0404, 0x00},    // SCALE_M[8]
	{0x0405, 0x10},    // SCALE_M[7:0]
	{0x0408, 0x00},    // DIG_CROP_X_OFFSET[12:8]
	{0x0409, 0x00},    // DIG_CROP_X_OFFSET[7:0]
	{0x040A, 0x00},    // DIG_CROP_Y_OFFSET[12:8]
	{0x040B, 0x00},    // DIG_CROP_Y_OFFSET[7:0]
	{0x040C, 0x0F},    // DIG_CROP_IMAGE_WIDTH[12:8]
	{0x040D, 0xD8},    // DIG_CROP_IMAGE_WIDTH[7:0]
	{0x040E, 0x0B},    // DIG_CROP_IMAGE_HEIGHT[12:8]
	{0x040F, 0xE0},    // DIG_CROP_IMAGE_HEIGHT[7:0]
	/* Output Crop */
	{0x034C, 0x0F},    // X_OUT_SIZE[12:8]
	{0x034D, 0xc0},    // X_OUT_SIZE[7:0] D8 original 4056
	{0x034E, 0x0B},    // Y_OUT_SIZE[12:8]
	{0x034F, 0xE0},    // Y_OUT_SIZE[7:0]
	/* Clock Setting */
	{0x0301, 0x05},    // IVT_PXCK_DIV
	{0x0303, 0x02},    // IVT_SYCK_DIV
	{0x0305, 0x03},    // IVT_PREPLLCK_DIV
	{0x0306, 0x01},    // IVT_PLL_MPY[10:8]
	{0x0307, 0x5E},    // IVT_PLL_MPY[7:0]
	{0x0309, 0x0A},    // IOP_PXCK_DIV
	{0x030B, 0x02},    // IOP_SYCK_DIV
	{0x030D, 0x03},    // IOP_PREPLLCK_DIV
	{0x030E, 0x01},    // IOP_PLL_MPY[10:8]
	{0x030F, 0x4D},    // IOP_PLL_MPY[7:0]
	{0x0310, 0x01},    // PLL_MULT_DRIV
	{0x0820, 0x07},    // REQ_LINK_BIT_RATE_MBPS[31:24]
	{0x0821, 0xCE},    // REQ_LINK_BIT_RATE_MBPS[23:16]
	{0x0822, 0x00},    // REQ_LINK_BIT_RATE_MBPS[15:8]
	{0x0823, 0x00},    // REQ_LINK_BIT_RATE_MBPS[7:0]
	/* Output Data Select Setting */
	{0x3E20, 0x01},
	{0x3E37, 0x01},    // PDAF_CTRL1[7:0]
	{0xBCF1, 0x00},    // EBD_SIZE_V[0]
	/* PowerSave Setting */
	{0x3F50, 0x00},    // POWER_SAVE_ENABLE
	{0x3F56, 0x01},    // LINE_LENGTH_INCK[15:8]
	{0x3F57, 0x64},    // LINE_LENGTH_INCK[7:0]
	/* Coarse Storage Time (Exposure) */
	{0x0202, 0x03},    // COARSE_INTEG_TIME[15:8]
	{0x0203, 0xE8},    // COARSE_INTEG_TIME[7:0]
	/* Analog Gain */
	{0x0204, 0x00},    // ANA_GAIN_GLOBAL[9:8]
	{0x0205, 0x00},    // ANA_GAIN_GLOBAL[7:0]
	/* Digital Gain (White Balance) */
	{0x3FF9, 0x00},    // DPGA_USE_GLOBAL_GAIN
	{0x020E, 0x01},    // DIG_GAIN_GR[15:8]
	{0x020F, 0x00},    // DIG_GAIN_GR[7:0]
	{0x0210, 0x02},    // DIG_GAIN_R[15:8]
	{0x0211, 0x21},    // DIG_GAIN_R[7:0]
	{0x0212, 0x01},    // DIG_GAIN_B[15:8]
	{0x0213, 0xEE},    // DIG_GAIN_B[7:0]
	{0x0214, 0x01},    // DIG_GAIN_GB[15:8]
	{0x0215, 0x00},    // DIG_GAIN_GB[7:0]
	/* Pedestal (Black Level) */
	{0x3030, 0x01},    // MANUAL_DATA_PEDESTAL_EN
	{0x3032, 0x00},    // MANUAL_DATA_PEDESTAL_VALUE[11:8]
	{0x3033, 0x40},    // MANUAL_DATA_PEDESTAL_VALUE[7:0]
	/* Global Setting Footer */
	GLOBAL_SETTING_FOOTER
	{IMX378_TABLE_END, 0x00}
};

/* MCLK:18MHz  3840x2160  15fps   MIPI LANE2 */
static const struct imx378_reg imx378_init_tab_3840_2160_20fps[] = {
	{0x0136, 0x12},    // EXCK_FREQ[15:8]
	{0x0137, 0x00},    // EXCK_FREQ[7:0]
	/* Global Setting Header */
	GLOBAL_SETTING_HEADER
	/* Image Quality Adjustment Setting */
	IMAGE_QUALITY_SETTING
	/* MIPI setting */
	{0x0112, 0x0A},    // CSI_DT_FMT_H
	{0x0113, 0x0A},    // CSI_DT_FMT_L
	{0x0114, 0x01},    // CSI_LANE_MODE
	/* Frame Horizontal Clock Count */
	{0x0342, 0x44},    // LINE_LENGTH_PCK[15:8]
	{0x0343, 0x5C},    // LINE_LENGTH_PCK[7:0]
	/* Frame Vertical Clock Count */
	{0x0340, 0x09},    // FRM_LENGTH_LINES[15:8]
	{0x0341, 0x60},    // FRM_LENGTH_LINES[7:0]
	/* Visible Size */ 
	{0x0344, 0x00},    // X_ADD_STA[12:8] 
	{0x0345, 0x00},    // X_ADD_STA[7:0] 
	{0x0346, 0x01},    // Y_ADD_STA[12:8] 
	{0x0347, 0x78},    // Y_ADD_STA[7:0] 
	{0x0348, 0x0F},    // X_ADD_END[12:8] 
	{0x0349, 0xD7},    // X_ADD_END[7:0] 
	{0x034A, 0x0A},    // Y_ADD_END[12:8] 
	{0x034B, 0x67},    // Y_ADD_END[7:0] 
	/* Mode Setting */
	{0x0220, 0x00},    // HDR_MODE
	{0x0221, 0x11},    // HDR_RESO_REDU_V
	{0x0381, 0x01},    // X_EVN_INC
	{0x0383, 0x01},    // X_ODD_INC
	{0x0385, 0x01},    // Y_EVN_INC
	{0x0387, 0x01},    // Y_ODD_INC
	{0x0900, 0x00},    // BINNING_MODE
	{0x0901, 0x11},    // BINNING_TYPE_H
	{0x0902, 0x02},    // BINNING_WEIGHTING
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x03},
	{0x3C02, 0xDC},
	{0x3C03, 0x00},
	{0x3C04, 0x00},
	{0x3F0D, 0x00},
	{0x5748, 0x07},
	{0x5749, 0xFF},
	{0x574A, 0x00},
	{0x574B, 0x00},
	{0x7B53, 0x01},
	{0x9369, 0x5A},
	{0x936B, 0x55},
	{0x936D, 0x28},
	{0x9304, 0x03},
	{0x9305, 0x00},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x60},
	{0xA2B7, 0x00},
	/* Digital Crop & Scaling */
	{0x0401, 0x00},    // SCALE_MODE
	{0x0404, 0x00},    // SCALE_M[8]
	{0x0405, 0x10},    // SCALE_M[7:0]
	{0x0408, 0x00},    // DIG_CROP_X_OFFSET[12:8]
	{0x0409, 0x00},    // DIG_CROP_X_OFFSET[7:0]
	{0x040A, 0x00},    // DIG_CROP_Y_OFFSET[12:8]
	{0x040B, 0x00},    // DIG_CROP_Y_OFFSET[7:0]
	{0x040C, 0x0F},    // DIG_CROP_IMAGE_WIDTH[12:8]
	{0x040D, 0x00},    // DIG_CROP_IMAGE_WIDTH[7:0]
	{0x040E, 0x08},    // DIG_CROP_IMAGE_HEIGHT[12:8]
	{0x040F, 0x70},    // DIG_CROP_IMAGE_HEIGHT[7:0]
	/* Output Crop */
	{0x034C, 0x0F},    // X_OUT_SIZE[12:8]
	{0x034D, 0x00},    // X_OUT_SIZE[7:0]
	{0x034E, 0x08},    // Y_OUT_SIZE[12:8]
	{0x034F, 0x70},    // Y_OUT_SIZE[7:0]
	/* Clock Setting */
	{0x0301, 0x05},    // IVT_PXCK_DIV
	{0x0303, 0x02},    // IVT_SYCK_DIV
	{0x0305, 0x03},    // IVT_PREPLLCK_DIV
	{0x0306, 0x01},    // IVT_PLL_MPY[10:8]
	{0x0307, 0x5E},    // IVT_PLL_MPY[7:0]
	{0x0309, 0x0A},    // IOP_PXCK_DIV
	{0x030B, 0x02},    // IOP_SYCK_DIV
	{0x030D, 0x03},    // IOP_PREPLLCK_DIV
	{0x030E, 0x01},    // IOP_PLL_MPY[10:8]
	{0x030F, 0x4D},    // IOP_PLL_MPY[7:0]
	{0x0310, 0x01},    // PLL_MULT_DRIV
	{0x0820, 0x07},    // REQ_LINK_BIT_RATE_MBPS[31:24]
	{0x0821, 0xCE},    // REQ_LINK_BIT_RATE_MBPS[23:16]
	{0x0822, 0x00},    // REQ_LINK_BIT_RATE_MBPS[15:8]
	{0x0823, 0x00},    // REQ_LINK_BIT_RATE_MBPS[7:0]
	/* Output Data Select Setting */
	{0x3E20, 0x01},
	{0x3E37, 0x01},    // PDAF_CTRL1[7:0]
	{0xBCF1, 0x00},    // EBD_SIZE_V[0]
	/* PowerSave Setting */
	{0x3F50, 0x00},    // POWER_SAVE_ENABLE
	{0x3F56, 0x01},    // LINE_LENGTH_INCK[15:8]
	{0x3F57, 0x64},    // LINE_LENGTH_INCK[7:0]
	/* Coarse Storage Time (Exposure) */
	{0x0202, 0x03},    // COARSE_INTEG_TIME[15:8]
	{0x0203, 0xE8},    // COARSE_INTEG_TIME[7:0]
	/* Analog Gain */
	{0x0204, 0x00},    // ANA_GAIN_GLOBAL[9:8]
	{0x0205, 0x00},    // ANA_GAIN_GLOBAL[7:0]
	/* Digital Gain (White Balance) */
	{0x3FF9, 0x00},    // DPGA_USE_GLOBAL_GAIN
	{0x020E, 0x01},    // DIG_GAIN_GR[15:8]
	{0x020F, 0x00},    // DIG_GAIN_GR[7:0]
	{0x0210, 0x02},    // DIG_GAIN_R[15:8]
	{0x0211, 0x21},    // DIG_GAIN_R[7:0]
	{0x0212, 0x01},    // DIG_GAIN_B[15:8]
	{0x0213, 0xEE},    // DIG_GAIN_B[7:0]
	{0x0214, 0x01},    // DIG_GAIN_GB[15:8]
	{0x0215, 0x00},    // DIG_GAIN_GB[7:0]
	/* Pedestal (Black Level) */
	{0x3030, 0x01},    // MANUAL_DATA_PEDESTAL_EN
	{0x3032, 0x00},    // MANUAL_DATA_PEDESTAL_VALUE[11:8]
	{0x3033, 0x40},    // MANUAL_DATA_PEDESTAL_VALUE[7:0]
	/* Global Setting Footer */
	GLOBAL_SETTING_FOOTER
	{IMX378_TABLE_END, 0x00}
};

/* MCLK:18MHz  1920x1080 2x2 binning 30fps   MIPI LANE2 */
static const struct imx378_reg imx378_init_tab_1920_1080_80fps[] = {
	{0x0136, 0x12},    // EXCK_FREQ[15:8]
	{0x0137, 0x00},    // EXCK_FREQ[7:0]
	/* Global Setting Header */
	GLOBAL_SETTING_HEADER
	/* Image Quality Adjustment Setting */
	IMAGE_QUALITY_SETTING
	/* MIPI setting */
	{0x0112, 0x0A},    // CSI_DT_FMT_H
	{0x0113, 0x0A},    // CSI_DT_FMT_L
	{0x0114, 0x01},    // CSI_LANE_MODE
	/* Frame Horizontal Clock Count */
	{0x0342, 0x24},    // LINE_LENGTH_PCK[15:8]
	{0x0343, 0x6D},    // LINE_LENGTH_PCK[7:0]
	/* Frame Vertical Clock Count */
	{0x0340, 0x04},    // FRM_LENGTH_LINES[15:8]
	{0x0341, 0x66},    // FRM_LENGTH_LINES[7:0]
	/* Visible Size */
	{0x0344, 0x00},    // X_ADD_STA[12:8]
	{0x0345, 0x6C},    // X_ADD_STA[7:0]
	{0x0346, 0x01},    // Y_ADD_STA[12:8]
	{0x0347, 0xB8},    // Y_ADD_STA[7:0]
	{0x0348, 0x0F},    // X_ADD_END[12:8]
	{0x0349, 0x6B},    // X_ADD_END[7:0]
	{0x034A, 0x0A},    // Y_ADD_END[12:8]
	{0x034B, 0x27},    // Y_ADD_END[7:0]
	/* Mode Setting */
	{0x0220, 0x00},    // HDR_MODE
	{0x0221, 0x11},    // HDR_RESO_REDU_V
	{0x0381, 0x01},    // X_EVN_INC
	{0x0383, 0x01},    // X_ODD_INC
	{0x0385, 0x01},    // Y_EVN_INC
	{0x0387, 0x01},    // Y_ODD_INC
	{0x0900, 0x01},    // BINNING_MODE
	{0x0901, 0x22},    // BINNING_TYPE_H
	{0x0902, 0x02},    // BINNING_WEIGHTING
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x03},
	{0x3C02, 0xDC},
	{0x3C03, 0x00},
	{0x3C04, 0x00},
	{0x3F0D, 0x00},
	{0x5748, 0x07},
	{0x5749, 0xFF},
	{0x574A, 0x00},
	{0x574B, 0x00},
	{0x7B53, 0x01},
	{0x9369, 0x5A},
	{0x936B, 0x55},
	{0x936D, 0x28},
	{0x9304, 0x03},
	{0x9305, 0x00},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x60},
	{0xA2B7, 0x00},
	/* Digital Crop & Scaling */
	{0x0401, 0x00},    // SCALE_MODE
	{0x0404, 0x00},    // SCALE_M[8]
	{0x0405, 0x10},    // SCALE_M[7:0]
	{0x0408, 0x00},    // DIG_CROP_X_OFFSET[12:8]
	{0x0409, 0x00},    // DIG_CROP_X_OFFSET[7:0]
	{0x040A, 0x00},    // DIG_CROP_Y_OFFSET[12:8]
	{0x040B, 0x00},    // DIG_CROP_Y_OFFSET[7:0]
	{0x040C, 0x07},    // DIG_CROP_IMAGE_WIDTH[12:8]
	{0x040D, 0x80},    // DIG_CROP_IMAGE_WIDTH[7:0]
	{0x040E, 0x04},    // DIG_CROP_IMAGE_HEIGHT[12:8]
	{0x040F, 0x38},    // DIG_CROP_IMAGE_HEIGHT[7:0]
	/* Output Crop */
	{0x034C, 0x07},    // X_OUT_SIZE[12:8]
	{0x034D, 0x80},    // X_OUT_SIZE[7:0]
	{0x034E, 0x04},    // Y_OUT_SIZE[12:8]
	{0x034F, 0x38},    // Y_OUT_SIZE[7:0]
	/* Clock Setting */
	{0x0301, 0x05},    // IVT_PXCK_DIV
	{0x0303, 0x02},    // IVT_SYCK_DIV
	{0x0305, 0x03},    // IVT_PREPLLCK_DIV
	{0x0306, 0x01},    // IVT_PLL_MPY[10:8]
	{0x0307, 0x5E},    // IVT_PLL_MPY[7:0]
	{0x0309, 0x0A},    // IOP_PXCK_DIV
	{0x030B, 0x02},    // IOP_SYCK_DIV
	{0x030D, 0x03},    // IOP_PREPLLCK_DIV
	{0x030E, 0x01},    // IOP_PLL_MPY[10:8]
	{0x030F, 0x4D},    // IOP_PLL_MPY[7:0]
	{0x0310, 0x01},    // PLL_MULT_DRIV
	{0x0820, 0x07},    // REQ_LINK_BIT_RATE_MBPS[31:24]
	{0x0821, 0xCE},    // REQ_LINK_BIT_RATE_MBPS[23:16]
	{0x0822, 0x00},    // REQ_LINK_BIT_RATE_MBPS[15:8]
	{0x0823, 0x00},    // REQ_LINK_BIT_RATE_MBPS[7:0]
	/* Output Data Select Setting */
	{0x3E20, 0x01},
	{0x3E37, 0x01},    // PDAF_CTRL1[7:0]
	{0xBCF1, 0x00},    // EBD_SIZE_V[0]
	/* PowerSave Setting */
	{0x3F50, 0x00},    // POWER_SAVE_ENABLE
	{0x3F56, 0x00},    // LINE_LENGTH_INCK[15:8]
	{0x3F57, 0xB7},    // LINE_LENGTH_INCK[7:0]
	/* Coarse Storage Time (Exposure) */
	{0x0202, 0x03},    // COARSE_INTEG_TIME[15:8]
	{0x0203, 0xE8},    // COARSE_INTEG_TIME[7:0]
	/* Analog Gain */
	{0x0204, 0x03},    // ANA_GAIN_GLOBAL[9:8]
	{0x0205, 0x00},    // ANA_GAIN_GLOBAL[7:0]
	/* Digital Gain (White Balance) */
	{0x3FF9, 0x00},    // DPGA_USE_GLOBAL_GAIN
	{0x020E, 0x01},    // DIG_GAIN_GR[15:8]
	{0x020F, 0x00},    // DIG_GAIN_GR[7:0]
	{0x0210, 0x02},    // DIG_GAIN_R[15:8]
	{0x0211, 0x21},    // DIG_GAIN_R[7:0]
	{0x0212, 0x01},    // DIG_GAIN_B[15:8]
	{0x0213, 0xEE},    // DIG_GAIN_B[7:0]
	{0x0214, 0x01},    // DIG_GAIN_GB[15:8]
	{0x0215, 0x00},    // DIG_GAIN_GB[7:0]
	/* Pedestal (Black Level) */
	{0x3030, 0x01},    // MANUAL_DATA_PEDESTAL_EN
	{0x3032, 0x00},    // MANUAL_DATA_PEDESTAL_VALUE[11:8]
	{0x3033, 0x40},    // MANUAL_DATA_PEDESTAL_VALUE[7:0]
	/* Global Setting Footer */
	GLOBAL_SETTING_FOOTER
	{IMX378_TABLE_END, 0x00}
};

/* MCLK:18MHz  1024x768 2x2 binning 120fps   MIPI LANE2 */
static const struct imx378_reg imx378_init_tab_1024_768_210fps[] = {
	{0x0136, 0x12},     // EXCK_FREQ[15:8]
	{0x0137, 0x00},     // EXCK_FREQ[7:0]
	/* Global Setting Header */
	GLOBAL_SETTING_HEADER
	/* Image Quality Adjustment Setting */
	IMAGE_QUALITY_SETTING
	/* MIPI setting */
	{0x0112, 0x0A},     // CSI_DT_FMT_H
	{0x0113, 0x0A},     // CSI_DT_FMT_L
	{0x0114, 0x01},     // CSI_LANE_MODE
	/* Frame Horizontal Clock Count */
	{0x0342, 0x13},     // LINE_LENGTH_PCK[15:8]
	{0x0343, 0x0E},     // LINE_LENGTH_PCK[7:0]
	/* Frame Vertical Clock Count */
	//{0x0340, 0x04},     // FRM_LENGTH_LINES[15:8]
	//{0x0341, 0x98},     // FRM_LENGTH_LINES[7:0]
	{0x0340, 0x03},     // FRM_LENGTH_LINES[15:8]
	{0x0341, 0x34},     // FRM_LENGTH_LINES[7:0]
	/* Visible Size */
	{0x0344, 0x03},     // X_ADD_STA[12:8]
	{0x0345, 0xEC},     // X_ADD_STA[7:0]
	{0x0346, 0x02},     // Y_ADD_STA[12:8]
	{0x0347, 0xF0},     // Y_ADD_STA[7:0]
	{0x0348, 0x0B},     // X_ADD_END[12:8]
	{0x0349, 0xEB},     // X_ADD_END[7:0]
	{0x034A, 0x08},     // Y_ADD_END[12:8]
	{0x034B, 0xEF},     // Y_ADD_END[7:0]
	/* Mode Setting */
	{0x0220, 0x00},     // HDR_MODE
	{0x0221, 0x11},     // HDR_RESO_REDU_V
	{0x0381, 0x01},     // X_EVN_INC
	{0x0383, 0x01},     // X_ODD_INC
	{0x0385, 0x01},     // Y_EVN_INC
	{0x0387, 0x01},     // Y_ODD_INC
	{0x0900, 0x01},     // BINNING_MODE
	{0x0901, 0x22},     // BINNING_TYPE_H
	{0x0902, 0x02},     // BINNING_WEIGHTING
	{0x3140, 0x02},
	{0x3C00, 0x00},
	{0x3C01, 0x03},
	{0x3C02, 0xDC},
	{0x3C03, 0x00},
	{0x3C04, 0x00},
	{0x3F0D, 0x00},
	{0x5748, 0x07},
	{0x5749, 0xFF},
	{0x574A, 0x00},
	{0x574B, 0x00},
	{0x7B53, 0x01},
	{0x9369, 0x5A},
	{0x936B, 0x55},
	{0x936D, 0x28},
	{0x9304, 0x03},
	{0x9305, 0x00},
	{0x9E9A, 0x2F},
	{0x9E9B, 0x2F},
	{0x9E9C, 0x2F},
	{0x9E9D, 0x00},
	{0x9E9E, 0x00},
	{0x9E9F, 0x00},
	{0xA2A9, 0x60},
	{0xA2B7, 0x00},
	/* Digital Crop & Scaling */
	{0x0401, 0x00},     // SCALE_MODE
	{0x0404, 0x00},     // SCALE_M[8]
	{0x0405, 0x10},     // SCALE_M[7:0]
	{0x0408, 0x00},     // DIG_CROP_X_OFFSET[12:8]
	{0x0409, 0x00},     // DIG_CROP_X_OFFSET[7:0]
	{0x040A, 0x00},     // DIG_CROP_Y_OFFSET[12:8]
	{0x040B, 0x00},     // DIG_CROP_Y_OFFSET[7:0]
	{0x040C, 0x04},     // DIG_CROP_IMAGE_WIDTH[12:8]
	{0x040D, 0x00},     // DIG_CROP_IMAGE_WIDTH[7:0]
	{0x040E, 0x03},     // DIG_CROP_IMAGE_HEIGHT[12:8]
	{0x040F, 0x00},     // DIG_CROP_IMAGE_HEIGHT[7:0]
	/* Output Crop */
	{0x034C, 0x04},     // X_OUT_SIZE[12:8]
	{0x034D, 0x00},     // X_OUT_SIZE[7:0]
	{0x034E, 0x03},     // Y_OUT_SIZE[12:8]
	{0x034F, 0x00},     // Y_OUT_SIZE[7:0]
	/* Clock Setting */
	{0x0301, 0x05},     // IVT_PXCK_DIV
	{0x0303, 0x02},     // IVT_SYCK_DIV
	{0x0305, 0x03},     // IVT_PREPLLCK_DIV
	{0x0306, 0x01},     // IVT_PLL_MPY[10:8]
	{0x0307, 0x5E},     // IVT_PLL_MPY[7:0]
	{0x0309, 0x0A},     // IOP_PXCK_DIV
	{0x030B, 0x02},     // IOP_SYCK_DIV
	{0x030D, 0x03},     // IOP_PREPLLCK_DIV
	{0x030E, 0x01},     // IOP_PLL_MPY[10:8]
	{0x030F, 0x4D},     // IOP_PLL_MPY[7:0]
	{0x0310, 0x01},     // PLL_MULT_DRIV
	{0x0820, 0x07},     // REQ_LINK_BIT_RATE_MBPS[31:24]
	{0x0821, 0xCE},     // REQ_LINK_BIT_RATE_MBPS[23:16]
	{0x0822, 0x00},     // REQ_LINK_BIT_RATE_MBPS[15:8]
	{0x0823, 0x00},     // REQ_LINK_BIT_RATE_MBPS[7:0]
	/* Output Data Select Setting */
	{0x3E20, 0x01},
	{0x3E37, 0x01},     // PDAF_CTRL1[7:0]
	/* PowerSave Setting */
	{0x3F50, 0x00},     // POWER_SAVE_ENABLE
	{0x3F56, 0x00},     // LINE_LENGTH_INCK[15:8]
	{0x3F57, 0x40},     // LINE_LENGTH_INCK[7:0]
	/* Coarse Storage Time (Exposure) */
	{0x0202, 0x03},     // COARSE_INTEG_TIME[15:8]
	{0x0203, 0xE8},     // COARSE_INTEG_TIME[7:0]
	/* Analog Gain */
	{0x0204, 0x03},     // ANA_GAIN_GLOBAL[9:8]
	{0x0205, 0x00},     // ANA_GAIN_GLOBAL[7:0]
	/* Digital Gain (White Balance) */
	{0x3FF9, 0x00},     // DPGA_USE_GLOBAL_GAIN
	{0x020E, 0x01},     // DIG_GAIN_GR[15:8]
	{0x020F, 0x00},     // DIG_GAIN_GR[7:0]
	{0x0210, 0x02},     // DIG_GAIN_R[15:8]
	{0x0211, 0x21},     // DIG_GAIN_R[7:0]
	{0x0212, 0x01},     // DIG_GAIN_B[15:8]
	{0x0213, 0xEE},     // DIG_GAIN_B[7:0]
	{0x0214, 0x01},     // DIG_GAIN_GB[15:8]
	{0x0215, 0x00},     // DIG_GAIN_GB[7:0]
	/* Pedestal (Black Level) */
	{0x3030, 0x01},     // MANUAL_DATA_PEDESTAL_EN
	{0x3032, 0x00},     // MANUAL_DATA_PEDESTAL_VALUE[11:8]
	{0x3033, 0x40},     // MANUAL_DATA_PEDESTAL_VALUE[7:0]
	/* Global Setting Footer */
	GLOBAL_SETTING_FOOTER
	{IMX378_TABLE_END, 0x00}
};

static const struct imx378_reg start[] = {
	{0x0100, 0x01},		/* mode select streaming on */
	{IMX378_TABLE_END, 0x00}
};

static const struct imx378_reg stop[] = {
	{0x0100, 0x00},		/* mode select streaming off */
	{IMX378_TABLE_END, 0x00}
};

enum {
	TEST_PATTERN_DISABLED,
	TEST_PATTERN_SOLID_BLACK,
	TEST_PATTERN_SOLID_WHITE,
	TEST_PATTERN_SOLID_RED,
	TEST_PATTERN_SOLID_GREEN,
	TEST_PATTERN_SOLID_BLUE,
	TEST_PATTERN_COLOR_BAR,
	TEST_PATTERN_FADE_TO_GREY_COLOR_BAR,
	TEST_PATTERN_PN9,
	TEST_PATTERN_16_SPLIT_COLOR_BAR,
	TEST_PATTERN_16_SPLIT_INVERTED_COLOR_BAR,
	TEST_PATTERN_COLUMN_COUNTER,
	TEST_PATTERN_INVERTED_COLUMN_COUNTER,
	TEST_PATTERN_PN31,
	TEST_PATTERN_MAX
};

static const char *const tp_qmenu[] = {
	"Disabled",
	"Solid Black",
	"Solid White",
	"Solid Red",
	"Solid Green",
	"Solid Blue",
	"Color Bar",
	"Fade to Grey Color Bar",
	"PN9",
	"16 Split Color Bar",
	"16 Split Inverted Color Bar",
	"Column Counter",
	"Inverted Column Counter",
	"PN31",
};

#define SIZEOF_I2C_TRANSBUF 32

struct imx378 {
	struct v4l2_subdev subdev;
	struct media_pad pad;
	struct v4l2_ctrl_handler ctrl_handler;
	struct clk *clk;
	struct v4l2_rect crop_rect;
	int hflip;
	int vflip;
	u16 analogue_gain;
	u16 digital_gain_r;	/* bits 11:0 */
	u16 digital_gain_gr;	/* bits 11:0 */
	u16 digital_gain_b;	/* bits 11:0 */
	u16 digital_gain_gb;	/* bits 11:0 */
	u16 exposure_time;
	u16 test_pattern;
	u16 test_pattern_solid_color_r;
	u16 test_pattern_solid_color_gr;
	u16 test_pattern_solid_color_b;
	u16 test_pattern_solid_color_gb;
	struct v4l2_ctrl *hblank;
	struct v4l2_ctrl *vblank;
	struct v4l2_ctrl *pixel_rate;
	const struct imx378_mode *cur_mode;
	u16 cur_vts;
};

static const struct imx378_mode supported_modes[] = {
	{
		.width = 1920,
		.height = 1080,
		.max_fps = 80,
		.hts_def = 2218,
		.vts_def = 1126,
		.reg_list = imx378_init_tab_1920_1080_80fps,
		.binning_h = 0,
		.binning_v = 0,
	},
	{
		.width = 4032,
		.height = 3040,
		.max_fps = 15,
		.hts_def = 4162,
		.vts_def = 3200,
		.reg_list = imx378_init_tab_4032_3040_15fps,
		.binning_h = 0,
		.binning_v = 0,
	},
	{
		.width = 3840,
		.height = 2160,
		.max_fps = 20,
		.hts_def = 4162,
		.vts_def = 2400,
		.reg_list = imx378_init_tab_3840_2160_20fps,
		.binning_h = 0,
		.binning_v = 0,
	},
	{
		.width = 1024,
		.height = 768,
		.max_fps = 210,
		.hts_def = 1160,
		.vts_def = 820,
		.reg_list = imx378_init_tab_1024_768_210fps,
		.binning_h = 0,
		.binning_v = 0,
	},
};

static struct imx378 *to_imx378(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct imx378, subdev);
}

static int reg_write(struct i2c_client *client, const u16 addr, const u8 data)
{
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;
	u8 tx[3];
	int ret;

	msg.addr = client->addr;
	msg.buf = tx;
	msg.len = 3;
	msg.flags = 0;
	tx[0] = addr >> 8;
	tx[1] = addr & 0xff;
	tx[2] = data;
	ret = i2c_transfer(adap, &msg, 1);
	mdelay(2);

	return ret == 1 ? 0 : -EIO;
}

static int reg_read(struct i2c_client *client, const u16 addr)
{
	u8 buf[2] = {addr >> 8, addr & 0xff};
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr  = client->addr,
			.flags = 0,
			.len   = 2,
			.buf   = buf,
		}, {
			.addr  = client->addr,
			.flags = I2C_M_RD,
			.len   = 1,
			.buf   = buf,
		},
	};

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret < 0) {
		dev_warn(&client->dev, "Reading register %x from %x failed\n",
			 addr, client->addr);
		return ret;
	}

	return buf[0];
}

static int reg_write_table(struct i2c_client *client,
			   const struct imx378_reg table[])
{
	const struct imx378_reg *reg;
	int ret;

	for (reg = table; reg->addr != IMX378_TABLE_END; reg++) {
		ret = reg_write(client, reg->addr, reg->val);
		if (ret < 0)
			return ret;
	}

	return 0;
}

/* V4L2 subdev video operations */
static int imx378_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx378 *priv = to_imx378(client);
	u8 reg = 0x00;
	int ret;

	if (!enable)
		return reg_write_table(client, stop);

	ret = reg_write_table(client, priv->cur_mode->reg_list);
	if (ret)
		return ret;

	/* Handle crop */
	/*
	ret  = reg_write(client, 0x0344, priv->crop_rect.left >> 8);
	ret |= reg_write(client, 0x0345, priv->crop_rect.left & 0xff);
	ret |= reg_write(client, 0x0346, priv->crop_rect.top >> 8);
	ret |= reg_write(client, 0x0347, priv->crop_rect.top & 0xff);
	ret |= reg_write(client, 0x0348, (priv->crop_rect.left + priv->crop_rect.width - 1) >> 8);
	ret |= reg_write(client, 0x0349, (priv->crop_rect.left + priv->crop_rect.width - 1) & 0xff);
	ret |= reg_write(client, 0x034A, (priv->crop_rect.top + priv->crop_rect.height - 1) >> 8);
	ret |= reg_write(client, 0x034B, (priv->crop_rect.top + priv->crop_rect.height - 1) & 0xff);
	ret |= reg_write(client, 0x034C, priv->crop_rect.width >> 8);
	ret |= reg_write(client, 0x034D, priv->crop_rect.width & 0xff);
	ret |= reg_write(client, 0x034E, priv->crop_rect.height >> 8);
	ret |= reg_write(client, 0x034F, priv->crop_rect.height & 0xff);
	*/

	if (ret)
		return ret;

	/* Handle flip/mirror */
	if (priv->hflip)
		reg |= 0x1;
	if (priv->vflip)
		reg |= 0x2;

	ret = reg_write(client, 0x0101, reg);
	if (ret)
		return ret;

	/* Handle test pattern */
	if (priv->test_pattern) {
		ret = reg_write(client, 0x0600, priv->test_pattern >> 8);
		ret |= reg_write(client, 0x0601, priv->test_pattern & 0xff);
		ret |= reg_write(client, 0x0602,
				 priv->test_pattern_solid_color_r >> 8);
		ret |= reg_write(client, 0x0603,
				 priv->test_pattern_solid_color_r & 0xff);
		ret |= reg_write(client, 0x0604,
				 priv->test_pattern_solid_color_gr >> 8);
		ret |= reg_write(client, 0x0605,
				 priv->test_pattern_solid_color_gr & 0xff);
		ret |= reg_write(client, 0x0606,
				 priv->test_pattern_solid_color_b >> 8);
		ret |= reg_write(client, 0x0607,
				 priv->test_pattern_solid_color_b & 0xff);
		ret |= reg_write(client, 0x0608,
				 priv->test_pattern_solid_color_gb >> 8);
		ret |= reg_write(client, 0x0609,
				 priv->test_pattern_solid_color_gb & 0xff);
		ret |= reg_write(client, 0x0620, priv->crop_rect.left >> 8);
		ret |= reg_write(client, 0x0621, priv->crop_rect.left & 0xff);
		ret |= reg_write(client, 0x0622, priv->crop_rect.top >> 8);
		ret |= reg_write(client, 0x0623, priv->crop_rect.top & 0xff);
		ret |= reg_write(client, 0x0624, priv->crop_rect.width >> 8);
		ret |= reg_write(client, 0x0625, priv->crop_rect.width & 0xff);
		ret |= reg_write(client, 0x0626, priv->crop_rect.height >> 8);
		ret |= reg_write(client, 0x0627, priv->crop_rect.height & 0xff);
	} else {
		ret = reg_write(client, 0x0600, 0x00);
		ret |= reg_write(client, 0x0601, 0x00);
	}

	priv->cur_vts = priv->cur_mode->vts_def - IMX378_EXP_LINES_MARGIN;
	if (ret)
		return ret;

	/* Set exposure and gain */

	ret  = reg_write(client, 0x0204, priv->analogue_gain >> 8);
	ret |= reg_write(client, 0x0205, priv->analogue_gain & 0xff);
	ret |= reg_write(client, 0x020E, priv->digital_gain_gr >> 8);
	ret |= reg_write(client, 0x020F, priv->digital_gain_gr & 0xff);
	ret |= reg_write(client, 0x0210, priv->digital_gain_r >> 8);
	ret |= reg_write(client, 0x0211, priv->digital_gain_r & 0xff);
	ret |= reg_write(client, 0x0212, priv->digital_gain_b >> 8);
	ret |= reg_write(client, 0x0213, priv->digital_gain_b & 0xff);
	ret |= reg_write(client, 0x0214, priv->digital_gain_gr >> 8);
	ret |= reg_write(client, 0x0215, priv->digital_gain_gr & 0xff);
	ret |= reg_write(client, 0x0202, priv->exposure_time >> 8);
	ret |= reg_write(client, 0x0203, priv->exposure_time & 0xff);
	if (ret)
		return ret;

	return reg_write_table(client, start);
}

/* V4L2 subdev core operations */
static int imx378_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx378 *priv = to_imx378(client);

	if (on)	{
		dev_dbg(&client->dev, "imx378 power on\n");
		clk_prepare_enable(priv->clk);
	} else if (!on) {
		dev_dbg(&client->dev, "imx378 power off\n");
		clk_disable_unprepare(priv->clk);
	}

	return 0;
}

/* V4L2 ctrl operations */
static int imx378_s_ctrl_test_pattern(struct v4l2_ctrl *ctrl)
{
	struct imx378 *priv =
	    container_of(ctrl->handler, struct imx378, ctrl_handler);

	switch (ctrl->val) {
	case TEST_PATTERN_DISABLED:
		priv->test_pattern = 0x0000;
		break;
	case TEST_PATTERN_SOLID_BLACK:
		priv->test_pattern = 0x0001;
		priv->test_pattern_solid_color_r = 0x0000;
		priv->test_pattern_solid_color_gr = 0x0000;
		priv->test_pattern_solid_color_b = 0x0000;
		priv->test_pattern_solid_color_gb = 0x0000;
		break;
	case TEST_PATTERN_SOLID_WHITE:
		priv->test_pattern = 0x0001;
		priv->test_pattern_solid_color_r = 0x0fff;
		priv->test_pattern_solid_color_gr = 0x0fff;
		priv->test_pattern_solid_color_b = 0x0fff;
		priv->test_pattern_solid_color_gb = 0x0fff;
		break;
	case TEST_PATTERN_SOLID_RED:
		priv->test_pattern = 0x0001;
		priv->test_pattern_solid_color_r = 0x0fff;
		priv->test_pattern_solid_color_gr = 0x0000;
		priv->test_pattern_solid_color_b = 0x0000;
		priv->test_pattern_solid_color_gb = 0x0000;
		break;
	case TEST_PATTERN_SOLID_GREEN:
		priv->test_pattern = 0x0001;
		priv->test_pattern_solid_color_r = 0x0000;
		priv->test_pattern_solid_color_gr = 0x0fff;
		priv->test_pattern_solid_color_b = 0x0000;
		priv->test_pattern_solid_color_gb = 0x0fff;
		break;
	case TEST_PATTERN_SOLID_BLUE:
		priv->test_pattern = 0x0001;
		priv->test_pattern_solid_color_r = 0x0000;
		priv->test_pattern_solid_color_gr = 0x0000;
		priv->test_pattern_solid_color_b = 0x0fff;
		priv->test_pattern_solid_color_gb = 0x0000;
		break;
	case TEST_PATTERN_COLOR_BAR:
		priv->test_pattern = 0x0002;
		break;
	case TEST_PATTERN_FADE_TO_GREY_COLOR_BAR:
		priv->test_pattern = 0x0003;
		break;
	case TEST_PATTERN_PN9:
		priv->test_pattern = 0x0004;
		break;
	case TEST_PATTERN_16_SPLIT_COLOR_BAR:
		priv->test_pattern = 0x0005;
		break;
	case TEST_PATTERN_16_SPLIT_INVERTED_COLOR_BAR:
		priv->test_pattern = 0x0006;
		break;
	case TEST_PATTERN_COLUMN_COUNTER:
		priv->test_pattern = 0x0007;
		break;
	case TEST_PATTERN_INVERTED_COLUMN_COUNTER:
		priv->test_pattern = 0x0008;
		break;
	case TEST_PATTERN_PN31:
		priv->test_pattern = 0x0009;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int imx378_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct imx378 *priv =
	    container_of(ctrl->handler, struct imx378, ctrl_handler);
	struct i2c_client *client = v4l2_get_subdevdata(&priv->subdev);
	u8 reg;
	int ret;
	u16 gain = 256;
	u16 a_gain = 256;
	u16 d_gain = 1;

	switch (ctrl->id) {
	case V4L2_CID_HFLIP:
		priv->hflip = ctrl->val;
		break;

	case V4L2_CID_VFLIP:
		priv->vflip = ctrl->val;
		break;

	case V4L2_CID_ANALOGUE_GAIN:
	case V4L2_CID_GAIN:
		/*
		 * hal transfer (gain * 256)  to kernel
		 * than divide into analog gain & digital gain in kernel
		 */

		gain = ctrl->val;
		if (gain < IMX378_DIGITAL_GAIN_MIN)
			gain = IMX378_DIGITAL_GAIN_MIN;
		if (gain > IMX378_DIGITAL_GAIN_MAX)
			gain = IMX378_DIGITAL_GAIN_MAX;
		if (gain >= IMX378_DIGITAL_GAIN_MIN && gain <= IMX378_ANALOGUE_GAIN_MAX) {
			a_gain = gain;
			d_gain = IMX378_DIGITAL_GAIN_MIN;
		} else {
			a_gain = IMX378_ANALOGUE_GAIN_MAX;
			d_gain = (gain * IMX378_ANALOGUE_GAIN_MULTIPLIER) / a_gain;
		}

		/*
		 * Analog gain, reg range[0, 978], gain value[1, 22.26]
		 * reg = 1024 - 1024 / again
		 * a_gain here is 256 multify
		 * so the reg = 1024 - 1024 * 256 / a_gain
		 */
		//priv->analogue_gain = (256 - (256 * 256) / a_gain);
		priv->analogue_gain = (1024 - (1024 * IMX378_ANALOGUE_GAIN_MULTIPLIER) / a_gain);
		if (a_gain < IMX378_ANALOGUE_GAIN_MIN)
			priv->analogue_gain = 0;
		if (priv->analogue_gain > 978)
			priv->analogue_gain = 978;

		/*
		 * Digital gain, reg range[256, 4095], gain rage[1, 16]
		 * reg = dgain * 256
		 */
		priv->digital_gain_gr = d_gain * IMX378_WHITE_BALANCE_GR / 100;
		if (priv->digital_gain_gr < 256)
			priv->digital_gain_gr = 256;
		if (priv->digital_gain_gr > 4095)
			priv->digital_gain_gr = 4095;
		priv->digital_gain_r = d_gain * IMX378_WHITE_BALANCE_R / 100;
		if (priv->digital_gain_r < 256)
			priv->digital_gain_r = 256;
		if (priv->digital_gain_r > 4095)
			priv->digital_gain_r = 4095;
		priv->digital_gain_b = d_gain * IMX378_WHITE_BALANCE_B / 100;
		if (priv->digital_gain_b < 256)
			priv->digital_gain_b = 256;
		if (priv->digital_gain_b > 4095)
			priv->digital_gain_b = 4095;
		priv->digital_gain_gb = d_gain * IMX378_WHITE_BALANCE_GB / 100;
		if (priv->digital_gain_gb < 256)
			priv->digital_gain_gb = 256;
		if (priv->digital_gain_gb > 4095)
			priv->digital_gain_gb = 4095;

		/*
		 * for bank A and bank B switch
		 * exposure time , gain, vts must change at the same time
		 * so the exposure & gain can reflect at the same frame
		 */
		ret  = reg_write(client, 0x0204, priv->analogue_gain >> 8);
		ret |= reg_write(client, 0x0205, priv->analogue_gain & 0xff);
		ret |= reg_write(client, 0x020E, priv->digital_gain_gr >> 8);
		ret |= reg_write(client, 0x020F, priv->digital_gain_gr & 0xff);
		ret |= reg_write(client, 0x0210, priv->digital_gain_r >> 8);
		ret |= reg_write(client, 0x0211, priv->digital_gain_r & 0xff);
		ret |= reg_write(client, 0x0212, priv->digital_gain_b >> 8);
		ret |= reg_write(client, 0x0213, priv->digital_gain_b & 0xff);
		ret |= reg_write(client, 0x0214, priv->digital_gain_gb >> 8);
		ret |= reg_write(client, 0x0215, priv->digital_gain_gb & 0xff);
		
		return ret;

	case V4L2_CID_EXPOSURE:
		priv->exposure_time = ctrl->val;
		ret  = reg_write(client, 0x0202, priv->exposure_time >> 8);
		ret |= reg_write(client, 0x0203, priv->exposure_time & 0xff);
		return ret;

	case V4L2_CID_TEST_PATTERN:
		return imx378_s_ctrl_test_pattern(ctrl);

	case V4L2_CID_VBLANK:
		if (ctrl->val < priv->cur_mode->vts_def)
			ctrl->val = priv->cur_mode->vts_def;
		if ((ctrl->val - IMX378_EXP_LINES_MARGIN) != priv->cur_vts)
			priv->cur_vts = ctrl->val - IMX378_EXP_LINES_MARGIN;
		ret  = reg_write(client, 0x0340, ((priv->cur_vts >> 8) & 0xff));
		ret |= reg_write(client, 0x0341, (priv->cur_vts & 0xff));
		return ret;

	default:
		return -EINVAL;
	}
	/* If enabled, apply settings immediately */
	reg = reg_read(client, 0x0100);
	if ((reg & 0x1f) == 0x01)
		imx378_s_stream(&priv->subdev, 1);

	return 0;
}

static int imx378_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index != 0)
		return -EINVAL;
	code->code = MEDIA_BUS_FMT_SBGGR10_1X10;

	return 0;
}

static int imx378_get_reso_dist(const struct imx378_mode *mode,
				struct v4l2_mbus_framefmt *framefmt)
{
	return abs(mode->width - framefmt->width) +
	       abs(mode->height - framefmt->height);
}

static const struct imx378_mode *imx378_find_best_fit(
					struct v4l2_subdev_format *fmt)
{
	struct v4l2_mbus_framefmt *framefmt = &fmt->format;
	int dist;
	int cur_best_fit = 0;
	int cur_best_fit_dist = -1;
	int i;

	for (i = 0; i < ARRAY_SIZE(supported_modes); i++) {
		dist = imx378_get_reso_dist(&supported_modes[i], framefmt);
		if (cur_best_fit_dist == -1 || dist < cur_best_fit_dist) {
			cur_best_fit_dist = dist;
			cur_best_fit = i;
		}
	}

	return &supported_modes[cur_best_fit];
}

static int imx378_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx378 *priv = to_imx378(client);
	const struct imx378_mode *mode;
	s64 h_blank, v_blank, pixel_rate;

	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY)
		return 0;

	mode = imx378_find_best_fit(fmt);
	fmt->format.code = MEDIA_BUS_FMT_SBGGR10_1X10;
	fmt->format.width = mode->width;
	fmt->format.height = mode->height;
	fmt->format.field = V4L2_FIELD_NONE;
	priv->cur_mode = mode;
	h_blank = mode->hts_def - mode->width;
	__v4l2_ctrl_modify_range(priv->hblank, h_blank,
					h_blank, 1, h_blank);
	v_blank = mode->vts_def - mode->height;
	__v4l2_ctrl_modify_range(priv->vblank, v_blank,
					v_blank,
					1, v_blank);
	pixel_rate = mode->vts_def * mode->hts_def * mode->max_fps;
	__v4l2_ctrl_modify_range(priv->pixel_rate, pixel_rate,
					pixel_rate, 1, pixel_rate);

	/* reset crop window */
	priv->crop_rect.left = (4056 - (mode->width  << mode->binning_h)) / 2;
	if (priv->crop_rect.left < 0)
		priv->crop_rect.left = 0;
	priv->crop_rect.top  = (3040 - (mode->height << mode->binning_v)) / 2;
	if (priv->crop_rect.top < 0)
		priv->crop_rect.top = 0;
	priv->crop_rect.width  = (mode->width  << mode->binning_h);
	priv->crop_rect.height = (mode->height << mode->binning_v);

	return 0;
}

static int imx378_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx378 *priv = to_imx378(client);
	const struct imx378_mode *mode = priv->cur_mode;

	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY)
		return 0;

	fmt->format.width = mode->width;
	fmt->format.height = mode->height;
	fmt->format.code = MEDIA_BUS_FMT_SBGGR10_1X10;
	fmt->format.field = V4L2_FIELD_NONE;

	return 0;
}

/* Various V4L2 operations tables */
static struct v4l2_subdev_video_ops imx378_subdev_video_ops = {
	.s_stream = imx378_s_stream,
};

static struct v4l2_subdev_core_ops imx378_subdev_core_ops = {
	.s_power = imx378_s_power,
};

static const struct v4l2_subdev_pad_ops imx378_subdev_pad_ops = {
	.enum_mbus_code = imx378_enum_mbus_code,
	.set_fmt = imx378_set_fmt,
	.get_fmt = imx378_get_fmt,
};

static struct v4l2_subdev_ops imx378_subdev_ops = {
	.core = &imx378_subdev_core_ops,
	.video = &imx378_subdev_video_ops,
	.pad = &imx378_subdev_pad_ops,
};

static const struct v4l2_ctrl_ops imx378_ctrl_ops = {
	.s_ctrl = imx378_s_ctrl,
};

static int imx378_video_probe(struct i2c_client *client)
{
	struct v4l2_subdev *subdev = i2c_get_clientdata(client);
	u16 model_id;
	u32 lot_id;
	u32 chip_id;
	int ret;

	ret = imx378_s_power(subdev, 1);
	if (ret < 0)
		return ret;

	/* Check and show model, lot, and chip ID. */
	ret = reg_read(client, 0x0016);
	if (ret < 0) {
		dev_err(&client->dev, "Failure to read Model ID (high byte)\n");
		goto done;
	}
	model_id = ret << 8;

	ret = reg_read(client, 0x0017);
	if (ret < 0) {
		dev_err(&client->dev, "Failure to read Model ID (low byte)\n");
		goto done;
	}
	model_id |= ret;

	ret = reg_read(client, 0x0018);
	if (ret < 0) {
		dev_err(&client->dev, "Failure to read Lot ID (high byte)\n");
		goto done;
	}
	lot_id = ret << 16;

	ret = reg_read(client, 0x0019);
	if (ret < 0) {
		dev_err(&client->dev, "Failure to read Lot ID (mid byte)\n");
		goto done;
	}
	lot_id |= ret << 8;

	ret = reg_read(client, 0x001A);
	if (ret < 0) {
		dev_err(&client->dev, "Failure to read Lot ID (low byte)\n");
		goto done;
	}
	lot_id |= ret;

	ret = reg_read(client, 0x001C);
	if (ret < 0) {
		dev_err(&client->dev, "Failure to read Chip ID (high byte)\n");
		goto done;
	}
	chip_id = ret << 8;

	ret = reg_read(client, 0x001D);
	if (ret < 0) {
		dev_err(&client->dev, "Failure to read Chip ID (low byte)\n");
		goto done;
	}
	chip_id |= ret;

	ret = reg_read(client, 0x001E);
	if (ret < 0) {
		dev_err(&client->dev, "Failure to read Chip ID (low byte)\n");
		goto done;
	}
	chip_id |= ret;

	ret = reg_read(client, 0x001F);
	if (ret < 0) {
		dev_err(&client->dev, "Failure to read Chip ID (low byte)\n");
		goto done;
	}
	chip_id |= ret;

	if (model_id != 0x0378) {
		dev_err(&client->dev, "Model ID: %x not supported!\n",
			model_id);
		ret = -ENODEV;
		goto done;
	}
	dev_info(&client->dev,
		 "Model ID 0x%04x, Lot ID 0x%06x, Chip ID 0x%08x\n",
		 model_id, lot_id, chip_id);
done:
	imx378_s_power(subdev, 0);
	return ret;
}

static int imx378_ctrls_init(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx378 *priv = to_imx378(client);
	const struct imx378_mode *mode = priv->cur_mode;
	s64 pixel_rate, h_blank, v_blank;
	int ret;

	v4l2_ctrl_handler_init(&priv->ctrl_handler, 10);
	v4l2_ctrl_new_std(&priv->ctrl_handler, &imx378_ctrl_ops,
			  V4L2_CID_HFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&priv->ctrl_handler, &imx378_ctrl_ops,
			  V4L2_CID_VFLIP, 0, 1, 1, 0);

	/* exposure */
	v4l2_ctrl_new_std(&priv->ctrl_handler, &imx378_ctrl_ops,
			  V4L2_CID_ANALOGUE_GAIN,
			  IMX378_ANALOGUE_GAIN_MIN,
			  IMX378_ANALOGUE_GAIN_MAX,
			  1, IMX378_ANALOGUE_GAIN_DEFAULT);
	v4l2_ctrl_new_std(&priv->ctrl_handler, &imx378_ctrl_ops,
			  V4L2_CID_GAIN,
			  IMX378_DIGITAL_GAIN_MIN,
			  IMX378_DIGITAL_GAIN_MAX, 1,
			  IMX378_DIGITAL_GAIN_DEFAULT);
	v4l2_ctrl_new_std(&priv->ctrl_handler, &imx378_ctrl_ops,
			  V4L2_CID_EXPOSURE,
			  IMX378_DIGITAL_EXPOSURE_MIN,
			  IMX378_DIGITAL_EXPOSURE_MAX, 1,
			  IMX378_DIGITAL_EXPOSURE_DEFAULT);

	/* blank */
	h_blank = mode->hts_def - mode->width;
	priv->hblank = v4l2_ctrl_new_std(&priv->ctrl_handler, NULL, V4L2_CID_HBLANK,
			  h_blank, h_blank, 1, h_blank);
	v_blank = mode->vts_def - mode->height;
	priv->vblank = v4l2_ctrl_new_std(&priv->ctrl_handler, NULL, V4L2_CID_VBLANK,
			  v_blank, v_blank, 1, v_blank);

	/* freq */
	v4l2_ctrl_new_int_menu(&priv->ctrl_handler, NULL, V4L2_CID_LINK_FREQ,
			       0, 0, link_freq_menu_items);
	pixel_rate = mode->vts_def * mode->hts_def * mode->max_fps;
	priv->pixel_rate = v4l2_ctrl_new_std(&priv->ctrl_handler, NULL, V4L2_CID_PIXEL_RATE,
			  0, pixel_rate, 1, pixel_rate);

	v4l2_ctrl_new_std_menu_items(&priv->ctrl_handler, &imx378_ctrl_ops,
				     V4L2_CID_TEST_PATTERN,
				     ARRAY_SIZE(tp_qmenu) - 1, 0, 0, tp_qmenu);

	priv->subdev.ctrl_handler = &priv->ctrl_handler;
	if (priv->ctrl_handler.error) {
		dev_err(&client->dev, "Error %d adding controls\n",
			priv->ctrl_handler.error);
		ret = priv->ctrl_handler.error;
		goto error;
	}

	ret = v4l2_ctrl_handler_setup(&priv->ctrl_handler);
	if (ret < 0) {
		dev_err(&client->dev, "Error %d setting default controls\n",
			ret);
		goto error;
	}

	return 0;
error:
	v4l2_ctrl_handler_free(&priv->ctrl_handler);
	return ret;
}

static int imx378_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct imx378 *priv;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int ret;

	printk("[jcliao] imx378_probe \n");
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_warn(&adapter->dev,
			 "I2C-Adapter doesn't support I2C_FUNC_SMBUS_BYTE\n");
		return -EIO;
	}
	priv = devm_kzalloc(&client->dev, sizeof(struct imx378), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->clk = devm_clk_get(&client->dev, NULL);
	if (IS_ERR(priv->clk)) {
		dev_info(&client->dev, "Error %ld getting clock\n",
			 PTR_ERR(priv->clk));
		return -EPROBE_DEFER;
	}

	/* 1920 * 1080 by default */
	priv->cur_mode = &supported_modes[0];
	priv->crop_rect.left = (4056 - (priv->cur_mode->width  << priv->cur_mode->binning_h)) / 2;
	if (priv->crop_rect.left < 0)
		priv->crop_rect.left = 0;
	priv->crop_rect.top  = (3040 - (priv->cur_mode->height << priv->cur_mode->binning_v)) / 2;
	if (priv->crop_rect.top < 0)
		priv->crop_rect.top = 0;
	priv->crop_rect.width = priv->cur_mode->width;
	priv->crop_rect.height = priv->cur_mode->height;

	v4l2_i2c_subdev_init(&priv->subdev, client, &imx378_subdev_ops);
	ret = imx378_ctrls_init(&priv->subdev);
	if (ret < 0)
		return ret;
	ret = imx378_video_probe(client);
	if (ret < 0)
		return ret;

	priv->subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	priv->pad.flags = MEDIA_PAD_FL_SOURCE;
	priv->subdev.entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
	ret = media_entity_init(&priv->subdev.entity, 1, &priv->pad, 0);
	if (ret < 0)
		return ret;

	ret = v4l2_async_register_subdev(&priv->subdev);
	if (ret < 0)
		return ret;

	return ret;
}

static int imx378_remove(struct i2c_client *client)
{
	struct imx378 *priv = to_imx378(client);

	v4l2_async_unregister_subdev(&priv->subdev);
	media_entity_cleanup(&priv->subdev.entity);
	v4l2_ctrl_handler_free(&priv->ctrl_handler);

	return 0;
}

static const struct i2c_device_id imx378_id[] = {
	{"imx378", 0},
	{}
};

static const struct of_device_id imx378_of_match[] = {
	{ .compatible = "sony,imx378" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, imx378_of_match);

MODULE_DEVICE_TABLE(i2c, imx378_id);
static struct i2c_driver imx378_i2c_driver = {
	.driver = {
		.of_match_table = of_match_ptr(imx378_of_match),
		.name = "imx378",
	},
	.probe = imx378_probe,
	.remove = imx378_remove,
	.id_table = imx378_id,
};

module_i2c_driver(imx378_i2c_driver);
MODULE_DESCRIPTION("Sony IMX378 Camera driver");
MODULE_AUTHOR("Guennadi Liakhovetski <g.liakhovetski@gmx.de>");
MODULE_LICENSE("GPL v2");
