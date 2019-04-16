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

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/module.h>

#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#include <media/tegra-v4l2-camera.h>
#include <media/camera_common.h>
#include <media/imx378.h>

#include "imx378_mode_tbls.h"

#define IMX378_MAX_COARSE_DIFF          20

#define IMX378_GAIN_SHIFT               8
#define IMX378_MIN_GAIN                 (1  << IMX378_GAIN_SHIFT)
#define IMX378_MAX_GAIN                 (22 << IMX378_GAIN_SHIFT)
#define IMX378_MIN_FRAME_LENGTH         (0x0384)
#define IMX378_MAX_FRAME_LENGTH         (0xFFFF)
#define IMX378_MIN_EXPOSURE_COARSE      (0x0001)
#define IMX378_MAX_EXPOSURE_COARSE      \
        (IMX378_MAX_FRAME_LENGTH-IMX378_MAX_COARSE_DIFF)

#define IMX378_DEFAULT_GAIN             IMX378_MIN_GAIN
#define IMX378_DEFAULT_FRAME_LENGTH     (0x099C)
#define IMX378_DEFAULT_EXPOSURE_COARSE  \
        (IMX378_DEFAULT_FRAME_LENGTH-IMX378_MAX_COARSE_DIFF)

#define IMX378_DEFAULT_MODE             IMX378_MODE_3840X2160
#define IMX378_DEFAULT_WIDTH            3840
#define IMX378_DEFAULT_HEIGHT           2160
#define IMX378_DEFAULT_DATAFMT          MEDIA_BUS_FMT_SRGGB10_1X10
#define IMX378_DEFAULT_CLK_FREQ         18000000

static int num_multi_camera = 0;        // number of multi camera

struct imx378 {
        struct camera_common_power_rail power;
        int                             num_ctrls;
        struct v4l2_ctrl_handler        ctrl_handler;
        struct i2c_client               *i2c_client;
        struct v4l2_subdev              *subdev;
        struct media_pad                pad;
        u32                             vmax;
        s32                             group_hold_prev;
        bool                            group_hold_en;
        struct regmap                   *regmap;
        struct camera_common_data       *s_data;
        struct camera_common_pdata      *pdata;
        struct v4l2_ctrl                *ctrls[];
};

static const struct regmap_config sensor_regmap_config = {
        .reg_bits = 16,
        .val_bits = 8,
        .cache_type = REGCACHE_RBTREE,
        .use_single_rw = true,
};

static int imx378_s_ctrl(struct v4l2_ctrl *ctrl);

static const struct v4l2_ctrl_ops imx378_ctrl_ops = {
        .s_ctrl         = imx378_s_ctrl,
};

