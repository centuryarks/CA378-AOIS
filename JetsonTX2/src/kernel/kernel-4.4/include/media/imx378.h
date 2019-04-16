/**
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

#ifndef __IMX378_H__
#define __IMX378_H__

#include <linux/ioctl.h>  /* For IOCTL macros */
#include <media/nvc.h>
#include <media/nvc_image.h>

#define IMX378_IOCTL_SET_MODE                   _IOW('o', 1, struct imx378_mode)
#define IMX378_IOCTL_GET_STATUS                 _IOR('o', 2, __u8)
#define IMX378_IOCTL_SET_FRAME_LENGTH           _IOW('o', 3, __u32)
#define IMX378_IOCTL_SET_COARSE_TIME            _IOW('o', 4, __u32)
#define IMX378_IOCTL_SET_GAIN                   _IOW('o', 5, __u16)
#define IMX378_IOCTL_GET_SENSORDATA             _IOR('o', 6, struct imx378_sensordata)
#define IMX378_IOCTL_SET_GROUP_HOLD             _IOW('o', 7, struct imx378_ae)
#define IMX378_IOCTL_SET_HDR_COARSE_TIME        _IOW('o', 8, struct imx378_hdr)
#define IMX378_IOCTL_SET_POWER                  _IOW('o', 20, __u32)

#define IMX378_COARSE_TIME_ADDR_MSB             0x0202
#define IMX378_COARSE_TIME_ADDR_LSB             0x0203

#define IMX378_GAIN_ADDR_MSB                    0x0204
#define IMX378_GAIN_ADDR_LSB                    0x0205

#define IMX378_FRAME_LENGTH_ADDR_MSB            0x0340
#define IMX378_FRAME_LENGTH_ADDR_LSB            0x0341

struct imx378_mode {
        __u32 xres;
        __u32 yres;
        __u32 frame_length;
        __u32 coarse_time;
        __u32 coarse_time_short;
        __u16 gain;
        __u8 hdr_en;
};

struct imx378_hdr {
        __u32 coarse_time_long;
        __u32 coarse_time_short;
};

struct imx378_ae {
        __u32 frame_length;
        __u8  frame_length_enable;
        __u32 coarse_time;
        __u32 coarse_time_short;
        __u8  coarse_time_enable;
        __s32 gain;
        __u8  gain_enable;
};

#ifdef __KERNEL__
struct imx378_power_rail {
        struct regulator *dvdd;
        struct regulator *avdd;
        struct regulator *iovdd;
        struct clk *mclk;
        unsigned int pwdn_gpio;
        unsigned int reset_gpio;
        unsigned int af_gpio;
};

struct imx378_platform_data {
        const char *mclk_name; /* NULL for default default_mclk */
        unsigned int reset_gpio;
        unsigned int af_gpio;
        bool ext_reg;
        int (*power_on)(struct imx378_power_rail *pw);
        int (*power_off)(struct imx378_power_rail *pw);
};
#endif  /* __KERNEL__ */

#endif  /* __IMX378_H__ */
