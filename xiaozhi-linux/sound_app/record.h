#ifndef RECORD_H
#define RECORD_H

#include <stddef.h> // For size_t

// Define the callback function type
typedef void (*audio_record_callback_t)(unsigned char *buffer, size_t size, void *user_data);

/**
 * 创建一个用于录音的线程
 * 
 * 此函数负责初始化录音所需的用户数据和回调函数，并创建一个独立的线程来执行录音操作
 * 
 * @param cb 一个指向音频回调函数的指针，用于处理录音数据
 * @param user_data 一个指向用户定义数据的指针，将传递给回调函数
 * 
 * @return 返回线程ID或NULL
 */
pthread_t create_record_thread(audio_record_callback_t cb, void *user_data);

/**
 * 获取实际录音设置
 * 
 * 此函数用于获取当前系统或设备实际使用的录音设置，包括采样率、声道数和音频格式
 * 这些设置可能与用户设置的期望值不同，因此需要通过此函数来获取实际应用的值
 * 
 * @param sample_rate 指向无符号整数的指针，用于存储实际的采样率
 * @param channels 指向无符号整数的指针，用于存储实际的声道数
 * @param format 指向音频格式的指针，用于存储实际的音频格式
 */
void get_actual_record_settings(unsigned int *sample_rate, unsigned int *channels, snd_pcm_format_t *format);


#endif // RECORD_H