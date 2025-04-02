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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "ipc_udp.h"

// 定义UDP数据结构体
typedef struct upd_data_t {
    int socket_send;       // 发送数据的套接字
    int port_remote;         // 目标端口号
    int socket_recv;       // 接收数据的套接字
    int port_local;          // 源端口号
    struct sockaddr_in remote_addr;  // 目标地址结构体
}upd_data_t, *p_upd_data_t;

// 线程处理函数声明
static void* handle_udp_connection(void* arg);

// 发送数据的函数声明
static int udp_send_data(ipc_endpoint_t *pendpoint, const char *data, int len);

// 接收数据的函数声明
static int udp_recv_data(ipc_endpoint_t *pendpoint, unsigned char *data, int maxlen, int *retlen);

// 创建一个UDP类型的IPC端点
// 参数:
//   port_local: 本地端口号
//   port_remote: 远程端口号
//   cb: 数据传输回调函数
//   user_data: 用户数据，将传递给回调函数
// 返回值:
//   成功: 返回IPC端点指针
//   失败: 返回NULL
p_ipc_endpoint_t ipc_endpoint_create_udp(int port_local, int port_remote, transfer_callback_t cb, void *user_data)
{
    // 分配并清零UDP数据结构体
    p_upd_data_t pudpdata = (p_upd_data_t)calloc(1, sizeof(upd_data_t));
    // 分配并清零IPC端点结构体
    p_ipc_endpoint_t pendpoint = (p_ipc_endpoint_t)calloc(1, sizeof(ipc_endpoint_t));

    int fd_recv;
    struct sockaddr_in local_addr;
    struct sockaddr_in server_addr;

    if (!pudpdata || !pendpoint) {
        if (pendpoint)free(pendpoint);
        if (pudpdata)free(pudpdata);
        return NULL;            

    }

    // 关联UDP数据结构体和IPC端点结构体
    pendpoint->priv = pudpdata;
    pendpoint->cb = cb;
    pendpoint->user_data = user_data;
    pendpoint->send = udp_send_data;
    pendpoint->recv = udp_recv_data;

    // 设置远程和本地端口号
    pudpdata->port_remote = port_remote;
    pudpdata->port_local = port_local;

    // 1. 为了发送数据进行网络初始化
    // 创建UDP套接字
    int fd_send = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_send < 0) {
        perror("Failed to create UDP socket for audio client");
        if (pendpoint)free(pendpoint);
        if (pudpdata)free(pudpdata);
        return NULL;            
}

    // 初始化服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_remote); // 使用传入的端口号
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        if (pendpoint)free(pendpoint);
        if (pudpdata)free(pudpdata);
        return NULL;            
}

    // 保存套接字和服务器地址信息到UDP数据结构体
    pudpdata->socket_send = fd_send;
    pudpdata->remote_addr = server_addr;    
    
    // 2. 为了接收数据进行网络初始化
    // 创建UDP套接字
    if ((fd_recv = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket");
        if (pendpoint)free(pendpoint);
        if (pudpdata)free(pudpdata);
        return NULL;            
}

    pudpdata->socket_recv = fd_recv;

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port_local);

    if (inet_pton(AF_INET, "127.0.0.1", &local_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        if (pendpoint)free(pendpoint);
        if (pudpdata)free(pudpdata);
        return NULL;            
}

    // 绑定套接字
    if (bind(fd_recv, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Failed to bind socket");
        close(fd_recv);
        if (pendpoint)free(pendpoint);
        if (pudpdata)free(pudpdata);
        return NULL;            
}

    // 如果有回调函数，创建线程处理UDP连接
    if (cb) {
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_udp_connection, pendpoint) != 0) {
            perror("Failed to create thread");

            if (pendpoint)free(pendpoint);
            if (pudpdata)free(pudpdata);
            return NULL;            
        }
    }

    return pendpoint;    
}

// 销毁IPC端点，释放相关资源
void ipc_endpoint_destroy_udp(p_ipc_endpoint_t pendpoint)
{
    free(pendpoint->priv);
    free(pendpoint);
}

/**
 * 处理UDP连接的线程函数
 * 
 * @param arg 指向ipc_endpoint_t结构体的指针
 * @return 线程退出时返回NULL
 */
static void* handle_udp_connection(void* arg)
{
    ipc_endpoint_t *pendpoint = (ipc_endpoint_t*)arg;
    p_upd_data_t pudpdata = (p_upd_data_t)pendpoint->priv;

    int fd_recv = pudpdata->socket_recv;
    char buffer[2048];

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    ssize_t bytes_received;

    printf("Listening on port_local %d\n", pudpdata->port_local);

    while (1) {
        // 接收数据
        bytes_received = recvfrom(fd_recv, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);
        if (bytes_received > 0) {
            // 处理接收到的数据
            if (pendpoint->cb) {
                pendpoint->cb(buffer, bytes_received, pendpoint->user_data);
            }
        }
    }

    pthread_exit(NULL);
}

/**
 * 发送数据到指定endpoint的通用函数
 * 
 * @param pendpoint 指向ipc_endpoint_t结构体的指针
 * @param data 指向要发送的数据的指针
 * @param len 要发送的数据的长度
 * @return 成功返回0，失败返回-1
 */
static int udp_send_data(ipc_endpoint_t *pendpoint, const char *data, int len)
{
    p_upd_data_t pudpdata = (p_upd_data_t)pendpoint->priv;

    // 获取套接字文件描述符
    int fd = pudpdata->socket_send;
    // 获取服务器地址的指针
    struct sockaddr_in *p_server_addr = &pudpdata->remote_addr;

    // 检查套接字是否已初始化
    if (fd < 0) {
        fprintf(stderr, "UDP socket for audio server is not initialized\n");
        return -1;
    }

    // 发送数据到客户端
    ssize_t bytes_sent = sendto(fd, data, len, 0, (struct sockaddr *)p_server_addr, sizeof(*p_server_addr));
    // 检查发送的数据量是否与预期相符
    if (bytes_sent != len) {
        perror("Failed to send data to client");
        return -1;
    }

    // 发送成功
    return 0;
}

/**
 * 接收数据函数
 * 
 * 该函数负责从UDP套接字接收数据，并将其写入指定的缓冲区
 * 
 * @param data 接收到的数据将存储在这个缓冲区中
 * @param maxlen 缓冲区的最大长度
 * @param retlen 实际接收的数据长度
 * 
 * @return 返回接收结果的整数表示，具体含义可能包括成功、失败或特定错误代码
 */
static int udp_recv_data(ipc_endpoint_t *pendpoint, unsigned char *data, int maxlen, int *retlen)
{
    p_upd_data_t pudpdata = (p_upd_data_t)pendpoint->priv;

    // 获取套接字文件描述符
    int fd = pudpdata->socket_recv;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    ssize_t bytes_received;

    if (fd < 0) {
        fprintf(stderr, "UDP socket for audio client is not initialized\n");
        return -1;
    }

    // 接收数据
    bytes_received = recvfrom(fd, data, maxlen, 0, (struct sockaddr *)&client_addr, &client_len);
    if (bytes_received < 0) {
        perror("Failed to receive data from server");
        return -1;
    }

    *retlen = (int)bytes_received;

    return 0;
}