static struct v4l2_ctrl_config ctrl_config_list[] = {
/* Do not change the name field for the controls! */
        {
                .ops = &imx378_ctrl_ops,
                .id = TEGRA_CAMERA_CID_GAIN,
                .name = "Gain",
                .type = V4L2_CTRL_TYPE_INTEGER,
                .flags = V4L2_CTRL_FLAG_SLIDER,
                .min = IMX378_MIN_GAIN,
                .max = IMX378_MAX_GAIN,
                .def = IMX378_DEFAULT_GAIN,
                .step = 1,
        },
        {
                .ops = &imx378_ctrl_ops,
                .id = TEGRA_CAMERA_CID_FRAME_LENGTH,
                .name = "Frame Length",
                .type = V4L2_CTRL_TYPE_INTEGER,
                .flags = V4L2_CTRL_FLAG_SLIDER,
                .min = IMX378_MIN_FRAME_LENGTH,
                .max = IMX378_MAX_FRAME_LENGTH,
                .def = IMX378_DEFAULT_FRAME_LENGTH,
                .step = 1,
        },
        {
                .ops = &imx378_ctrl_ops,
                .id = TEGRA_CAMERA_CID_COARSE_TIME,
                .name = "Coarse Time",
                .type = V4L2_CTRL_TYPE_INTEGER,
                .flags = V4L2_CTRL_FLAG_SLIDER,
                .min = IMX378_MIN_EXPOSURE_COARSE,
                .max = IMX378_MAX_EXPOSURE_COARSE,
                .def = IMX378_DEFAULT_EXPOSURE_COARSE,
                .step = 1,
        },
        {
                .ops = &imx378_ctrl_ops,
                .id = TEGRA_CAMERA_CID_COARSE_TIME_SHORT,
                .name = "Coarse Time Short",
                .type = V4L2_CTRL_TYPE_INTEGER,
                .flags = V4L2_CTRL_FLAG_SLIDER,
                .min = IMX378_MIN_EXPOSURE_COARSE,
                .max = IMX378_MAX_EXPOSURE_COARSE,
                .def = IMX378_DEFAULT_EXPOSURE_COARSE,
                .step = 1,
        },
        {
                .ops = &imx378_ctrl_ops,
                .id = TEGRA_CAMERA_CID_GROUP_HOLD,
                .name = "Group Hold",
                .type = V4L2_CTRL_TYPE_INTEGER_MENU,
                .min = 0,
                .max = ARRAY_SIZE(switch_ctrl_qmenu) - 1,
                .menu_skip_mask = 0,
                .def = 0,
                .qmenu_int = switch_ctrl_qmenu,
        },
        {
                .ops = &imx378_ctrl_ops,
                .id = TEGRA_CAMERA_CID_HDR_EN,
                .name = "HDR enable",
                .type = V4L2_CTRL_TYPE_INTEGER_MENU,
                .min = 0,
                .max = ARRAY_SIZE(switch_ctrl_qmenu) - 1,
                .menu_skip_mask = 0,
                .def = 0,
                .qmenu_int = switch_ctrl_qmenu,
        },
};

static inline void imx378_get_vmax_regs(imx378_reg *regs,
                                u32 vmax)
{
        regs->addr = IMX378_FRAME_LENGTH_ADDR_MSB;
        regs->val = (vmax >> 8) & 0xff;
        (regs + 1)->addr = IMX378_FRAME_LENGTH_ADDR_LSB;
        (regs + 1)->val = (vmax) & 0xff;
}

static inline void imx378_get_shr_regs(imx378_reg *regs,
                                u32 shr)
{
        regs->addr = IMX378_COARSE_TIME_ADDR_MSB;
        regs->val = (shr >> 8) & 0xff;
        (regs + 1)->addr = IMX378_COARSE_TIME_ADDR_LSB;
        (regs + 1)->val = (shr) & 0xff;
}

static inline void imx378_get_gain_reg(imx378_reg *regs,
                                u16 gain)
{
        regs->addr = IMX378_GAIN_ADDR_MSB;
        regs->val = (gain >> 8) & 0xff;
        (regs + 1)->addr = IMX378_GAIN_ADDR_LSB;
        (regs + 1)->val = (gain) & 0xff;
}

static int test_mode;
module_param(test_mode, int, 0644);

static inline int imx378_read_reg(struct camera_common_data *s_data,
                                u16 addr, u8 *val)
{
        struct imx378 *priv = (struct imx378 *)s_data->priv;
        int err = 0;
        u32 reg_val = 0;

        err = regmap_read(priv->regmap, addr, &reg_val);
        *val = reg_val & 0xFF;

        return err;
}

static int imx378_write_reg(struct camera_common_data *s_data, u16 addr, u8 val)
{
        int err;
        struct imx378 *priv = (struct imx378 *)s_data->priv;

        err = regmap_write(priv->regmap, addr, val);
        if (err)
                pr_err("%s:i2c write failed, %x = %x\n",
                        __func__, addr, val);

        return err;
}

static int imx378_write_table(struct imx378 *priv,
                                const imx378_reg table[])
{
        return regmap_util_write_table_8(priv->regmap,
                                         table,
                                         NULL, 0,
                                         IMX378_TABLE_WAIT_MS,
                                         IMX378_TABLE_END);
}

