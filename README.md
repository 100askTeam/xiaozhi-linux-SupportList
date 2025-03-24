# xiaozhi-linux-SupportList

设计参考来源：https://github.com/78/xiaozhi-esp32，此版本为支持全志Tina V85x系列平台，SDK版本为Tina 4。

演示视频：https://mp.weixin.qq.com/s/sC72kgotz9s2IEcYaz0BZg

## 项目介绍

目前项目主要参考虾哥 AI小智，我们在Linux平台进行完整支持，其中有幸联系到虾哥本人，非常感谢在调试中给予我们建议方向，我们才能以最短的时间支持完成。

项目主要实现：

1. Audio处理
2. WebSocket对接
3. LVGL UI 开发

仓库源码介绍：

```
├── BSP
│   ├── build
│   ├── device
│   └── target
├── externPackage
│   ├── boost_1_69_0
│   └── websocketpp-0.8.1
├── LICENSE
├── xiaozhi-linux
│   ├── control_center
│   ├── gui
│   └── sound_app
└── README.md
```

- BSP：需要拷贝至Tina4 SDK中的文件，拷贝并替换原本的SDK文件
- externPackage：需要重新指定工具链路径外编的库
- xiaozhi-linux：[xiaozhi-linux]([100askTeam/xiaozhi-linux: Embedded Linux AI Xiaozhi Intelligent voice dialogue.](https://github.com/100askTeam/xiaozhi-linux))的源码参考工程，可参考其中的Makefile和Cmake文件，指定其SDK中的工具链和库以及外编的库等。



## 使用教程

在开始前，请须知：本工程基于全志V85x Tina4 版SDK，使用的工具链版本为 `glibc`。

**1.获取仓库源码**

```
https://github.com/100askTeam/xiaozhi-linux-SupportList.git
```



**2.将BSP目录下的文件拷贝至Tina SDK中**

```
cp xiaozhi-linux-SupportList/BSP/* tina-v853-100ask/ -rf
```



**3 编译Boost库**

> Boost源码可访问：https://archives.boost.io/release/1.69.0/source/

进入bootst源码目录

```
cd xiaozhi-linux-SupportList/externPackage/boost_1_69_0
```



生成编译配置文件

```
./bootstrap.sh --with-libraries=all --with-toolset=gcc
```



修改`project-config.jam`中的路径，重新指定为您SDK中glibc具体的路径。例如：

```
using gcc : arm : /home/ubuntu/tina-v853-100ask/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc/toolchain/bin/arm-openwrt-linux-gcc
```



编译源码

```
./b2
```



安装boost库

```
./b2 install --prefix=./boost_arm
```

安装完成后可以在boost_arm目录下看到boost库文件和头文件。

> 后续需要在xiaozhi-linux源码中指定boost库和头文件路径，并需要把库文件拷贝至开发板中使用。



**4.编译websocketpp库**

> websocketpp源码可访问：https://github.com/zaphoyd/websocketpp

修改`toolchain.cmake`文件，修改工具链、boost库、openssl等路径为您具体的路径。



新建build的目录

```
mkdir build
```



编译源码

```
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -DCMAKE_INSTALL_PREFIX=./websocketpp-arm
```



编译和安装源码

```
make
make install
```

安装完成后可以在build/websocketpp-arm目录下可以找到对应的头文件。



**4.编译xiaozhi-linux源码**

> 注意：如果xiaozhi-linux源码更新，请注意根据实际的编译规则文件路径进行调整！

1.修改`control_center/Makefile`重新指定编译器和库路径，指定完成后重新`make`后可在当前目录下看到生成的可执行程序control_center。



2.修改`gui/lv_100ask_xiaozhi/`目录下的CMakeLists.txt和toolchain.cmake重新指定编译器和库路径，指定完成后执行`./build.sh`后可在bin目录下看到生成的可执行程序lvgl_xiaozhi。



3.修改`sound_app/Makefile`重新指定编译器和库路径，指定完成后重新`make`后可在当前目录下看到生成的可执行程序sound_app。



## 端侧运行程序

**1.开发板配网**

在终端输入`wifi -c <WIFI名称> <WIFI密码>`，假设我需要连接的WIFI名称为test，密码为：123456789，则配网命令为：

```
wifi -c test 123456789
```

> 注意：只能连接2.4GHz的WIFI



2.通过网络同步时间

```
ntpdate ntp.aliyun.com
```



3.运行程序

```
sound_app &
lvgl_xiaozhi &
control_center
```

> 注意**首次运行**需要登陆激活设备，需要登录[小智 AI 聊天机器人](https://xiaozhi.me/)的`控制台`中新建智能体，并在`管理设备`中添加设备号（一般为六位数字），设备号可在显示屏或者串口上显示。

