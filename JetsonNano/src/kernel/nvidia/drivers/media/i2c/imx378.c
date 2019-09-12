/*
 * imx378.c - imx378 sensor driver
 *
 * Copyright (c) 2019, CenturyArks All rights reserved.
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

#include <media/tegra_v4l2_camera.h>
#include <media/tegracam_core.h>
#include <media/imx378.h>

#include "../platform/tegra/camera/camera_gpio.h"
#include "imx378_mode_tbls.h"

/* imx378 - sensor parameter limits */
#define IMX378_MAX_COARSE_DIFF		  20

#define IMX378_GAIN_SHIFT			   8
#define IMX378_MIN_GAIN					(1 << IMX378_GAIN_SHIFT)
#define IMX378_MAX_GAIN					(22 << IMX378_GAIN_SHIFT)
#define IMX378_MIN_FRAME_LENGTH			0x0015
#define IMX378_MAX_FRAME_LENGTH			0xFFFF
#define IMX378_MIN_COARSE_EXPOSURE		0x0001
#define IMX378_MAX_COARSE_EXPOSURE		\
	(IMX378_MAX_FRAME_LENGTH-IMX378_MAX_COARSE_DIFF)

static const struct of_device_id imx378_of_match[] = {
	{ .compatible = "nvidia,imx378", },
	{ },
};
MODULE_DEVICE_TABLE(of, imx378_of_match);

static const u32 ctrl_cid_list[] = {
	TEGRA_CAMERA_CID_GAIN,
	TEGRA_CAMERA_CID_EXPOSURE,
	TEGRA_CAMERA_CID_FRAME_RATE,
	TEGRA_CAMERA_CID_SENSOR_MODE_ID,
};

struct imx378 {
	struct i2c_client		*i2c_client;
	struct v4l2_subdev		*subdev;
	u16						fine_integ_time;
	u32						frame_length;
	u32						vmax;
	s32						group_hold_prev;
	bool					group_hold_en;
	struct camera_common_data	*s_data;
	struct tegracam_device		*tc_dev;
};

static const struct regmap_config sensor_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
	.use_single_rw = true,
};

static inline void imx378_get_frame_length_regs(imx378_reg *regs,
	u32 frame_length)
{
	regs->addr = IMX378_FRAME_LENGTH_ADDR_MSB;
	regs->val = (frame_length >> 8) & 0xff;
	(regs + 1)->addr = IMX378_FRAME_LENGTH_ADDR_LSB;
	(regs + 1)->val = (frame_length) & 0xff;
}

static inline void imx378_get_coarse_integ_time_regs(imx378_reg *regs,
	u32 coarse_time)
{
	regs->addr = IMX378_COARSE_TIME_ADDR_MSB;
	regs->val = (coarse_time >> 8) & 0xff;
	(regs + 1)->addr = IMX378_COARSE_TIME_ADDR_LSB;
	(regs + 1)->val = (coarse_time) & 0xff;
}

static inline void imx378_get_coarse_integ_time_short_regs(imx378_reg *regs,
	u32 coarse_time)
{
	regs->addr = IMX378_COARSE_TIME_SHORT_ADDR_MSB;
	regs->val = (coarse_time >> 8) & 0xff;
	(regs + 1)->addr = IMX378_COARSE_TIME_SHORT_ADDR_LSB;
	(regs + 1)->val = (coarse_time) & 0xff;
}

static inline void imx378_get_gain_reg(imx378_reg *regs, u32 gain)
{
	regs->addr = IMX378_GAIN_ADDR_MSB;
	regs->val = (gain >> 8) & 0xff;
	(regs + 1)->addr = IMX378_GAIN_ADDR_LSB;
	(regs + 1)->val = (gain) & 0xff;
}

static inline int imx378_read_reg(struct camera_common_data *s_data,
	u16 addr, u8 *val)
{
	int err = 0;
	u32 reg_val = 0;

	err = regmap_read(s_data->regmap, addr, &reg_val);
	*val = reg_val & 0xff;

	return err;
}

static inline int imx378_write_reg(struct camera_common_data *s_data,
	u16 addr, u8 val)
{
	int err = 0;

	err = regmap_write(s_data->regmap, addr, val);
	if (err)
		dev_err(s_data->dev, "%s: i2c write failed, 0x%x = %x",
			__func__, addr, val);

	return err;
}

static int imx378_write_table(struct imx378 *priv, const imx378_reg table[])
{
	return regmap_util_write_table_8(priv->s_data->regmap, table, NULL, 0,
		IMX378_TABLE_WAIT_MS, IMX378_TABLE_END);
}