static int imx378_power_on(struct camera_common_data *s_data)
{
        int err = 0;
        struct imx378 *priv = (struct imx378 *)s_data->priv;
        struct camera_common_power_rail *pw = &priv->power;

        dev_dbg(&priv->i2c_client->dev, "%s: power on\n", __func__);

        if (priv->pdata && priv->pdata->power_on) {
                err = priv->pdata->power_on(pw);
                if (err)
                        pr_err("%s failed.\n", __func__);
                else
                        pw->state = SWITCH_ON;
                return err;
        }

#if 0   // currently unused
        if (pw->reset_gpio)
                gpio_set_value(pw->reset_gpio, 0);
        if (pw->af_gpio)
                gpio_set_value(pw->af_gpio, 1);
        if (pw->pwdn_gpio)
                gpio_set_value(pw->pwdn_gpio, 0);
        usleep_range(10, 20);

        if (pw->dvdd)
                err = regulator_enable(pw->dvdd);
        if (err)
                goto imx378_dvdd_fail;

        if (pw->iovdd)
                err = regulator_enable(pw->iovdd);
        if (err)
                goto imx378_iovdd_fail;

        if (pw->avdd)
                err = regulator_enable(pw->avdd);
        if (err)
                goto imx378_avdd_fail;
#endif

        usleep_range(1, 2);
        if (pw->reset_gpio)
                gpio_set_value(pw->reset_gpio, 1);
        if (pw->pwdn_gpio)
                gpio_set_value(pw->pwdn_gpio, 1);

        usleep_range(9000, 9010);

        pw->state = SWITCH_ON;

        num_multi_camera++;
        dev_dbg(&priv->i2c_client->dev, "[%s]multi camera num: %d\n", __func__, num_multi_camera);

        return 0;

#if 0   // currently unused
imx378_dvdd_fail:
        if (pw->af_gpio)
                gpio_set_value(pw->af_gpio, 0);

imx378_iovdd_fail:
        regulator_disable(pw->dvdd);

imx378_avdd_fail:
        regulator_disable(pw->iovdd);

        pr_err("%s failed.\n", __func__);
        return -ENODEV;
#endif
}

static int imx378_power_off(struct camera_common_data *s_data)
{
        int err = 0;
        struct imx378 *priv = (struct imx378 *)s_data->priv;
        struct camera_common_power_rail *pw = &priv->power;

        dev_dbg(&priv->i2c_client->dev, "%s: power off\n", __func__);

        if (priv->pdata->power_off) {
                err = priv->pdata->power_off(pw);
                if (err) {
                        pr_err("%s failed.\n", __func__);
                        return err;
                } else {
                        goto power_off_done;
                }
        }

#if 0   // currently unused
        usleep_range(1, 2);
        if (pw->reset_gpio)
                gpio_set_value(pw->reset_gpio, 0);
        if (pw->af_gpio)
                gpio_set_value(pw->af_gpio, 0);
        if (pw->pwdn_gpio)
                gpio_set_value(pw->pwdn_gpio, 0);
        usleep_range(1, 2);

        if (pw->avdd)
                regulator_disable(pw->avdd);
        if (pw->iovdd)
                regulator_disable(pw->iovdd);
        if (pw->dvdd)
                regulator_disable(pw->dvdd);
#endif

power_off_done:
        pw->state = SWITCH_OFF;

        num_multi_camera--;
        dev_dbg(&priv->i2c_client->dev, "[%s]multi camera num: %d\n", __func__, num_multi_camera);

        return 0;
}

static int imx378_power_put(struct imx378 *priv)
{
        struct camera_common_power_rail *pw = &priv->power;
        if (unlikely(!pw))
                return -EFAULT;

        if (likely(pw->avdd))
                regulator_put(pw->avdd);

        if (likely(pw->iovdd))
                regulator_put(pw->iovdd);

        if (likely(pw->dvdd))
                regulator_put(pw->dvdd);

        pw->avdd = NULL;
        pw->iovdd = NULL;
        pw->dvdd = NULL;

        return 0;
}

