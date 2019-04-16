#!/bin/bash

FNAME=$1
FPS_4K3K=$2
FPS_4K2K=$3
FPS_FHD=$4
FPS_VGA=$5
CAMERA_NUM=$6

source ./scripts/generate_framerate.sh

LANE=("a" "b" "c" "d" "e" "f")
WIDTH=(4056 3840 1920 640)
HEIGHT=(3040 2160 1080 480)
LINE_LENGTH=(${LINE_LENGTH_4K3K} ${LINE_LENGTH_4K2K} ${LINE_LENGTH_FHD} ${LINE_LENGTH_VGA})
MCLK_MULTI=(${PLL_4K3K} ${PLL_4K2K} ${PLL_FHD} ${PLL_VGA})
PIX_CLK=(${PIX_CLK_4K3K} ${PIX_CLK_4K2K} ${PIX_CLK_FHD} ${PIX_CLK_VGA})
FPS=(${FPS_4K3K} ${FPS_4K2K} ${FPS_FHD} ${FPS_VGA})

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
#echo '#include "tegra186-camera-imx378-c03-cu.dtsi"'>>${FNAME}
#echo>>${FNAME}
echo '/ {'>>${FNAME}
echo "	host1x {">>${FNAME}
echo "		vi@15700000 {">>${FNAME}
echo "			num-channels = <${CAMERA_NUM}>;">>${FNAME}
echo "			ports {">>${FNAME}
echo "				#address-cells = <1>;">>${FNAME}
echo "				#size-cells = <0>;">>${FNAME}

ID=0
while :
do
	if [ ${ID} = ${CAMERA_NUM} ]; then
		break
	fi
		echo "				port@${ID} {">>${FNAME}
		echo "					reg = <${ID}>;">>${FNAME}
		echo "					vi_in${ID}: endpoint {">>${FNAME}
		echo "						csi-port = <${ID}>;">>${FNAME}
		echo "						bus-width = <2>;">>${FNAME}
		echo "						remote-endpoint = <&csi_out${ID}>;">>${FNAME}
		echo "					};">>${FNAME}
		echo "				};">>${FNAME}
	ID=`expr ${ID} + 1`
done

echo "			};">>${FNAME}
echo "		};">>${FNAME}
echo>>${FNAME}
echo "		nvcsi@150c0000 {">>${FNAME}
echo "			num-channels = <${CAMERA_NUM}>;">>${FNAME}

ID=0
while :
do
	if [ ${ID} = ${CAMERA_NUM} ]; then
		break
	fi
		EP0=`expr ${ID} \* 2`
		EP1=`expr ${ID} \* 2 + 1`
		echo "			channel@${ID} {">>${FNAME}
		echo "				reg = <${ID}>;">>${FNAME}
		echo "				ports {">>${FNAME}
		echo "					#address-cells = <1>;">>${FNAME}
		echo "					#size-cells = <0>;">>${FNAME}
		echo "					port@0 {">>${FNAME}
		echo "						reg = <0>;">>${FNAME}
		echo "						csi_in${ID}: endpoint@${EP0} {">>${FNAME}
		echo "							csi-port = <${ID}>;">>${FNAME}
		echo "							bus-width = <2>;">>${FNAME}
		echo "							remote-endpoint = <&imx378_out${ID}>;">>${FNAME}
		echo "						};">>${FNAME}
		echo "					};">>${FNAME}
		echo "					port@1 {">>${FNAME}
		echo "						reg = <1>;">>${FNAME}
		echo "						csi_out${ID}: endpoint@${EP1} {">>${FNAME}
		echo "							remote-endpoint = <&vi_in${ID}>;">>${FNAME}
		echo "						};">>${FNAME}
		echo "					};">>${FNAME}
		echo "				};">>${FNAME}
		echo "			};">>${FNAME}
	ID=`expr ${ID} + 1`
done

echo "		};">>${FNAME}
echo "	};">>${FNAME}
echo>>${FNAME}
echo "	i2c@3180000 {">>${FNAME}
echo "		tca9548@70 {">>${FNAME}

