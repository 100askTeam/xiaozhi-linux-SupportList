#ifndef _HTTP_H_
#define _HTTP_H_

#include <string>

typedef struct http_data_t {
    std::string url;
    std::string post;
    std::string headers;
} http_data_t, *p_http_data_t;

/**
 * 激活设备
 * 
 * @param pHttpData http连接数据
 * @param code 存储激活码
 * @return 0-已经激活, 1-得到了激活码(等待激活), -1-失败
 */
int active_device(p_http_data_t pHttpData, char *codebuf);

#endif