static int imx378_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct device *dev = tc_dev->dev;
	int err;
	
	err = imx378_write_reg(s_data,
				IMX378_GROUP_HOLD_ADDR, val);
	if (err) {
		dev_dbg(dev,
			"%s: Group hold control error\n", __func__);
		return err;
	}
	
	return 0;
}

static int imx378_set_gain(struct tegracam_device *tc_dev, s64 val)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct device *dev = s_data->dev;
	const struct sensor_mode_properties *mode =
		&s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];

	int err = 0;
	imx378_reg gain_regs[2];
	u32 gain;
	int i;

	if (val < mode->control_properties.min_gain_val)
		val = mode->control_properties.min_gain_val;
	else if (val > mode->control_properties.max_gain_val)
		val = mode->control_properties.max_gain_val;

	/* translate value (from normalized analog gain) */
	gain = (u32)((1024 * mode->control_properties.gain_factor) / val);
	gain = 1024 - gain;

	if (gain < IMX378_MIN_GAIN)
		gain = IMX378_MAX_GAIN;
	else if (gain > IMX378_MAX_GAIN)
		gain = IMX378_MAX_GAIN;

	dev_dbg(dev, "%s: val: %lld (/%d) [times], gain: %u\n",
		__func__, val, mode->control_properties.gain_factor, gain);

	imx378_get_gain_reg(gain_regs, gain);
	for (i = 0; i < 2; i++) {
		err = imx378_write_reg(s_data, gain_regs[i].addr, gain_regs[i].val);
		if (err) {
			dev_dbg(dev,
				"%s: gain control error\n", __func__);
			return err;
		}
	}
	if (err)
		dev_dbg(dev, "%s: gain control error\n", __func__);

	return 0;
}

static int imx378_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct imx378 *priv = (struct imx378 *)tc_dev->priv;
	struct device *dev = tc_dev->dev;
	const struct sensor_mode_properties *mode =
		&s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];

	int err = 0;
	imx378_reg fl_regs[2];
	u32 frame_length;
	int i;

	frame_length = (u32)(mode->signal_properties.pixel_clock.val *
		(u64)mode->control_properties.framerate_factor /
		mode->image_properties.line_length / val);

	if (frame_length < IMX378_MIN_FRAME_LENGTH)
		frame_length = IMX378_MIN_FRAME_LENGTH;
	else if (frame_length > IMX378_MAX_FRAME_LENGTH)
		frame_length = IMX378_MAX_FRAME_LENGTH;

	dev_dbg(dev,
		"%s: val: %llde-6 [fps], frame_length: %u [lines]\n",
		__func__, val, frame_length);

	imx378_get_frame_length_regs(fl_regs, frame_length);
	for (i = 0; i < 2; i++) {
		err = imx378_write_reg(s_data, fl_regs[i].addr, fl_regs[i].val);
		if (err) {
			dev_dbg(dev,
				"%s: frame_length control error\n", __func__);
			return err;
		}
	}

	priv->frame_length = frame_length;

	return 0;
}

static int imx378_set_exposure(struct tegracam_device *tc_dev, s64 val)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct device *dev = tc_dev->dev;

	int err = 0;
	imx378_reg ct_regs[2];
	u32 coarse_time;
	int i;

	coarse_time = val;

	if (coarse_time < IMX378_MIN_COARSE_EXPOSURE)
		coarse_time = IMX378_MIN_COARSE_EXPOSURE;
	else if (coarse_time > IMX378_MAX_COARSE_EXPOSURE) {
		coarse_time = IMX378_MAX_COARSE_EXPOSURE;
		dev_dbg(dev,
			"%s: exposure limited by frame_length: %d [lines]\n",
			__func__, IMX378_MAX_COARSE_EXPOSURE);
	}

	dev_dbg(dev, "%s: val: %lld [us], coarse_time: %d [lines]\n",
		__func__, val, coarse_time);

	imx378_get_coarse_integ_time_regs(ct_regs, coarse_time);

	for (i = 0; i < 2; i++) {
		err = imx378_write_reg(s_data, ct_regs[i].addr, ct_regs[i].val);
		if (err) {
			dev_dbg(dev,
				"%s: coarse_time control error\n", __func__);
			return err;
		}
	}
	return 0;
}

