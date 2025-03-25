# xiaozhi-linux-SupportList

设计参考来源：[https://github.com/78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32 )  

Linux 小智源代码：https://github.com/100askTeam/xiaozhi-linux/tree/master

## 项目介绍

目前项目主要参考虾哥 AI小智，我们在Linux平台进行完整支持，并将其支持在全志T113s3 tina5 buildroot系统中。

项目主要实现：

1. Audio处理
2. WebSocket对接
3. LVGL UI 开发

仓库源码介绍：

```
├── App
│   ├── control_center
│   ├── gui
│   └── sound_app
├── Bsp
│   ├── buildroot
│   └── device
├── externPackage
│   └── xiaozhi
└── README.md
```

- Bsp：需要拷贝至Tina5 SDK中的文件。
- App：Linux小智应用程序。
- externPackage：gui程序运行需要加载的字库和图片

## 使用教程

在开始前，请须知：本工程基于全志T113s3 Tina5 buildroot 版SDK。

**1.获取本仓库源码**

```bash
cd ~
git clone -b 100ask-T113s3-Pro_Tina5 https://github.com/100askTeam/xiaozhi-linux-SupportList.git
```

**2.将Bsp目录下的文件拷贝至Tina SDK中**

在这之前，默认已编译SDK。（下面需要更换实际的 tina5 sdk 名称，例如这里是 tina5.0-t113）

```bash
cp ~/xiaozhi-linux-SupportList/Bsp/* ~/tina5.0-t113 -rfvd
cd ~/tina5.0-t113/
rm out -rf
./build.sh 
./build.sh pack
```

**4.编译xiaozhi-linux源码**

> 注意：如果xiaozhi-linux源码更新，请注意根据实际的编译规则文件路径进行调整！

获取xiaozhi-linux源码：

~~~bash
cd ~
git clone https://github.com/100askTeam/xiaozhi-linux.git
~~~

设置交叉编译环境变量：（下面需要更换实际交叉编译工具路径）

~~~bash
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-
export PATH=$PATH:/home/ubuntu/tina5.0-t113/out/t113/evb1_auto_nand/buildroot/buildroot/host/bin
~~~

1.编译 control_center：

~~~bash
cd ~/xiaozhi-linux/control_center
make
~~~

2.编译 sound_app：

~~~bash
cd ~/xiaozhi-linux/sound_app
make
~~~

3.编译 gui：

~~~bash
cd ~/xiaozhi-linux/gui
vim toolchain.cmake
~~~

修改交叉编译工具链：（下面需要更换实际交叉编译工具路径）

~~~bash
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(tools "/home/ubuntu/tina5.0-t113/out/t113/evb1_auto_nand/buildroot/buildroot/host")
set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-gnueabi-g++)
~~~

执行编译指令：

~~~bash
./build.sh
~~~

## 端侧运行程序

**1.开发板配网**

~~~bash
insmod /lib/modules/5.4.61/aic8800_bsp.ko
insmod /lib/modules/5.4.61/aic8800_fdrv.ko
wifi_daemon
wifi -o sta
wifi -c wifi名 密码
~~~

**2.通过网络同步时间**

```bash
ntpdate ntp.aliyun.com
```

**3.上传字库和图片至 t113s3 开发板**

在Ubuntu上操作：

~~~bash
adb push ~/xiaozhi-linux-SupportList/externPackage/xiaozhi /usr/share/
~~~

**4.运行程序**

```bash
sound_app &
lvgl_xiaozhi &
control_center
```

> 注意**首次运行**需要登陆激活设备，需要登录[小智 AI 聊天机器人](https://xiaozhi.me/)的`控制台`中新建智能体，并在`管理设备`中添加设备号（一般为六位数字），设备号可在显示屏或者串口上显示。