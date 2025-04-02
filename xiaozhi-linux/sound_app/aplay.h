#ifndef APLAY_H
#define APLAY_H

#include <stddef.h> // For size_t

// Define the callback function type
typedef int (*audio_play_callback_t)(unsigned char *buffer, size_t size);

/**
 * 创建播放音频的线程
 * 
 * @param cb 音频回调函数指针，用于播放音频数据
 * @param user_data 用户数据，将传递给回调函数
 * 
 * 此函数负责创建一个用于播放音频的线程它接受一个音频回调函数和一组用户数据作为参数
 * 当线程创建成功时，返回0；如果创建失败，则返回-1
 * 返回线程ID或NULL
 */
pthread_t create_play_thread(audio_play_callback_t cb, void *user_data);

/**
 * 获取实际播放设置
 * 
 * 此函数用于获取当前音频播放的实际设置，包括采样率、声道数和数据格式这些设置信息
 * 通过引用参数传递给调用者
 * 
 * @param sample_rate 指向存储实际采样率的无符号整数变量的指针
 * @param channels 指向存储实际声道数的无符号整数变量的指针
 * @param format 指向存储实际音频数据格式的变量的指针
 */
void get_actual_play_settings(unsigned int *sample_rate, unsigned int *channels, snd_pcm_format_t *format);

#endif // APLAY_H