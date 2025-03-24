# xiaozhi-linux-SupportList

设计参考来源：[https://github.com/78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32 )  

Linux 小智源代码：https://github.com/100askTeam/xiaozhi-linux/tree/master

## 项目介绍

目前项目主要参考虾哥 AI小智，我们在Linux平台进行完整支持，并将其支持在全志T113s3 Openwrt系统中。

项目主要实现：

1. Audio处理
2. WebSocket对接
3. LVGL UI 开发

仓库源码介绍：

```
├── BSP  ===============================》 Tina4 SDK 补丁包
│   ├── device
│   ├── lichee
│   ├── package
│   ├── scripts
│   └── target
├── externPackage  =====================》 需要外编的库
│   ├── boost_1_69_0.tar.bz2
│   └── websocketpp-0.8.1.tar.gz
├── app  ===============================》 可执行程序
│   ├── control_center
│   ├── gui
│   └── sound_app
└── README.md
```

- BSP：需要拷贝至Tina4 SDK中的文件。
- externPackage：需要重新指定工具链路径外编的库。
- app：Linux小智应用程序。

## 使用教程

在开始前，请须知：本工程基于全志T113s3 Tina4 版SDK，使用的工具链版本为 `glibc`。

**1.获取本仓库源码**

```bash
cd ~
git clone -b 100ASK_T113s3-Pro_Tina4 https://github.com/100askTeam/xiaozhi-linux-SupportList.git
```

**2.将BSP目录下的文件拷贝至Tina SDK中**

```bash
cp ~/xiaozhi-linux-SupportList/BSP/* ~/tina-d1-h -rfvd
```

**3 编译Boost库**

> Boost源码可访问：https://archives.boost.io/release/1.69.0/source/

进入bootst源码目录

```bash
tar -xjf ~/xiaozhi-linux-SupportList/externPackage/boost_1_69_0.tar.bz2
cd ~/xiaozhi-linux-SupportList/externPackage/boost_1_69_0
```

生成编译配置文件

```bash
./bootstrap.sh --with-libraries=all --with-toolset=gcc
```

修改`project-config.jam`中的路径，重新指定为您SDK中glibc具体的路径。例如：

```bash
using gcc : arm : /home/ubuntu/tina-d1-h/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc/toolchain/bin/arm-openwrt-linux-gcc
```

编译源码

```bash
./b2
```

安装boost库

```bash
mkdir tmp
./b2 install --prefix=/home/ubuntu/xiaozhi-linux-SupportList/externPackage/boost_1_69_0/tmp
```

安装完成后可以在boost_arm目录下看到boost库文件和头文件。

> 后续需要在xiaozhi-linux源码中指定boost库和头文件路径，并需要把库文件拷贝至开发板中使用。

**4.编译websocketpp库**

> websocketpp源码可访问：https://github.com/zaphoyd/websocketpp

修改`toolchain.cmake`文件，修改工具链、boost库、openssl等路径为您具体的路径。

进入websocketpp-0.8.1源码目录

~~~bash
tar -xjf ~/xiaozhi-linux-SupportList/externPackage/websocketpp-0.8.1.tar.gz
cd ~/xiaozhi-linux-SupportList/externPackage/websocketpp-0.8.1
~~~

指定交叉编译工具链

~~~bash
vim toolchain.cmake
~~~

填入以下内容：

~~~bash
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR arm)

SET(CMAKE_C_COMPILER /home/ubuntu/tina-d1-h/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc/toolchain/bin/arm-openwrt-linux-gcc)
SET(CMAKE_CXX_COMPILER /home/ubuntu/tina-d1-h/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc/toolchain/bin/arm-openwrt-linux-g++)
SET(CMAKE_FIND_ROOT_PATH /home/ubuntu/tina-d1-h/out/t113-100ask/staging_dir/target)

SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_CXX_FLAGS "-std=c++11")

SET(BOOST_ROOT /home/ubuntu/Downloads/xiaozhi/boost_1_69_0/tmp)
SET(BOOST_INCLUDEDIR /home/ubuntu/Downloads/xiaozhi/boost_1_69_0/tmp/include)
SET(BOOST_LIBRARYDIR /home/ubuntu/Downloads/xiaozhi/boost_1_69_0/tmp/lib)

SET(OPENSSL_INCLUDE_DIR /home/ubuntu/tina-d1-h/out/t113-100ask/staging_dir/target/usr/include)
SET(OPENSSL_LIBRARIES /home/ubuntu/tina-d1-h/out/t113-100ask/staging_dir/target/usr/lib)
add_definitions(-DBOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT)
~~~

新建build的目录

```bash
mkdir build
```

编译源码

```bash
cd build
export STAGING_DIR=/home/ubuntu/tina-d1-h/out/t113-100ask/staging_dir/target
cmake .. \
-DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
-DCMAKE_INSTALL_PREFIX=/home/ubuntu/Downloads/xiaozhi/websocketpp-0.8.1/build/wsbsocketpp-arm
```

编译和安装源码

```bash
make
make install
```

安装完成后可以在build/websocketpp-arm目录下可以找到对应的头文件。

**4.编译xiaozhi-linux源码**

> 注意：如果xiaozhi-linux源码更新，请注意根据实际的编译规则文件路径进行调整！

获取xiaozhi-linux源码：

~~~bash
cd ~
git clone https://github.com/100askTeam/xiaozhi-linux.git
~~~

编译：

1.修改`control_center/Makefile`重新指定编译器和库路径，指定完成后重新`make`后可在当前目录下看到生成的可执行程序control_center。

2.修改`gui/lv_100ask_xiaozhi/`目录下的CMakeLists.txt和toolchain.cmake重新指定编译器和库路径，指定完成后执行`./build.sh`后可在bin目录下看到生成的可执行程序lvgl_xiaozhi。

3.修改`sound_app/Makefile`重新指定编译器和库路径，指定完成后重新`make`后可在当前目录下看到生成的可执行程序sound_app。

## 端侧运行程序

**1.开发板配网**

参考：[WiFi联网](https://docs.100ask.net/dshanpi/docs/T113s3-Pro/part2/WifiNetworking)

2.通过网络同步时间

```bash
ntpdate ntp.aliyun.com
```

3.运行程序

```bash
sound_app &
lvgl_xiaozhi &
control_center
```

> 注意**首次运行**需要登陆激活设备，需要登录[小智 AI 聊天机器人](https://xiaozhi.me/)的`控制台`中新建智能体，并在`管理设备`中添加设备号（一般为六位数字），设备号可在显示屏或者串口上显示。