#if 0
static int imx378_set_exposure_short(struct tegracam_device *tc_dev, s64 val)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct device *dev = tc_dev->dev;
	const struct sensor_mode_properties *mode =
		&s_data->sensor_props.sensor_modes[s_data->mode_prop_idx];

	int err = 0;
	imx378_reg ct_regs[2];
	const s32 max_coarse_time = priv->frame_length - IMX378_MAX_COARSE_DIFF;
	const s32 fine_integ_time_factor = priv->fine_integ_time *
		mode->control_properties.exposure_factor /
		mode->signal_properties.pixel_clock.val;
	u32 coarse_time;
	int i;

	coarse_time = (val - fine_integ_time_factor)
		* mode->signal_properties.pixel_clock.val
		/ mode->control_properties.exposure_factor
		/ mode->image_properties.line_length;

	if (coarse_time < IMX378_MIN_COARSE_EXPOSURE)
		coarse_time = IMX378_MIN_COARSE_EXPOSURE;
	else if (coarse_time > max_coarse_time) {
		coarse_time = max_coarse_time;
		dev_dbg(dev,
			"%s: exposure limited by frame_length: %d [lines]\n",
			__func__, max_coarse_time);
	}

	dev_dbg(dev, "%s: val: %lld [us], coarse_time: %d [lines]\n",
		__func__, val, coarse_time);

	imx378_get_coarse_integ_time_short_regs(ct_regs, coarse_time);

	for (i = 0; i < 2; i++) {
		err = imx378_write_reg(s_data, ct_regs[i].addr, ct_regs[i].val);
		if (err) {
			dev_dbg(dev,
				"%s: coarse_time control error\n", __func__);
			return err;
		}
	}

	return 0;
}
#endif


static struct tegracam_ctrl_ops imx378_ctrl_ops = {
	.numctrls = ARRAY_SIZE(ctrl_cid_list),
	.ctrl_cid_list = ctrl_cid_list,
	.set_gain = imx378_set_gain,
	.set_exposure = imx378_set_exposure,
//	.set_exposure_short = imx378_set_exposure_short,
	.set_frame_rate = imx378_set_frame_rate,
	.set_group_hold = imx378_set_group_hold,
};

static int imx378_power_on(struct camera_common_data *s_data)
{
	int err = 0;
	struct camera_common_power_rail *pw = s_data->power;
	struct camera_common_pdata *pdata = s_data->pdata;
	struct device *dev = s_data->dev;

	dev_dbg(dev, "%s: power on\n", __func__);
	if (pdata && pdata->power_on) {
		err = pdata->power_on(pw);
		if (err)
			dev_err(dev, "%s failed.\n", __func__);
		else
			pw->state = SWITCH_ON;
		return err;
	}

	usleep_range(1, 2);
	if (pw->reset_gpio)
			gpio_set_value(pw->reset_gpio, 1);
	if (pw->pwdn_gpio)
			gpio_set_value(pw->pwdn_gpio, 1);

	usleep_range(9000, 9010);

	pw->state = SWITCH_ON;

	return 0;
}

static int imx378_power_off(struct camera_common_data *s_data)
{
	int err = 0;
	struct camera_common_power_rail *pw = s_data->power;
	struct camera_common_pdata *pdata = s_data->pdata;
	struct device *dev = s_data->dev;

	dev_dbg(dev, "%s: power off\n", __func__);

	if (pdata && pdata->power_off) {
		err = pdata->power_off(pw);
		if (err) {
			dev_err(dev, "%s failed.\n", __func__);
			return err;
		}
	}

	pw->state = SWITCH_OFF;

	return 0;
}

static int imx378_power_put(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;

	if (unlikely(!pw))
		return -EFAULT;

	if (likely(pw->dvdd))
		regulator_put(pw->dvdd);

	if (likely(pw->avdd))
		regulator_put(pw->avdd);

	if (likely(pw->iovdd))
		regulator_put(pw->iovdd);

	pw->dvdd = NULL;
	pw->avdd = NULL;
	pw->iovdd = NULL;

	if (likely(pw->reset_gpio))
		gpio_free(pw->reset_gpio);

	return 0;
}