static int imx378_power_get(struct imx378 *priv)
{
        struct camera_common_power_rail *pw = &priv->power;
        struct camera_common_pdata *pdata = priv->pdata;
        const char *mclk_name;
        const char *parentclk_name;
        struct clk *parent;
        int err = 0;

        mclk_name = priv->pdata->mclk_name ?
                    priv->pdata->mclk_name : "cam_mclk1";
        pw->mclk = devm_clk_get(&priv->i2c_client->dev, mclk_name);
        if (IS_ERR(pw->mclk)) {
                dev_err(&priv->i2c_client->dev,
                        "unable to get clock %s\n", mclk_name);
                return PTR_ERR(pw->mclk);
        }

        parentclk_name = priv->pdata->parentclk_name;
        if (parentclk_name) {
                parent = devm_clk_get(&priv->i2c_client->dev, parentclk_name);
                if (IS_ERR(parent)) {
                        dev_err(&priv->i2c_client->dev,
                                "unable to get parent clcok %s",
                                parentclk_name);
                } else
                        clk_set_parent(pw->mclk, parent);
        }

        /* ananlog 2.8v */
        err |= camera_common_regulator_get(&priv->i2c_client->dev,
                        &pw->avdd, pdata->regulators.avdd);
        /* digital 1.2v */
        err |= camera_common_regulator_get(&priv->i2c_client->dev,
                        &pw->dvdd, pdata->regulators.dvdd);
        /* IO 1.8v */
        err |= camera_common_regulator_get(&priv->i2c_client->dev,
                        &pw->iovdd, pdata->regulators.iovdd);

        if (!err) {
                pw->reset_gpio = pdata->reset_gpio;
                pw->af_gpio = pdata->af_gpio;
                pw->pwdn_gpio = pdata->pwdn_gpio;
        }

        pw->state = SWITCH_OFF;
        return err;
}

static int imx378_set_gain(struct imx378 *priv, s32 val);
static int imx378_set_frame_length(struct imx378 *priv, s32 val);
static int imx378_set_coarse_time(struct imx378 *priv, s32 val);

static int imx378_s_stream(struct v4l2_subdev *sd, int enable)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct camera_common_data *s_data = to_camera_common_data(&client->dev);
        struct imx378 *priv = (struct imx378 *)s_data->priv;
        struct v4l2_control control;
        int err;

        dev_dbg(&client->dev, "%s++\n", __func__);

        imx378_write_table(priv, mode_table[IMX378_MODE_STOP_STREAM]);

        if (!enable)
                return 0;

        dev_dbg(&client->dev, "%s mode[%d]\n", __func__, s_data->mode);

        err = imx378_write_table(priv, mode_table[s_data->mode]);
        if (err)
                goto exit;


        if (s_data->override_enable) {
                /* write list of override regs for the asking frame length, */
                /* coarse integration time, and gain.                       */
                control.id = TEGRA_CAMERA_CID_GAIN;
                err = v4l2_g_ctrl(&priv->ctrl_handler, &control);
                err |= imx378_set_gain(priv, control.value);
                if (err)
                        dev_dbg(&client->dev,
                                "%s: error gain override\n", __func__);

                control.id = TEGRA_CAMERA_CID_FRAME_LENGTH;
                err = v4l2_g_ctrl(&priv->ctrl_handler, &control);
                err |= imx378_set_frame_length(priv, control.value);
                if (err)
                        dev_dbg(&client->dev,
                                "%s: error frame length override\n", __func__);

                control.id = TEGRA_CAMERA_CID_COARSE_TIME;
                err = v4l2_g_ctrl(&priv->ctrl_handler, &control);
                err |= imx378_set_coarse_time(priv, control.value);
                if (err)
                        dev_dbg(&client->dev,
                                "%s: error coarse time override\n", __func__);
        }

        if (test_mode) {
                err = imx378_write_table(priv,
                        mode_table[IMX378_MODE_TEST_PATTERN]);
                if (err)
                        goto exit;
                }

        err = imx378_write_table(priv, mode_table[IMX378_MODE_START_STREAM]);
        if (err)
                goto exit;


        return 0;
