cmake_minimum_required(VERSION 3.10)
project(lvgl)

set(CMAKE_C_STANDARD 99)#C99 # lvgl officially support C99 and above
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

 #CROSS_COMPILE Path
#set(CMAKE_C_COMPILER "/home/ubuntu/tina-v853-100ask/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc/toolchain/bin/arm-openwrt-linux-gcc")
 #Link LibsPath
link_directories(
                /home/ubuntu/tina-v853-100ask/out/v853-100ask/staging_dir/target/usr/lib
		/home/ubuntu/tina-v853-100ask/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc/toolchain/arm-openwrt-linux-gnueabi/lib
)

aux_source_directory(src SOURCE)

include_directories(/home/ubuntu/tina-v853-100ask/out/v853-100ask/staging_dir/target/usr/include/)
include_directories(/home/ubuntu/tina-v853-100ask/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc/toolchain/arm-openwrt-linux-gnueabi/include)



add_subdirectory(lv_100ask_xz_ai)
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})


add_executable(main main.c mouse_cursor_icon.c)


include_directories(${CMAKE_SOURCE_DIR}/lv_100ask_xz_ai)

target_link_libraries(main lv_100ask_xz_ai  lvgl lvgl::examples lvgl::demos lvgl::thorvg  m pthread freetype png)
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/lv_xiaozhiui DEPENDS main)