static int imx378_power_get(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;
	struct camera_common_pdata *pdata = s_data->pdata;
	struct clk *parent;
	int err = 0;

	if (!pdata) {
		dev_err(dev, "pdata missing\n");
		return -EFAULT;
	}

	/* Sensor MCLK (aka. INCK) */
	if (pdata->mclk_name) {
		pw->mclk = devm_clk_get(dev, pdata->mclk_name);
		if (IS_ERR(pw->mclk)) {
			dev_err(dev, "unable to get clock %s\n",
				pdata->mclk_name);
			return PTR_ERR(pw->mclk);
		}

		if (pdata->parentclk_name) {
			parent = devm_clk_get(dev, pdata->parentclk_name);
			if (IS_ERR(parent)) {
				dev_err(dev, "unable to get parent clock %s",
					pdata->parentclk_name);
			} else
				clk_set_parent(pw->mclk, parent);
		}
	}

	/* analog 2.8v */
	if (pdata->regulators.avdd)
		err |= camera_common_regulator_get(dev,
				&pw->avdd, pdata->regulators.avdd);
	/* IO 1.8v */
	if (pdata->regulators.iovdd)
		err |= camera_common_regulator_get(dev,
				&pw->iovdd, pdata->regulators.iovdd);
	/* dig 1.2v */
	if (pdata->regulators.dvdd)
		err |= camera_common_regulator_get(dev,
				&pw->dvdd, pdata->regulators.dvdd);
			
	if (!err){
		pw->reset_gpio = pdata->reset_gpio;
		pw->af_gpio = pdata->af_gpio;
		pw->pwdn_gpio = pdata->pwdn_gpio;
	}

	/* Reset or ENABLE GPIO */
	pw->reset_gpio = pdata->reset_gpio;
	err = gpio_request(pw->reset_gpio, "cam_reset_gpio");
	if (err < 0) {
		dev_err(dev, "%s: unable to request reset_gpio (%d)\n",
			__func__, err);
		goto done;
	}

done:
	pw->state = SWITCH_OFF;

	return err;
}

static struct camera_common_pdata *imx378_parse_dt(
	struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct device_node *np = dev->of_node;
	struct camera_common_pdata *board_priv_pdata;
	const struct of_device_id *match;
	struct camera_common_pdata *ret = NULL;
	int err = 0;
	int gpio;

	if (!np)
		return NULL;

	match = of_match_device(imx378_of_match, dev);
	if (!match) {
		dev_err(dev, "Failed to find matching dt id\n");
		return NULL;
	}

	board_priv_pdata = devm_kzalloc(dev,
		sizeof(*board_priv_pdata), GFP_KERNEL);
	if (!board_priv_pdata)
		return NULL;
		
//	err = camera_common_parse_clocks(dev, board_priv_pdata);
//	if(err){
//		dev_err(dev, "Failed to find clocks\n");
//		goto error;
//	}

	err = of_property_read_string(np, "mclk", &board_priv_pdata->mclk_name);
	if (err)
		dev_dbg(dev, "mclk name not present, "
			"assume sensor driven externally\n");

	gpio = of_get_named_gpio(np, "reset-gpios", 0);
	if (gpio < 0) {
		if (gpio == -EPROBE_DEFER)
			ret = ERR_PTR(-EPROBE_DEFER);
		dev_err(dev, "reset-gpios not found\n");
		goto error;
	}
	board_priv_pdata->reset_gpio = (unsigned int)gpio;

	err = of_property_read_string(np, "avdd-reg",
		&board_priv_pdata->regulators.avdd);
	err |= of_property_read_string(np, "iovdd-reg",
		&board_priv_pdata->regulators.iovdd);
	err |= of_property_read_string(np, "dvdd-reg",
		&board_priv_pdata->regulators.dvdd);
	if (err)
		dev_dbg(dev, "avdd, iovdd and/or dvdd reglrs. not present, "
			"assume sensor powered independently\n");

	board_priv_pdata->has_eeprom =
		of_property_read_bool(np, "has-eeprom");

	return board_priv_pdata;

error:
	devm_kfree(dev, board_priv_pdata);

	return ret;
}

static int imx378_set_mode(struct tegracam_device *tc_dev)
{
	struct imx378 *priv = (struct imx378 *)tegracam_get_privdata(tc_dev);
	struct camera_common_data *s_data = tc_dev->s_data;

	int err = 0;

	err = imx378_write_table(priv, mode_table[IMX378_MODE_STOP_STREAM]);
	if (err)
		return err;

	err = imx378_write_table(priv, mode_table[s_data->mode]);
	if (err)
		return err;

	return 0;
}

static int imx378_start_streaming(struct tegracam_device *tc_dev)
{
	struct imx378 *priv = (struct imx378 *)tegracam_get_privdata(tc_dev);

	return imx378_write_table(priv, mode_table[IMX378_MODE_START_STREAM]);
}

static int imx378_stop_streaming(struct tegracam_device *tc_dev)
{
	int err;
	struct imx378 *priv = (struct imx378 *)tegracam_get_privdata(tc_dev);

	err = imx378_write_table(priv, mode_table[IMX378_MODE_STOP_STREAM]);

	usleep_range(50000, 51000);

	return err;
}