exit:
        dev_dbg(&client->dev, "%s: error setting stream\n", __func__);
        return err;
}

static int imx378_get_fmt(struct v4l2_subdev *sd,
                struct v4l2_subdev_pad_config *cfg,
                struct v4l2_subdev_format *format)
{
        return camera_common_g_fmt(sd, &format->format);
}

static int imx378_set_fmt(struct v4l2_subdev *sd,
                struct v4l2_subdev_pad_config *cfg,
        struct v4l2_subdev_format *format)
{
        int ret;

        if (format->which == V4L2_SUBDEV_FORMAT_TRY)
                ret = camera_common_try_fmt(sd, &format->format);
        else
                ret = camera_common_s_fmt(sd, &format->format);

        return ret;
}

static int imx378_g_input_status(struct v4l2_subdev *sd, u32 *status)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct camera_common_data *s_data = to_camera_common_data(&client->dev);
        struct imx378 *priv = (struct imx378 *)s_data->priv;
        struct camera_common_power_rail *pw = &priv->power;

        *status = pw->state == SWITCH_ON;
        return 0;
}

static struct v4l2_subdev_video_ops imx378_subdev_video_ops = {
        .s_stream       = imx378_s_stream,
        .g_mbus_config  = camera_common_g_mbus_config,
        .g_input_status = imx378_g_input_status,
};

static struct v4l2_subdev_core_ops imx378_subdev_core_ops = {
        .s_power        = camera_common_s_power,
};

static struct v4l2_subdev_pad_ops imx378_subdev_pad_ops = {
        .set_fmt        = imx378_set_fmt,
        .get_fmt        = imx378_get_fmt,
        .enum_mbus_code = camera_common_enum_mbus_code,
        .enum_frame_size        = camera_common_enum_framesizes,
        .enum_frame_interval    = camera_common_enum_frameintervals,
};

static struct v4l2_subdev_ops imx378_subdev_ops = {
        .core   = &imx378_subdev_core_ops,
        .video  = &imx378_subdev_video_ops,
        .pad    = &imx378_subdev_pad_ops,
};

static struct of_device_id imx378_of_match[] = {
        { .compatible = "nvidia,imx378", },
        { },
};

static struct camera_common_sensor_ops imx378_common_ops = {
        .power_on = imx378_power_on,
        .power_off = imx378_power_off,
        .write_reg = imx378_write_reg,
        .read_reg = imx378_read_reg,
};

static int imx378_set_gain(struct imx378 *priv, s32 val)
{
        u8 data[2];
        u16 gain;
        int err;

        /* translate value */
        gain = 1024 - (1024 * (1 << IMX378_GAIN_SHIFT) / val);

        dev_dbg(&priv->i2c_client->dev,
                 "%s: val: %d\n", __func__, gain);

        data[0] = (gain >> 8) & 0xff;
        data[1] = gain & 0xff;
        err = regmap_raw_write(priv->regmap, IMX378_GAIN_ADDR_MSB,
                data, 2);
        if (err)
                goto fail;

        return 0;

fail:
        dev_dbg(&priv->i2c_client->dev,
                 "%s: GAIN control error\n", __func__);
        return err;
}

static int imx378_set_frame_length(struct imx378 *priv, s32 val)
{
        u8 data[2];
        int err;

        dev_dbg(&priv->i2c_client->dev,
                 "%s: val: %d\n", __func__, val);

        data[0] = (val >> 8) & 0xff;
        data[1] = val & 0xff;
        err = regmap_raw_write(priv->regmap, IMX378_FRAME_LENGTH_ADDR_MSB,
                data, 2);
        if (err)
                goto fail;

        return 0;

fail:
        dev_dbg(&priv->i2c_client->dev,
                 "%s: FRAME_LENGTH control error\n", __func__);
        return err;
}

