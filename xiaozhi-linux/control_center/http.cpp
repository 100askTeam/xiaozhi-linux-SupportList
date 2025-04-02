// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (c) 2008-2023 100askTeam : Dongshan WEI <weidongshan@100ask.net> 
 * Discourse:  https://forums.100ask.net
 */
 
/*  Copyright (C) 2008-2023 深圳百问网科技有限公司
 *  All rights reserved
 *
 * 免责声明: 百问网编写的文档, 仅供学员学习使用, 可以转发或引用(请保留作者信息),禁止用于商业用途！
 * 免责声明: 百问网编写的程序, 用于商业用途请遵循GPL许可, 百问网不承担任何后果！
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
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <sstream>
#include "json.hpp"

#include "http.h"

// 回调函数，用于处理HTTP响应数据
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * 激活设备
 * 
 * @param pHttpData http连接数据
 * @param code 存储激活码
 * @return 0-已经激活, 1-得到了激活码(等待激活), -1-失败
 */
int active_device(p_http_data_t pHttpData, char *codebuf) {
    int ret = -1;

    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    std::string url = pHttpData->url;
    std::string postFields = pHttpData->post;
    std::string headers = pHttpData->headers;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        // 设置URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // 设置POST数据
        std::cout << "PostFields: " << postFields << std::endl;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        // 设置请求头
        struct curl_slist *curlheaders = NULL;
        try {
            std::cout << "headers: " << headers << std::endl;
            nlohmann::json headerJson = nlohmann::json::parse(headers);
            for (const auto& [key, value] : headerJson.items()) {
                std::string headerLine = key + ": " + value.get<std::string>();
                std::cout << "parsed head: "<< headerLine << std::endl;
                curlheaders = curl_slist_append(curlheaders, headerLine.c_str());
            }
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "Failed to parse headers JSON: " << e.what() << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return 1;
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlheaders);

        // 设置回调函数以处理响应数据
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // 执行请求
        res = curl_easy_perform(curl);

        // 检查请求是否成功
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Response: " << readBuffer << std::endl;

            // 解析响应数据
            /* 
             * 响应数据格式如下: 有activation字段表示未激活,否则表示激活了
                {
                    "mqtt": {
                        "endpoint": "post-cn-apg3xckag01.mqtt.aliyuncs.com",
                        "client_id": "GID_test@@@00_0c_29_bd_43_04",
                        "username": "Signature|LTAI5tF8J3CrdWmRiuTjxHbF|post-cn-apg3xckag01",
                        "password": "ObF5W8laHbuh9Qr5Ok07V9LhLLg=",
                        "publish_topic": "device-server",
                        "subscribe_topic": "devices/00_0c_29_bd_43_04"
                    },
                    "server_time": {
                        "timestamp": 1741940708193,
                        "timezone_offset": 480
                    },
                    "firmware": {
                        "url": ""
                    },
                    "activation": {
                        "code": "600206",
                        "message": "xiaozhi.me\n600206"
                    }
                }            
            */
            try {
                nlohmann::json responseJson = nlohmann::json::parse(readBuffer);
                if (responseJson.contains("activation") && responseJson["activation"].contains("code")) {
                    std::string code = responseJson["activation"]["code"];
                    std::cout << "Activation Code: " << code << std::endl;
                    strncpy(codebuf, code.c_str(), code.size() + 1); // 存储激活码到codebuf
                    ret = 1; // 表示未激活
                } else {
                    std::cerr << "Device has been Activated" << std::endl;
                    ret = 0;
                }
            } catch (const nlohmann::json::parse_error& e) {
                std::cerr << "Failed to parse response JSON: " << e.what() << std::endl;
                ret = -1;
            }
        }

        // 清理请求头
        curl_slist_free_all(curlheaders);

        // 清理
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return ret;
}