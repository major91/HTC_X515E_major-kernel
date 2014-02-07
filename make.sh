#!/bin/sh
#make kernel tool
export PATH=$(pwd)/linaro_4.8.3/bin:$PATH
export ARCH=arm 
export SUBARCH=arm
export CROSS_COMPILE=/home/major/kernel/linaro_4.8.3/bin/arm-cortex_a8-linux-gnueabi-
make major_defconfig
make -j4