static int imx378_set_coarse_time(struct imx378 *priv, s32 val)
{
        u8 data[2];
        int err;

        dev_dbg(&priv->i2c_client->dev,
                 "%s: val: %d\n", __func__, val);

        data[0] = (val >> 8) & 0xff;
        data[1] = val & 0xff;
        err = regmap_raw_write(priv->regmap, IMX378_COARSE_TIME_ADDR_MSB,
                data, 2);
        if (err)
                goto fail;

        return 0;

fail:
        dev_dbg(&priv->i2c_client->dev,
                 "%s: COARSE_TIME control error\n", __func__);
        return err;
}

static int imx378_verify_streaming(struct imx378 *priv)
{
        int err = 0;

        err = camera_common_s_power(priv->subdev, true);
        if (err)
                return err;

        err = imx378_s_stream(priv->subdev, true);
        if (err)
                goto error;

error:
        imx378_s_stream(priv->subdev, false);
        camera_common_s_power(priv->subdev, false);

        return err;
}

static int imx378_s_ctrl(struct v4l2_ctrl *ctrl)
{
        struct imx378 *priv =
                container_of(ctrl->handler, struct imx378, ctrl_handler);
        int err = 0;

        if (priv->power.state == SWITCH_OFF)
                return 0;

        switch (ctrl->id) {
        case TEGRA_CAMERA_CID_GAIN:
                err = imx378_set_gain(priv, ctrl->val);
                break;
        case TEGRA_CAMERA_CID_FRAME_LENGTH:
                err = imx378_set_frame_length(priv, ctrl->val);
                break;
        case TEGRA_CAMERA_CID_COARSE_TIME:
                err = imx378_set_coarse_time(priv, ctrl->val);
                break;
        case TEGRA_CAMERA_CID_COARSE_TIME_SHORT:
                err = imx378_set_coarse_time(priv, ctrl->val);
                break;
        case TEGRA_CAMERA_CID_GROUP_HOLD:
                break;
        case TEGRA_CAMERA_CID_HDR_EN:
                break;
        default:
                pr_err("%s: unknown ctrl id.\n", __func__);
                return -EINVAL;
        }

        return err;
}

static int imx378_ctrls_init(struct imx378 *priv)
{
        struct i2c_client *client = priv->i2c_client;
        struct v4l2_ctrl *ctrl;
        int num_ctrls;
        int err;
        int i;

        dev_dbg(&client->dev, "%s++\n", __func__);

        num_ctrls = ARRAY_SIZE(ctrl_config_list);
        v4l2_ctrl_handler_init(&priv->ctrl_handler, num_ctrls);

        for (i = 0; i < num_ctrls; i++) {
                ctrl = v4l2_ctrl_new_custom(&priv->ctrl_handler,
                        &ctrl_config_list[i], NULL);
                if (ctrl == NULL) {
                        dev_err(&client->dev, "Failed to init %s ctrl\n",
                                ctrl_config_list[i].name);
                        continue;
                }

                if (ctrl_config_list[i].type == V4L2_CTRL_TYPE_STRING &&
                        ctrl_config_list[i].flags & V4L2_CTRL_FLAG_READ_ONLY) {
                        ctrl->p_new.p_char = devm_kzalloc(&client->dev,
                                ctrl_config_list[i].max + 1, GFP_KERNEL);
                }
                priv->ctrls[i] = ctrl;
        }

        priv->num_ctrls = num_ctrls;
        priv->subdev->ctrl_handler = &priv->ctrl_handler;
        if (priv->ctrl_handler.error) {
                dev_err(&client->dev, "Error %d adding controls\n",
                        priv->ctrl_handler.error);
                err = priv->ctrl_handler.error;
                goto error;
        }

        err = v4l2_ctrl_handler_setup(&priv->ctrl_handler);
        if (err) {
                dev_err(&client->dev,
                        "Error %d setting default controls\n", err);
                goto error;
        }

        return 0;

error:
        v4l2_ctrl_handler_free(&priv->ctrl_handler);
        return err;
}