ID=0
while :
do
	if [ ${ID} = ${CAMERA_NUM} ]; then
		break
	fi
	echo "			i2c@${ID} {">>${FNAME}
	echo "				imx378_${LANE[${ID}]}@1a {">>${FNAME}
	echo "					devnode = \"video${ID}\";">>${FNAME}
	echo "					compatible = \"nvidia,imx378\";">>${FNAME}
	echo "					reg = <0x1a>;">>${FNAME}
	echo>>${FNAME}
	echo "					physical_w = \"7.564\";">>${FNAME}
	echo "					physical_h = \"5.476\";">>${FNAME}
	echo>>${FNAME}
	echo "					sensor_model =\"imx378\";">>${FNAME}
	echo "					dovdd-supply = <&en_vdd_cam>;">>${FNAME}
	echo "					avdd-reg = \"vana\";">>${FNAME}
	echo "					dvdd-reg = \"vdig\";">>${FNAME}
	echo "					iovdd-reg = \"dovdd\";">>${FNAME}
	echo>>${FNAME}

	MODE=0
	while :
	do
		if [ ${MODE} = ${MODE_NUM} ]; then
			break
		fi
			echo "					mode${MODE} { // IMX378_MODE_${WIDTH[${MODE}]}X${HEIGHT[${MODE}]}">>${FNAME}
			echo "						mclk_khz = \"6000\";">>${FNAME}
			echo "						num_lanes = \"2\";">>${FNAME}
			echo "						tegra_sinterface = \"serial_${LANE[${ID}]}\";">>${FNAME}
			echo "						discontinuous_clk = \"yes\";">>${FNAME}
			echo "						cil_settletime = \"0\";">>${FNAME}
			echo "						active_w = \"${WIDTH[${MODE}]}\";">>${FNAME}
			echo "						active_h = \"${HEIGHT[${MODE}]}\";">>${FNAME}
			echo "						pixel_t = \"bayer_rggb\";">>${FNAME}
			echo "						readout_orientation = \"90\";">>${FNAME}
			echo "						line_length = \"${LINE_LENGTH[${MODE}]}\";">>${FNAME}
			echo "						inherent_gain = \"1\";">>${FNAME}
			echo "						mclk_multiplier = \"${MCLK_MULTI[${MODE}]}\";">>${FNAME}
			echo "						pix_clk_hz = \"${PIX_CLK[${MODE}]}\";">>${FNAME}
			echo>>${FNAME}
			echo "						min_gain_val = \"1.0\";">>${FNAME}
			echo "						max_gain_val = \"22.0\";">>${FNAME}
			echo "						min_hdr_ratio = \"1\";">>${FNAME}
			echo "						max_hdr_ratio = \"64\";">>${FNAME}
			echo "						min_framerate = \"1\";">>${FNAME}
			echo "						max_framerate = \"${FPS[${MODE}]}\";">>${FNAME}
			echo "						min_exp_time = \"1\";">>${FNAME}
			echo "						max_exp_time = \"65515\";">>${FNAME}
#			echo "						embedded_metadata_height = \"2\";">>${FNAME}
			echo "					};">>${FNAME}
		MODE=`expr ${MODE} + 1`
	done
	echo "					ports {">>${FNAME}
	echo "						#address-cells = <1>;">>${FNAME}
	echo "						#size-cells = <0>;">>${FNAME}
	echo "						port@0 {">>${FNAME}
	echo "							reg = <0>;">>${FNAME}
	echo "							imx378_out${ID}: endpoint {">>${FNAME}
	echo "								csi-port = <${ID}>;">>${FNAME}
	echo "								bus-width = <2>;">>${FNAME}
	echo "								remote-endpoint = <&csi_in${ID}>;">>${FNAME}
	echo "							};">>${FNAME}
	echo "						};">>${FNAME}
	echo "					};">>${FNAME}
	echo "				};">>${FNAME}
	echo "			};">>${FNAME}
	ID=`expr ${ID} + 1`
done

echo "		};">>${FNAME}
echo "	};">>${FNAME}
echo>>${FNAME}

ID=0
while :
do
	if [ ${ID} = ${CAMERA_NUM} ]; then
		break
	fi
	echo "	lens_imx378_${LANE[${ID}]}@3e {">>${FNAME}
	echo "		min_focus_distance = \"0.0\";">>${FNAME}
	echo "		hyper_focal = \"0.0\";">>${FNAME}
	echo "		focal_length = \"4.52\";">>${FNAME}
	echo "		f_number = \"2.0\";">>${FNAME}
	echo "		aperture = \"2.0\";">>${FNAME}
	echo "	};">>${FNAME}
	ID=`expr ${ID} + 1`
done