static struct camera_common_sensor_ops imx378_common_ops = {
	.numfrmfmts = ARRAY_SIZE(imx378_frmfmt),
	.frmfmt_table = imx378_frmfmt,
	.power_on = imx378_power_on,
	.power_off = imx378_power_off,
	.write_reg = imx378_write_reg,
	.read_reg = imx378_read_reg,
	.parse_dt = imx378_parse_dt,
	.power_get = imx378_power_get,
	.power_put = imx378_power_put,
	.set_mode = imx378_set_mode,
	.start_streaming = imx378_start_streaming,
	.stop_streaming = imx378_stop_streaming,
};

static int imx378_board_setup(struct imx378 *priv)
{
	struct camera_common_data *s_data = priv->s_data;
	struct camera_common_pdata *pdata = s_data->pdata;
	struct device *dev = s_data->dev;
	u8 reg_val[2];
	int err = 0;

	if (pdata->mclk_name) {
		err = camera_common_mclk_enable(s_data);
		if (err) {
			dev_err(dev, "error turning on mclk (%d)\n", err);
			goto done;
		}
	}

	err = imx378_power_on(s_data);
	if (err) {
		dev_err(dev, "error during power on sensor (%d)\n", err);
		goto err_power_on;
	}

	/* Probe sensor model id registers */
	err = imx378_read_reg(s_data, IMX378_MODEL_ID_ADDR_MSB, &reg_val[0]);
	if (err) {
		dev_err(dev, "%s: error during i2c read probe (%d)\n",
			__func__, err);
		goto err_reg_probe;
	}
	err = imx378_read_reg(s_data, IMX378_MODEL_ID_ADDR_LSB, &reg_val[1]);
	if (err) {
		dev_err(dev, "%s: error during i2c read probe (%d)\n",
			__func__, err);
		goto err_reg_probe;
	}
	if (!((reg_val[0] == 0x03) && reg_val[1] == 0x78))
		dev_err(dev, "%s: invalid sensor model id: %x%x\n",
			__func__, reg_val[0], reg_val[1]);

err_reg_probe:
	imx378_power_off(s_data);

err_power_on:
	if (pdata->mclk_name)
		camera_common_mclk_disable(s_data);

done:
	return err;
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

static int imx378_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct tegracam_device *tc_dev;
	struct imx378 *priv;
	int err;

	dev_dbg(dev, "probing v4l2 sensor at addr 0x%0x\n", client->addr);

	if (!IS_ENABLED(CONFIG_OF) || !client->dev.of_node)
		return -EINVAL;

	priv = devm_kzalloc(dev,
			sizeof(struct imx378), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	tc_dev = devm_kzalloc(dev,
			sizeof(struct tegracam_device), GFP_KERNEL);
	if (!tc_dev)
		return -ENOMEM;

	priv->i2c_client = tc_dev->client = client;
	tc_dev->dev = dev;
	strncpy(tc_dev->name, "imx378", sizeof(tc_dev->name));
	tc_dev->dev_regmap_config = &sensor_regmap_config;
	tc_dev->sensor_ops = &imx378_common_ops;
	tc_dev->v4l2sd_internal_ops = &imx378_subdev_internal_ops;
	tc_dev->tcctrl_ops = &imx378_ctrl_ops;

	err = tegracam_device_register(tc_dev);
	if (err) {
		dev_err(dev, "tegra camera driver registration failed\n");
		return err;
	}
	priv->tc_dev = tc_dev;
	priv->s_data = tc_dev->s_data;
	priv->subdev = &tc_dev->s_data->subdev;
	tegracam_set_privdata(tc_dev, (void *)priv);

	err = imx378_board_setup(priv);
	if (err) {
		dev_err(dev, "board setup failed\n");
		return err;
	}

	err = tegracam_v4l2subdev_register(tc_dev, true);
	if (err) {
		dev_err(dev, "tegra camera subdev registration failed\n");
		return err;
	}

	dev_dbg(dev, "detected imx378 sensor\n");

	return 0;
}

static int imx378_remove(struct i2c_client *client)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct imx378 *priv = (struct imx378 *)s_data->priv;

	tegracam_v4l2subdev_unregister(priv->tc_dev);
	tegracam_device_unregister(priv->tc_dev);

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
		.of_match_table = of_match_ptr(imx378_of_match),
	},
	.probe = imx378_probe,
	.remove = imx378_remove,
	.id_table = imx378_id,
};
module_i2c_driver(imx378_i2c_driver);

MODULE_DESCRIPTION("Media Controller driver for Sony IMX378");
MODULE_AUTHOR("ca-qa@centuryarks.com");
MODULE_LICENSE("GPL v2");
