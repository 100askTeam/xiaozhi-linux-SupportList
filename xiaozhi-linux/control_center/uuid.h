#ifndef __UUID_H__
#define __UUID_H__

#include <iostream>

/**
 * 获取无线网卡的 MAC 地址
 *
 * 该函数遍历 /sys/class/net/ 目录下的所有网络接口，
 * 查找名称以 "wlan" 或 "wlp" 开头的无线网卡接口，
 * 并读取其对应的 address 文件以获取 MAC 地址。
 *
 * @return 无线网卡的 MAC 地址，如果未找到则返回空字符串
 */
std::string get_wireless_mac_address();

/**
 * 生成 UUID
 *
 * 该函数使用 std::random_device 和 std::mt19937 生成一个随机的 UUID。
 * UUID 的格式为 8-4-4-4-12 的 32 位十六进制数字。
 *
 * @return 生成的 UUID 字符串
 */
std::string generate_uuid();


#endif