// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (c) 2008-2023 100askTeam : Dongshan WEI <weidongshan@100ask.net> 
 * Discourse:  https://forums.100ask.net
 */
 
/*  Copyright (C) 2008-2023 深圳百问网科技有限公司
 *  All rights reserved
 *
 * 免责声明: 百问网编写的文档, 仅供学员学习使用, 可以转发或引用(请保留作者信息),禁止用于商业用途！
 * 免责声明: 百问网编写的程序, 可以用于商业用途, 但百问网不承担任何后果！
 * 
 * 本程序遵循GPL V3协议, 请遵循协议
 * 百问网学习平台   : https://www.100ask.net
 * 百问网交流社区   : https://forums.100ask.net
 * 百问网官方B站    : https://space.bilibili.com/275908810
 * 本程序所用开发板 : Linux开发板
 * 百问网官方淘宝   : https://100ask.taobao.com
 * 联系我们(E-mail) : weidongshan@100ask.net
 *
 *          版权所有，盗版必究。
 *  
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2025.03.20      v01         百问科技      创建文件
 *-----------------------------------------------------
 */
#include "cfg.h"
#include "ipc_udp.h"
#include "cJSON.h"
#include "lv_100ask_xz_ai_main.h"
#include "lang_config.h"

// 定义设备状态枚举类型
typedef enum DeviceState {
    kDeviceStateUnknown,
    kDeviceStateStarting,
    kDeviceStateWifiConfiguring,
    kDeviceStateIdle,
    kDeviceStateConnecting,
    kDeviceStateListening,
    kDeviceStateSpeaking,
    kDeviceStateUpgrading,
    kDeviceStateActivating,
    kDeviceStateFatalError
} DeviceState;

// 静态变量，用于存储IPC端点
static p_ipc_endpoint_t g_ipc_ep;

// 将设备状态转换为本地字符串
static const char* ConvertToLocalString(DeviceState state)
{
    switch (state) {
        case kDeviceStateUnknown:
            return UNKNOWN_STATUS;
        case kDeviceStateStarting:
            return INITIALIZING;
        case kDeviceStateWifiConfiguring:
            return NETWORK_CFG;
        case kDeviceStateIdle:
            return STANDBY;
        case kDeviceStateConnecting:
            return CONNECTING;
        case kDeviceStateListening:
            return LISTENING;
        case kDeviceStateSpeaking:
            return SPEAKING;
        case kDeviceStateUpgrading:
            return UPGRADING;
        case kDeviceStateActivating:
            return ACTIVATION;
        case kDeviceStateFatalError:
            return ERROR_STR;
    }

    return "未知状态";
}

/*
 * 处理从IPC接收到的UI数据。
 * 处理的数据格式:
 * 1. 状态: {"state": 0}等, 取值对应DeviceState的取值
 * 2. 要显示的文本: {"text": "你好"}
 * 3. 要显示的emotion: {"emotion": "happy"}, 有这些取值:
 *           "neutral","happy","laughing","funny","sad","angry","crying","loving",
 *           "embarrassed","surprised","shocked","thinking","winking","cool","relaxed",
 *           "delicious","kissy","confident","sleepy","silly","confused"
 * 4. WIFI强度: {"wifi": "100"}
 * 5. 电量: {"battery": "100"}
 *
 * @param buffer 包含JSON格式数据的字符串缓冲区
 * @param size 缓冲区的大小
 * @param user_data 用户数据指针（未使用）
 * @return 0 表示成功，-1 表示解析错误
 */
static int process_ui_data(char *buffer, size_t size, void *user_data)
{
    cJSON *json;

    // 解析JSON数据
    json = cJSON_Parse(buffer);
    if (!json) {
        LV_LOG_USER("cJSON_Parse err: %s ", buffer);
        return -1;
    }

    // 获取状态字段
    cJSON *state = cJSON_GetObjectItem(json, "state");
    if (state) {
        LV_LOG_USER("get state = %d, %s ", state->valueint, ConvertToLocalString((DeviceState)state->valueint));
        SetStateString(ConvertToLocalString((DeviceState)state->valueint));
        if (state->valueint == kDeviceStateSpeaking)
            SetEmotion(ASSETS_PATH "img_joke.png");
        if (state->valueint == kDeviceStateListening)
            SetEmotion(ASSETS_PATH "img_naughty.png");
    }

    // 获取文本字段
    cJSON *text = cJSON_GetObjectItem(json, "text");
    if (text) {
        LV_LOG_USER("get text = %s ", text->valuestring);
        SetText(text->valuestring);
    }

    // 获取WIFI强度字段（未处理）
    cJSON *wifi = cJSON_GetObjectItem(json, "wifi");
    if (wifi) {
        // 处理WIFI强度
    }

    // 获取电量字段（未处理）
    cJSON *battery = cJSON_GetObjectItem(json, "battery");
    if (battery) {
        // 处理电量
    }

    // 释放JSON对象
    cJSON_Delete(json);

    return 0;
}

/*
 * 初始化UI系统。
 * 创建IPC端点，用于接收和处理UI数据。
 *
 * @return 0 表示成功，-1 表示创建IPC端点失败
 */
int ui_system_init(void)
{
    // 创建UDP IPC端点
    g_ipc_ep = ipc_endpoint_create_udp(UI_PORT_DOWN, UI_PORT_UP, process_ui_data, NULL);
    if (!g_ipc_ep) {
        LV_LOG_ERROR("Failed to create IPC endpoint\n");
        return -1;
    }
    return 0;
}