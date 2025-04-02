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
#include <stdint.h>
#include <alsa/asoundlib.h>
#include <speex/speex_preprocess.h>
#include <pthread.h>

#include "record.h"

static audio_record_callback_t g_callback = NULL;
static void *g_user_data = NULL;
static unsigned int g_actual_record_sample_rate;
static unsigned int g_actual_record_channels;
static snd_pcm_format_t g_actual_record_format;

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
void get_actual_record_settings(unsigned int *sample_rate, unsigned int *channels, snd_pcm_format_t *format) {
    // 将全局变量g_actual_sample_rate的值赋给sample_rate指向的变量
    *sample_rate = g_actual_record_sample_rate;
    
    // 将全局变量g_actual_channels的值赋给channels指向的变量
    *channels = g_actual_record_channels;
    
    // 将全局变量g_actual_format的值赋给format指向的变量
    *format = g_actual_record_format;
}

// Function to open PCM device for recording
int open_record(const char *device, unsigned int sample_rate, unsigned int channels, snd_pcm_format_t format, unsigned int *actual_sample_rate, unsigned int *actual_channels, snd_pcm_format_t *actual_format, snd_pcm_t **pcm_handle) {
    snd_pcm_hw_params_t *hw_params = NULL;
    int rc;

    // Open PCM device for recording
    rc = snd_pcm_open(pcm_handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "Failed to open PCM device: %s\n", snd_strerror(rc));
        return 1;
    }

    // Allocate and initialize hardware parameters structure
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(*pcm_handle, hw_params);

    // Set hardware parameters
    rc = snd_pcm_hw_params_set_access(*pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    rc |= snd_pcm_hw_params_set_format(*pcm_handle, hw_params, format);
    rc |= snd_pcm_hw_params_set_channels(*pcm_handle, hw_params, channels);
    rc |= snd_pcm_hw_params_set_rate_near(*pcm_handle, hw_params, &sample_rate, 0);
    if (rc < 0) {
        fprintf(stderr, "Failed to set parameters: %s\n", snd_strerror(rc));
        snd_pcm_close(*pcm_handle);
        *pcm_handle = NULL;
        return 1;
    }

    // Apply hardware parameters
    rc = snd_pcm_hw_params(*pcm_handle, hw_params);
    if (rc < 0) {
        fprintf(stderr, "Failed to apply parameters for record: %s, sample_rate = %d, channels = %d\n", snd_strerror(rc), sample_rate, channels);
        snd_pcm_close(*pcm_handle);
        *pcm_handle = NULL;
        return 1;
    }

    // Retrieve and display audio parameters
    snd_pcm_uframes_t frames;
    snd_pcm_hw_params_get_period_size(hw_params, &frames, 0);
    snd_pcm_hw_params_get_rate(hw_params, &sample_rate, 0);
    snd_pcm_hw_params_get_channels(hw_params, &channels);
    snd_pcm_hw_params_get_format(hw_params, &format);

    printf("frames = %d ************\n", frames);
    
    // Set output parameters
    if (actual_sample_rate) *actual_sample_rate = sample_rate;
    if (actual_channels) *actual_channels = channels;
    if (actual_format) *actual_format = format;

    return 0;
}

