set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(tools "/home/ubuntu/k230_linux_sdk/output/k230_canmv_dongshanpi_defconfig/host")
set(CMAKE_C_COMPILER ${tools}/bin/riscv64-unknown-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/riscv64-unknown-linux-gnu-g++)
