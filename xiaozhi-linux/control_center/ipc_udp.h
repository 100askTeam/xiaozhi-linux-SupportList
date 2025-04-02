#ifndef TRANSFER_CENTER_H
#define TRANSFER_CENTER_H

#include <cstddef> // 添加此行以定义 size_t

typedef int (*transfer_callback_t)(char *buffer, size_t size, void *user_data);

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

/**
 * 传输的各方被称为endpoint
 * 数据结构体，包含套接字、端口、服务器地址和回调函数
 */
typedef struct ipc_endpoint_t {  
    void *priv;
    void *user_data;
    transfer_callback_t cb;  // 接收到远端的客户端发来的信息后使用它来处理
    int (*send)(struct ipc_endpoint_t *self, const char *data, int len); // 发送数据的函数指针
    int (*recv)(struct ipc_endpoint_t *self, unsigned char *data, int maxlen, int *retlen); // 接收数据的函数指针
} ipc_endpoint_t, *p_ipc_endpoint_t;

// 创建一个UDP类型的IPC端点
// 参数:
//   port_local: 本地端口号
//   port_remote: 远程端口号
//   cb: 数据传输回调函数
//   user_data: 用户数据，将传递给回调函数
// 返回值:
//   成功: 返回IPC端点指针
//   失败: 返回NULL
p_ipc_endpoint_t ipc_endpoint_create_udp(int port_local, int port_remote, transfer_callback_t cb, void *user_data);

// 销毁IPC端点，释放相关资源
void ipc_endpoint_destroy_udp(p_ipc_endpoint_t pendpoint);

#endif // TRANSFER_H