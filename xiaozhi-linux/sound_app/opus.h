#ifndef __OPUS_H
#define __OPUS_H

#include <stdint.h>

// 函数声明
/**
 * 初始化 Opus 编码器
 * 
 * @param inputSampleRate 输入采样率
 * @param inputChannels 输入通道数
 * @param duration_ms 帧持续时间（毫秒）
 * @param outputSampleRate 输出采样率
 * @param outputChannels 输出通道数
 * @return 成功返回0，失败返回-1
 */
int init_opus_encoder(unsigned int inputSampleRate, unsigned int inputChannels, unsigned int duration_ms, 
        unsigned int outputSampleRate, unsigned int outputChannels);

/**
 * 初始化 Opus 解码器
 * 
 * @param inputSampleRate 输入采样率
 * @param inputChannels 输入通道数
 * @param duration_ms 帧持续时间（毫秒）
 * @param outputSampleRate 输出采样率
 * @param outputChannels 输出通道数
 * @return 成功返回0，失败返回-1
 */
int init_opus_decoder(int inputSampleRate, int inputChannels, int duration_ms, 
                       int outputSampleRate, int outputChannels);

/**
 * 将 PCM 数据编码为 Opus 数据
 * 
 * @param pcmdata 指向 PCM 数据的指针
 * @param pcmsize PCM 数据的大小（字节）
 * @param opusdata 指向 Opus 数据的指针
 * @param opussize 指向 Opus 数据大小的指针
 * @return 成功返回编码的帧数，失败返回-1
 */
int pcm2opus(unsigned char* pcmdata, int pcmsize, unsigned char* opusdata, int* opussize);

/**
 * 将 Opus 数据解码为 PCM 数据
 * 
 * @param opusdata 指向 Opus 数据的指针
 * @param opussize Opus 数据的大小（字节）
 * @param pcmdata 指向 PCM 数据的指针
 * @param pcmsize 指向 PCM 数据大小的指针
 * @return 成功返回0，失败返回-1
 */
int opus2pcm(unsigned char* opusdata, int opussize, unsigned char* pcmdata, int *pcmsize);

#endif // __OPUS_H