echo>>${FNAME}
echo "	tegra-camera-platform {">>${FNAME}
echo "		compatible = \"nvidia, tegra-camera-platform\";">>${FNAME}
echo "		/**">>${FNAME}
echo "		* Physical settings to calculate max ISO BW">>${FNAME}
echo "		*">>${FNAME}
echo "		* num_csi_lanes = <>;">>${FNAME}
echo "		* Total number of CSI lanes when all cameras are active">>${FNAME}
echo "		*">>${FNAME}
echo "		* max_lane_speed = <>;">>${FNAME}
echo "		* Max lane speed in Kbit/s">>${FNAME}
echo "		*">>${FNAME}
echo "		* min_bits_per_pixel = <>;">>${FNAME}
echo "		* Min bits per pixel">>${FNAME}
echo "		*">>${FNAME}
echo "		* vi_peak_byte_per_pixel = <>;">>${FNAME}
echo "		* Max byte per pixel for the VI ISO case">>${FNAME}
echo "		*">>${FNAME}
echo "		* vi_bw_margin_pct = <>;">>${FNAME}
echo "		* Vi bandwidth margin in percentage">>${FNAME}
echo "		*">>${FNAME}
echo "		* max_pixel_rate = <>;">>${FNAME}
echo "		* Max pixel rate in Kpixel/s for the ISP ISO case">>${FNAME}
echo "		*">>${FNAME}
echo "		* isp_peak_byte_per_pixel = <>;">>${FNAME}
echo "		* Max byte per pixel for the ISP ISO case">>${FNAME}
echo "		*">>${FNAME}
echo "		* isp_bw_margin_pct = <>;">>${FNAME}
echo "		* Isp bandwidth margin in percentage">>${FNAME}
echo "		*/">>${FNAME}
echo "		num_csi_lanes = <12>;">>${FNAME}
echo "		max_lane_speed = <1500000>;">>${FNAME}
echo "		min_bits_per_pixel = <10>;">>${FNAME}
echo "		vi_peak_byte_per_pixel = <2>;">>${FNAME}
echo "		vi_bw_margin_pct = <25>;">>${FNAME}
echo "		max_pixel_rate = <750000>;">>${FNAME}
echo "		isp_peak_byte_per_pixel = <5>;">>${FNAME}
echo "		isp_bw_margin_pct = <25>;">>${FNAME}
echo>>${FNAME}
echo "		/**">>${FNAME}
echo "		 * The general guideline for naming badge_info contains 3 parts, and is as follows,">>${FNAME}
echo "		 * The first part is the camera_board_id for the module; if the module is in a FFD">>${FNAME}
echo "		 * platform, then use the platform name for this part.">>${FNAME}
echo "		 * The second part contains the position of the module, ex. \"rear\" or \"front\".">>${FNAME}
echo "		 * The third part contains the last 6 characters of a part number which is found">>${FNAME}
echo "		 * in the module's specsheet from the vender.">>${FNAME}
echo "		 */">>${FNAME}
echo "		modules {">>${FNAME}

ID=0
while :
do
	if [ ${ID} = ${CAMERA_NUM} ]; then
		break
	fi
	MODULE=`expr 5 \- ${ID}`
	CH=`expr ${ID} + 1`
	echo "			module${MODULE} {">>${FNAME}
	echo "				status = \"okay\";">>${FNAME}
	echo "				badge = \"imx378_CH${CH}_396CP\";">>${FNAME}
	echo "				position = \"${ID}\";">>${FNAME}
	echo "				orientation = \"1\";">>${FNAME}
	echo "				drivernode0 {">>${FNAME}
	echo "					status = \"okay\";">>${FNAME}
	echo "					/* Declare PCL support driver (classically known as guid)  */">>${FNAME}
	echo "					pcl_id = \"v4l2_sensor\";">>${FNAME}
	echo "					/* Driver v4l2 device name */">>${FNAME}
	echo "					devname = \"imx378 3${ID}-001a\";">>${FNAME}
	echo "					/* Declare the device-tree hierarchy to driver instance */">>${FNAME}
	echo "					proc-device-tree = \"/proc/device-tree/i2c@3180000/tca9548@70/i2c@${ID}/imx378_${LANE[${ID}]}@1a\";">>${FNAME}
	echo "				};">>${FNAME}
	echo "				drivernode1 {">>${FNAME}
	echo "					/* Declare PCL support driver (classically known as guid)  */">>${FNAME}
	echo "					pcl_id = \"v4l2_lens\";">>${FNAME}
	echo "					proc-device-tree = \"/proc/device-tree/lens_imx378_${LANE[${ID}]}@3e/\";">>${FNAME}
	echo "				};">>${FNAME}
	echo "			};">>${FNAME}
	ID=`expr ${ID} + 1`
done

echo "		};">>${FNAME}
echo "	};">>${FNAME}
echo "};">>${FNAME}
