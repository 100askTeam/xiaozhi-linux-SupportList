set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(tools "/home/ubuntu/tina-v853-100ask/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc/toolchain")
set(CMAKE_C_COMPILER ${tools}/bin/arm-openwrt-linux-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-openwrt-linux-g++)
