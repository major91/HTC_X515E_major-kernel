#!/bin/sh
export KERNELDIR=`readlink -f .`
export RAMFS_SOURCE=`readlink -f $KERNELDIR/ramdisk`
echo "kerneldir = $KERNELDIR"
echo "ramfs_source = $RAMFS_SOURCE"

RAMFS_TMP="/tmp/rider-ramdisk"

echo "ramfs_tmp = $RAMFS_TMP"

. $KERNELDIR/.config

echo "Compiling kernel"
cd $KERNELDIR
make || exit 1

echo "Building new ramdisk"
#remove previous ramfs files
rm -rf '$RAMFS_TMP'*
rm -rf $RAMFS_TMP
rm -rf $RAMFS_TMP.cpio.lz4
#copy ramfs files to tmp directory
cp -ax $RAMFS_SOURCE $RAMFS_TMP
rm *.ko 2>/dev/null
find . -name "*.ko" -exec cp {} . \;
ls *.ko | while read file; do /home/major/kernel/android-toolchain-eabi/bin/arm-eabi-strip --strip-unneeded $file ; done
cd $RAMFS_TMP
find . | fakeroot cpio -H newc -o > $RAMFS_TMP.cpio
$KERNELDIR/lz4 -c0 $RAMFS_TMP.cpio $RAMFS_TMP.cpio.lz4
ls -lh $RAMFS_TMP.cpio.lz4
cd $KERNELDIR

echo "Making new boot image"
./mkbootimg --cmdline 'console=ttyHSL0 androidboot.hardware=rider no_console_suspend=1' --kernel $KERNELDIR/arch/arm/boot/zImage --ramdisk $RAMFS_TMP.cpio.lz4 --base 48000000 --pagesize 2048 -o $KERNELDIR/boot.img

echo "done"
ls -al boot.img
echo ""
ls -al *.ko
