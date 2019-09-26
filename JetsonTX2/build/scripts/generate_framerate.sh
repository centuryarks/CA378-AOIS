#!/bin/bash

MODE_NUM=4
INCK=6000000
MIPI_LANE=2
RAW_BIT=10
PIX_CLK_VIDEO=840000000

# Table of frames setting
PLL_TBL_4K3K=(348 280 232 348 280 232 280 232)
DIV_TBL_4K3K=(  1   1   1   2   2   2   4   4)
FPS_TBL_4K3K=( 30  24  20  15  12  10   6   5)
NUM_4K3K=${#FPS_TBL_4K3K[@]}

PLL_TBL_4K2K=(238 196 154 238 196 154)
DIV_TBL_4K2K=(  1   1   1   2   2   2)
FPS_TBL_4K2K=( 30  24  20  15  12  10)
NUM_4K2K=${#FPS_TBL_4K2K[@]}

PLL_TBL_FHD=(252 198 168 252 198 168 252)
DIV_TBL_FHD=(  1   1   1   2   2   2   4)
FPS_TBL_FHD=(120  96  80  60  48  40  30)
NUM_FHD=${#FPS_TBL_FHD[@]}

PLL_TBL_VGA=(    168 168 210 280 168)
DIV_TBL_VGA=(      4   2   2   2   1)
FPS_TBL_VGA=(     60 120 150 200 240)
FRAME_V_TBL_VGA=(1050 1050 1050 1050 1050)
NUM_VGA=${#FPS_TBL_VGA[@]}

# Search FPS match
ID=0
while :
do
	if [ ${ID} = ${NUM_4K3K} ]; then
		echo "Set default FPS for 4K3K(${FPS_TBL_4K3K[0]})"
		ID_4K3K=0
		break
	elif [ ${FPS_4K3K} = ${FPS_TBL_4K3K[${ID}]} ]; then
		ID_4K3K=${ID}
		break
	fi
	ID=`expr ${ID} + 1`
done

ID=0
while :
do
	if [ ${ID} = ${NUM_4K2K} ]; then
		echo "Set default FPS for 4K2K(${FPS_TBL_4K2K[0]})"
		ID_4K2K=0
		break
	elif [ ${FPS_4K2K} = ${FPS_TBL_4K2K[${ID}]} ]; then
		ID_4K2K=${ID}
		break
	fi
	ID=`expr ${ID} + 1`
done

ID=0
while :
do
	if [ ${ID} = ${NUM_FHD} ]; then
		echo "Set default FPS for FHD(${FPS_TBL_FHD[0]})"
		ID_FHD=0
		break
	elif [ ${FPS_FHD} = ${FPS_TBL_FHD[${ID}]} ]; then
		ID_FHD=${ID}
		break
	fi
	ID=`expr ${ID} + 1`
done

ID=0
while :
do
	if [ ${ID} = ${NUM_VGA} ]; then
		echo "Set default FPS for VGA(${FPS_TBL_VGA[0]})"
		ID_VGA=0
		break
	elif [ ${FPS_VGA} = ${FPS_TBL_VGA[${ID}]} ]; then
		ID_VGA=${ID}
		break
	fi
	ID=`expr ${ID} + 1`
done

PLL_4K3K=${PLL_TBL_4K3K[${ID_4K3K}]}
DIV_4K3K=${DIV_TBL_4K3K[${ID_4K3K}]}
PIX_CLK_4K3K=`expr ${PLL_4K3K} \* ${INCK} / ${DIV_4K3K} \* ${MIPI_LANE} / ${RAW_BIT}`
FRAME_LENGTH_4K3K=3200
LINE_LENGTH_4K3K=`expr ${PIX_CLK_4K3K} / ${FPS_4K3K} / ${FRAME_LENGTH_4K3K}`
LINE_LENGTH_4K3K_DUAL=`expr ${PIX_CLK_VIDEO} / ${FPS_4K3K} / ${FRAME_LENGTH_4K3K}`

PLL_4K2K=${PLL_TBL_4K2K[${ID_4K2K}]}
DIV_4K2K=${DIV_TBL_4K2K[${ID_4K2K}]}
PIX_CLK_4K2K=`expr ${PLL_4K2K} \* ${INCK} / ${DIV_4K2K} \* ${MIPI_LANE} / ${RAW_BIT}`
FRAME_LENGTH_4K2K=2240
LINE_LENGTH_4K2K=`expr ${PIX_CLK_4K2K} / ${FPS_4K2K} / ${FRAME_LENGTH_4K2K}`
LINE_LENGTH_4K2K_DUAL=`expr ${PIX_CLK_VIDEO} / ${FPS_4K2K} / ${FRAME_LENGTH_4K2K}`

PLL_FHD=${PLL_TBL_FHD[${ID_FHD}]}
DIV_FHD=${DIV_TBL_FHD[${ID_FHD}]}
PIX_CLK_FHD=`expr ${PLL_FHD} \* ${INCK} / ${DIV_FHD} \* ${MIPI_LANE} / ${RAW_BIT}`
FRAME_LENGTH_FHD=1125
LINE_LENGTH_FHD=`expr ${PIX_CLK_FHD} / ${FPS_FHD} / ${FRAME_LENGTH_FHD}`
LINE_LENGTH_FHD_DUAL=`expr ${PIX_CLK_VIDEO} / ${FPS_FHD} / ${FRAME_LENGTH_FHD}`

PLL_VGA=${PLL_TBL_VGA[${ID_VGA}]}
DIV_VGA=${DIV_TBL_VGA[${ID_VGA}]}
PIX_CLK_VGA=`expr ${PLL_VGA} \* ${INCK} / ${DIV_VGA} \* ${MIPI_LANE} / ${RAW_BIT}`
FRAME_LENGTH_VGA=${FRAME_V_TBL_VGA[${ID_VGA}]}
LINE_LENGTH_VGA=`expr ${PIX_CLK_VGA} / ${FPS_VGA} / ${FRAME_LENGTH_VGA}`
LINE_LENGTH_VGA_DUAL=`expr ${PIX_CLK_VIDEO} / ${FPS_VGA} / ${FRAME_LENGTH_VGA}`

# Array of frame settings
FRAME_LENGTH=(${FRAME_LENGTH_4K3K} ${FRAME_LENGTH_4K2K} ${FRAME_LENGTH_FHD} ${FRAME_LENGTH_VGA})
LINE_LENGTH=(${LINE_LENGTH_4K3K} ${LINE_LENGTH_4K2K} ${LINE_LENGTH_FHD} ${LINE_LENGTH_VGA})
FPS=(${FPS_4K3K} ${FPS_4K2K} ${FPS_FHD} ${FPS_VGA})
PLL_MULTI=(${PLL_4K3K} ${PLL_4K2K} ${PLL_FHD} ${PLL_VGA})
SYCK_DIV=(${DIV_4K3K} ${DIV_4K2K} ${DIV_FHD} ${DIV_VGA})

# Analog Gain settings
ISO=(100 100 `expr ${FPS_FHD} \* 100 / 30` `expr  ${FPS_VGA} \* 100 / 30`)
