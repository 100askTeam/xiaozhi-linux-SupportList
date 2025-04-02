#ifndef __UI_SYSTEM_H__
#define __UI_SYSTEM_H__ 

/*
 * 初始化UI系统。
 * 创建IPC端点，用于接收和处理UI数据。
 *
 * @return 0 表示成功，-1 表示创建IPC端点失败
 */
int ui_system_init(void);

#endif