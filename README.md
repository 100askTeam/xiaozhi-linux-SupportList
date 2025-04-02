# xiaozhi-linux-SupportList

设计参考来源：https://github.com/78/xiaozhi-esp32 ，此版本为支持嘉楠 Buildroot Linux系列平台，SDK版本为V0.6。



## 项目介绍

目前项目主要参考虾哥 AI小智，我们在Linux平台进行完整支持，并将其支持在嘉楠 K230 Buildroot系统中。

项目主要实现：

1. Audio处理
2. WebSocket对接
3. LVGL UI 开发

仓库源码介绍：

```
├── BSP
│   └── k230_canmv_dongshanpi_defconfig
├── LICENSE
├── xiaozhi-linux
│   ├── control_center
│   ├── gui
│   └── sound_app
└── README.md
```



- BSP：参考的Buildroot配置文件
- externPackage：需要重新指定工具链路径外编的库
- xiaozhi-linux：[xiaozhi-linux]([100askTeam/xiaozhi-linux: Embedded Linux AI Xiaozhi Intelligent voice dialogue.](https://github.com/100askTeam/xiaozhi-linux))的源码参考工程，可参考其中的Makefile和Cmake文件，指定其SDK中的工具链和库以及外编的库等。

## 使用教程



在开始前，请须知：本工程基于嘉楠 Buildroot 版Linux SDK，使用的工具链版本为 `glibc`。

**1.获取仓库源码**

```
https://github.com/100askTeam/xiaozhi-linux-SupportList.git
```



**2.参考BSP目录下的deconfig文件编译对应的依赖库**



**2.编译xiaozhi-linux源码**

> 注意：如果xiaozhi-linux源码更新，请注意根据实际的编译规则文件路径进行调整！

1.修改`control_center/Makefile`重新指定编译器和库路径，指定完成后重新`make`后可在当前目录下看到生成的可执行程序control_center。

2.修改`gui/lv_100ask_xiaozhi/`目录下的CMakeLists.txt和toolchain.cmake重新指定编译器和库路径，指定完成后执行`./build.sh`后可在bin目录下看到生成的可执行程序lvgl_xiaozhi。

3.修改`sound_app/Makefile`重新指定编译器和库路径，指定完成后重新`make`后可在当前目录下看到生成的可执行程序sound_app。

## 端侧运行程序

在开始前请先提前将MIPI显示屏和耳机（扬声器）连接至开发板端。可参考开发板的[《快速启动》](https://eai.100ask.net/CanaanK230/part1/DshanPICanMVV2quickstart)；并将固件参考[《更新EMMC系统》](https://eai.100ask.net/CanaanK230/part1/UpdateEMMCsystem)更新板载的系统，更新完成后可在拓展排针处将UART0连接至USB转TTL串口模块，即可访问开发板的串口终端。

注意：CanMV小智固件默认为Linux系统，系统默认的串口打印为UART0。



1.系统启动后，需手动连接网络，需通过USB口连接 `USB转网口模块` 或` USB WIFI模块`。

- 对于USB转网口模块，将开发板连接USB模块并将网线连接至模块后，可在串口终端输入：

  ```
  ifconfig eth0 up
  udhcpc -i eth0
  ```

  输入完成后即可使开发板正常上网。

- 对于USB转WIFI模块，将模块插入开发板中后，可在串口终端输入：

  ```
  ifconfig wlan0 up
  ```

  后续可按照[《开发板配网》](https://eai.100ask.net/CanaanK230/part1/DevelopmentBoardDistributionNetwork)连接2.4GHz的WIFI。

2.在保证开发板已经连接网络之后，可开始运行小智相关的程序，在终端输入：

```
sound_app &
lvgl_xiaozhi &
control_center
```



> 注意**首次运行**需要登陆激活设备，需要登录[小智 AI 聊天机器人](https://xiaozhi.me/)的`控制台`中登录之后新建智能体，并在`管理设备`中添加设备号（一般为六位数字），设备号可在显示屏或者串口上显示。