MODULE_DEVICE_TABLE(of, imx378_of_match);

static struct camera_common_pdata *imx378_parse_dt(struct i2c_client *client,
                                const struct camera_common_data *s_data)
{
        struct device_node *node = client->dev.of_node;
        struct camera_common_pdata *board_priv_pdata;
        const struct of_device_id *match;
        int err;

        if (!node)
                return NULL;

        match = of_match_device(imx378_of_match, &client->dev);
        if (!match) {
                dev_err(&client->dev, " Failed to find matching dt id\n");
                return NULL;
        }

        board_priv_pdata = devm_kzalloc(&client->dev,
                           sizeof(*board_priv_pdata), GFP_KERNEL);
        if (!board_priv_pdata) {
                dev_err(&client->dev, "Failed to allocate pdata\n");
                return NULL;
        }


        err = camera_common_parse_clocks(&client->dev, board_priv_pdata);
        if (err) {
                dev_err(&client->dev, "Failed to find clocks\n");
                goto error;
        }

        err = of_property_read_string(node, "mclk",
                                      &board_priv_pdata->mclk_name);
        if (err)
                dev_err(&client->dev, "mclk not in DT\n");

        board_priv_pdata->reset_gpio = of_get_named_gpio(node,
                        "reset-gpios", 0);

        of_property_read_string(node, "avdd-reg",
                        &board_priv_pdata->regulators.avdd);
        of_property_read_string(node, "dvdd-reg",
                        &board_priv_pdata->regulators.dvdd);
        of_property_read_string(node, "iovdd-reg",
                        &board_priv_pdata->regulators.iovdd);

        return board_priv_pdata;

error:
        devm_kfree(&client->dev, board_priv_pdata);
        return NULL;
}

static int imx378_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        dev_dbg(&client->dev, "%s:\n", __func__);


        return 0;
}

static const struct v4l2_subdev_internal_ops imx378_subdev_internal_ops = {
        .open = imx378_open,
};

static const struct media_entity_operations imx378_media_ops = {
#ifdef CONFIG_MEDIA_CONTROLLER
        .link_validate = v4l2_subdev_link_validate,
#endif
};