// Audio recording module
void* record_audio_thread(void* arg) {
    snd_pcm_t *pcm_handle = NULL;
    unsigned char *buffer = NULL;
    int rc;

    //sleep(1);

    const char *device = "default"; // Use the default PCM device
    unsigned int sample_rate = 16000; // 44.1 kHz
    unsigned int channels = 2; // Stereo
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; // 16-bit little-endian

    unsigned int actual_sample_rate;
    unsigned int actual_channels;
    snd_pcm_format_t actual_format;

    // Open PCM device for recording
    int result = open_record(device, sample_rate, channels, format, &actual_sample_rate, &actual_channels, &actual_format, &pcm_handle);
    if (result != 0) {
        fprintf(stderr, "Failed to open PCM device for recording\n");
        return NULL;
    }

    g_actual_record_sample_rate = actual_sample_rate;
    g_actual_record_channels    = actual_channels;
    g_actual_record_format      = actual_format;    

    // Get hardware parameters
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_current(pcm_handle, hw_params);

    // Retrieve period size
    snd_pcm_uframes_t frames;
    snd_pcm_hw_params_get_period_size(hw_params, &frames, 0);

    // Calculate frame size
    size_t frame_size = snd_pcm_format_width(actual_format) / 8;


    printf("Actual recording settings:\n");
    printf("  Sample Rate: %u Hz\n", actual_sample_rate);
    printf("  Channels: %u\n", actual_channels);
    printf("  Bit Depth: %s, actual_format = %d\n", snd_pcm_format_name(actual_format), actual_format);
    printf("  Frames: %lu\n", (unsigned long)frames);
    printf("  Frame Size: %zu\n", frame_size);

    // Allocate PCM data buffer
    buffer = (unsigned char *)malloc(frames * frame_size * actual_channels);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
        return NULL;
    }

    // 降噪
    SpeexPreprocessState *speex_states[2] = {NULL, NULL};
    for (int i = 0; i < channels; i++) {
        speex_states[i] = speex_preprocess_state_init(frames, sample_rate);
        int denoise = 1;
        int noise_suppress = -10;
        speex_preprocess_ctl(speex_states[i], SPEEX_PREPROCESS_SET_DENOISE, &denoise);
        speex_preprocess_ctl(speex_states[i], SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noise_suppress);
    }

    uint16_t *channel_buffers[2];
    for (int i = 0; i < actual_channels; i++) {
        channel_buffers[i] = (uint16_t *)malloc(frames * sizeof(uint16_t));
    }    

    // Recording loop
    printf("Recording started...\n");
    while (1) {
        rc = snd_pcm_readi(pcm_handle, buffer, frames);
        if (rc == -EPIPE) {
            fprintf(stderr, "Buffer overrun detected, recovering...\n");
            snd_pcm_prepare(pcm_handle);
        } else if (rc < 0) {
            fprintf(stderr, "Read error: %s\n", snd_strerror(rc));
            break;
        } else {
#if 0            
            // 降噪
            for (int i = 0; i < rc; i++) {
                for (int ch = 0; ch < actual_channels; ch++) {
                    channel_buffers[ch][i] = buffer[i * actual_channels + ch];
                }
            }
    
            for (int ch = 0; ch < actual_channels; ch++) {
                speex_preprocess_run(speex_states[ch], (spx_int16_t *)channel_buffers[ch]);
            }
    
            for (int i = 0; i < rc; i++) {
                for (int ch = 0; ch < actual_channels; ch++) {
                    buffer[i * actual_channels + ch] = channel_buffers[ch][i];
                }
            }
#endif
            // Call the callback function to return recorded audio data
            //printf("to read %d frame, get frame, rc = %d\n", frames, rc);
            if (g_callback) {
                g_callback(buffer, rc * frame_size * actual_channels, g_user_data);
            }
        }
    }

    // Release resources
    free(buffer);

    // Close the PCM device when done
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

    return NULL;
}

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
pthread_t create_record_thread(audio_record_callback_t cb, void *user_data) {
    // 保存用户数据和回调函数指针到全局变量
    g_user_data = user_data;
    g_callback = cb;

    pthread_t record_thread;
    int err = pthread_create(&record_thread, NULL, record_audio_thread, NULL);
    // 尝试创建录音线程
    if (err) {
        // 如果创建失败，打印错误信息并返回错误代码
        fprintf(stderr, "Failed to create recording thread\n");
        return (pthread_t)0;
    }
    
    // 如果线程成功创建，返回成功代码
    return record_thread;
}

int record_main() {
    // Create a thread for recording
    pthread_t record_thread;
    if (pthread_create(&record_thread, NULL, record_audio_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create recording thread\n");
        return 1;
    }

    // Wait for the recording thread to finish
    pthread_join(record_thread, NULL);

    return 0;
}
