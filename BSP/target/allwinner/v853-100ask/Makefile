#
# Copyright (C) 2013-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=v853-100ask
BOARDNAME:=v853-100ask
FEATURES:=fpu usb ext4 display rtc
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon
MAINTAINER:=Allwinner

KERNEL_PATCHVER:=4.9
UBOOT_PATCHVER:=2018
KERNELNAME:=zImage dtbs

include $(BUILD_DIR)/target.mk

DEFAULT_PACKAGES +=
TARGET_CFLAGS += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -DUSE_LOG_LIB_GLOG -Wno-unused-but-set-variable \
                -Wno-unused-variable -Wno-unused-function -Wno-unused-label -Wno-unused-const-variable -Wno-comment \
                -Wno-unused-value
#-DUSE_STD_LOG
TARGET_CXXFLAGS += -std=c++11 -fno-rtti

$(eval $(call BuildTarget))