static int imx378_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
        struct camera_common_data *common_data;
        struct device_node *node = client->dev.of_node;
        struct imx378 *priv;
        char debugfs_name[10];
        int err;

        pr_info("[IMX378]: probing v4l2 sensor.\n");

        if (!IS_ENABLED(CONFIG_OF) || !node)
                return -EINVAL;

        common_data = devm_kzalloc(&client->dev,
                            sizeof(struct camera_common_data), GFP_KERNEL);
        if (!common_data) {
                dev_err(&client->dev, "unable to allocate memory!\n");
                return -ENOMEM;
        }

        priv = devm_kzalloc(&client->dev,
                            sizeof(struct imx378) + sizeof(struct v4l2_ctrl *) *
                            ARRAY_SIZE(ctrl_config_list),
                            GFP_KERNEL);
        if (!priv) {
                dev_err(&client->dev, "unable to allocate memory!\n");
                return -ENOMEM;
        }

        priv->regmap = devm_regmap_init_i2c(client, &sensor_regmap_config);
        if (IS_ERR(priv->regmap)) {
                dev_err(&client->dev,
                        "regmap init failed: %ld\n", PTR_ERR(priv->regmap));
                return -ENODEV;
        }

        priv->pdata = imx378_parse_dt(client, common_data);
        if (!priv->pdata) {
                dev_err(&client->dev, " unable to get platform data\n");
                return -EFAULT;
        }

        common_data->ops                = &imx378_common_ops;
        common_data->ctrl_handler       = &priv->ctrl_handler;
        common_data->dev                = &client->dev;
        common_data->frmfmt             = &imx378_frmfmt[0];
        common_data->colorfmt           = camera_common_find_datafmt(
                                          IMX378_DEFAULT_DATAFMT);
        common_data->power              = &priv->power;
        common_data->ctrls              = priv->ctrls;
        common_data->priv               = (void *)priv;
        common_data->numctrls           = ARRAY_SIZE(ctrl_config_list);
        common_data->numfmts            = ARRAY_SIZE(imx378_frmfmt);
        common_data->def_mode           = IMX378_DEFAULT_MODE;
        common_data->def_width          = IMX378_DEFAULT_WIDTH;
        common_data->def_height         = IMX378_DEFAULT_HEIGHT;
        common_data->fmt_width          = common_data->def_width;
        common_data->fmt_height         = common_data->def_height;
        common_data->def_clk_freq       = IMX378_DEFAULT_CLK_FREQ;

        priv->i2c_client                = client;
        priv->s_data                    = common_data;
        priv->subdev                    = &common_data->subdev;
        priv->subdev->dev               = &client->dev;

        err = imx378_power_get(priv);
        if (err)
                return err;

        err = camera_common_parse_ports(&client->dev, common_data);
        if (err) {
                dev_err(&client->dev, "Failed to find port info\n");
                return err;
        }
    snprintf(debugfs_name, sizeof(debugfs_name), "%s.%s",
            dev_driver_string(&client->dev), dev_name(&client->dev));
        dev_dbg(&client->dev, "%s: name %s\n", __func__, debugfs_name);

        camera_common_create_debugfs(common_data, debugfs_name);

        v4l2_i2c_subdev_init(priv->subdev, client, &imx378_subdev_ops);

        err = imx378_ctrls_init(priv);
        if (err)
                return err;

        err = imx378_verify_streaming(priv);
        if (err)
                return err;

        priv->subdev->internal_ops = &imx378_subdev_internal_ops;
        priv->subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE |
                               V4L2_SUBDEV_FL_HAS_EVENTS;

#if defined(CONFIG_MEDIA_CONTROLLER)
        priv->pad.flags = MEDIA_PAD_FL_SOURCE;
        priv->subdev->entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
        priv->subdev->entity.ops = &imx378_media_ops;
        err = media_entity_init(&priv->subdev->entity, 1, &priv->pad, 0);
        if (err < 0) {
                dev_err(&client->dev, "unable to init media entity\n");
                return err;
        }
#endif

        err = v4l2_async_register_subdev(priv->subdev);
        if (err)
                return err;

        dev_dbg(&client->dev, "Detected IMX378 sensor\n");

        return 0;
}

static int
imx378_remove(struct i2c_client *client)
{
        struct camera_common_data *s_data = to_camera_common_data(&client->dev);
        struct imx378 *priv = (struct imx378 *)s_data->priv;

        v4l2_async_unregister_subdev(priv->subdev);
#if defined(CONFIG_MEDIA_CONTROLLER)
        media_entity_cleanup(&priv->subdev->entity);
#endif

        v4l2_ctrl_handler_free(&priv->ctrl_handler);
        imx378_power_put(priv);
        camera_common_remove_debugfs(s_data);

        return 0;
}

static const struct i2c_device_id imx378_id[] = {
        { "imx378", 0 },
        { }
};

MODULE_DEVICE_TABLE(i2c, imx378_id);

static struct i2c_driver imx378_i2c_driver = {
        .driver = {
                .name = "imx378",
                .owner = THIS_MODULE,
        },
        .probe = imx378_probe,
        .remove = imx378_remove,
        .id_table = imx378_id,
};

module_i2c_driver(imx378_i2c_driver);

MODULE_DESCRIPTION("Media Controller driver for Sony IMX378");
MODULE_AUTHOR("ca-qa@centuryarks.com");
MODULE_LICENSE("GPL v2");
