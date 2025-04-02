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
#include <fstream>
#include <vector>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <opus/opus.h>
#include <speex/speex_resampler.h>  // 新增重采样头文件

typedef struct opus_encoder {
    unsigned int inputSampleRate;
    unsigned int inputChannels;
    unsigned int outputSampleRate;
    unsigned int outputChannels;
    unsigned int duration_ms;
    SpeexResamplerState* resampler;
    OpusEncoder* encoder;
} opus_encoder;

typedef struct opus_decoder {
    int inputSampleRate;
    int inputChannels;
    int outputSampleRate;
    int outputChannels;
    int duration_ms;
    SpeexResamplerState* resampler;
    OpusDecoder* decoder;
} opus_decoder;

static opus_encoder g_opus_encoder;
static opus_decoder g_opus_decoder;

int init_opus_encoder(unsigned int inputSampleRate, unsigned int inputChannels, unsigned int duration_ms, 
                     unsigned int outputSampleRate, unsigned int outputChannels) {
    // 设置全局配置结构体
    g_opus_encoder.inputSampleRate = inputSampleRate;
    g_opus_encoder.inputChannels = inputChannels;
    g_opus_encoder.duration_ms = duration_ms;
    g_opus_encoder.outputSampleRate = outputSampleRate;
    g_opus_encoder.outputChannels = outputChannels;

    int resampleErr;
    g_opus_encoder.resampler = speex_resampler_init(
        g_opus_encoder.outputChannels,
        g_opus_encoder.inputSampleRate,
        g_opus_encoder.outputSampleRate,
        SPEEX_RESAMPLER_QUALITY_DEFAULT,
        &resampleErr
    );

    if (resampleErr != RESAMPLER_ERR_SUCCESS) {
        std::cerr << "重采样器初始化失败 for encoder: " << resampleErr << std::endl;
        return -1;
    }

    int opusError;
    g_opus_encoder.encoder = opus_encoder_create(
        g_opus_encoder.outputSampleRate,
        g_opus_encoder.outputChannels,
        OPUS_APPLICATION_AUDIO,
        &opusError
    );
    if (opusError != OPUS_OK) {
        std::cerr << "编码器初始化失败: " << opus_strerror(opusError) << std::endl;
        speex_resampler_destroy(g_opus_encoder.resampler);
        return -1;
    }
    opus_encoder_ctl(g_opus_encoder.encoder, OPUS_SET_BITRATE(64000));

    return 0;
}

int init_opus_decoder(int inputSampleRate, int inputChannels, int duration_ms, 
                       int outputSampleRate, int outputChannels) {
    // 设置全局配置结构体
    g_opus_decoder.inputSampleRate = inputSampleRate;
    g_opus_decoder.inputChannels = inputChannels;
    g_opus_decoder.duration_ms = duration_ms;
    g_opus_decoder.outputSampleRate = outputSampleRate;
    g_opus_decoder.outputChannels = outputChannels;

    int resampleErr;
    g_opus_decoder.resampler = speex_resampler_init(
        g_opus_decoder.inputChannels,
        g_opus_decoder.inputSampleRate,
        g_opus_decoder.outputSampleRate,
        SPEEX_RESAMPLER_QUALITY_DEFAULT,
        &resampleErr
    );

    if (resampleErr != RESAMPLER_ERR_SUCCESS) {
        std::cerr << "重采样器初始化失败 for decoder: " << resampleErr <<" inputSampleRate "<< g_opus_decoder.inputSampleRate << "  "<< g_opus_decoder.outputSampleRate << "inputChannels "<< g_opus_decoder.inputChannels<< std::endl;
        return -1;
    }

    // 初始化 Opus 解码器
    int error;
    OpusDecoder* decoder = opus_decoder_create(g_opus_decoder.inputSampleRate, g_opus_decoder.inputChannels, &error);
    if (error != OPUS_OK) {
        std::cerr << "解码器初始化失败: " << opus_strerror(error) << std::endl;
        return -1;
    }    
    g_opus_decoder.decoder = decoder;
    return 0;
}

int pcm2opus(unsigned char* pcmdata, int pcmsize, unsigned char* opusdata, int* opussize) {
    // 使用全局配置结构体中的参数
    int sampleRate = g_opus_encoder.inputSampleRate;
    int inputChannels = g_opus_encoder.inputChannels;
    int outsampleRate = g_opus_encoder.outputSampleRate;
    int outputChannels = g_opus_encoder.outputChannels;

    const int originalFrameSize = sampleRate * g_opus_encoder.duration_ms / 1000;  // 原始帧大小
    const int targetFrameSize = outsampleRate * g_opus_encoder.duration_ms / 1000; // 目标帧大小

    // 输入缓冲区（多声道）
    std::vector<opus_int16> rawFrame(originalFrameSize * inputChannels);
    // 中间缓冲区（多声道原始采样率）
    std::vector<opus_int16> pcmFrame(originalFrameSize * inputChannels);
    // 输出缓冲区（多声道目标采样率）
    std::vector<opus_int16> resampledFrame(targetFrameSize * outputChannels);

    // Opus 数据缓冲区
    std::vector<unsigned char> opusFrame(4000);

    // 逐帧处理
    int frameCount = 0;
    size_t totalBytesRead = 0;
    int totalEncodedBytes = 0; // 用于累加编码后的总字节数
    while (totalBytesRead < pcmsize) {
        // 读取原始 PCM 数据
        size_t bytesRead = std::min(static_cast<size_t>(originalFrameSize * inputChannels * sizeof(opus_int16)), static_cast<size_t>(pcmsize - totalBytesRead));
        //printf("bytesRead=%d,originalFrameSize=%d,pcmsize=%d\n", bytesRead, originalFrameSize, pcmsize);
        //printf("%s %d\n", __FUNCTION__, __LINE__);
        memcpy(rawFrame.data(), pcmdata + totalBytesRead, bytesRead);
        totalBytesRead += bytesRead;

        //printf("%s %d\n", __FUNCTION__, __LINE__);

        // 处理不完整帧
        if (bytesRead < rawFrame.size() * sizeof(opus_int16)) {
            size_t samplesRead = bytesRead / sizeof(opus_int16);
            std::fill(rawFrame.begin() + samplesRead, rawFrame.end(), 0);
        }

        //printf("%s %d\n", __FUNCTION__, __LINE__);

        // 根据目标通道数处理音频数据
        if (outputChannels == 1 && inputChannels > 1) {
            // 多声道转单声道
            for (int i = 0; i < originalFrameSize; ++i) {
                opus_int32 sum = 0;
                for (int c = 0; c < inputChannels; ++c) {
                    sum += rawFrame[i * inputChannels + c];
                }
                pcmFrame[i] = static_cast<opus_int16>(sum / inputChannels);
            }
        } else if (outputChannels == inputChannels) {
            // 通道数相同，直接使用原始数据
            memcpy(pcmFrame.data(), rawFrame.data(), originalFrameSize * inputChannels * sizeof(opus_int16));
        } else {
            // 通道数不同且不为单声道，需要进行通道数转换
            // 这里简单地将每个通道的数据复制到目标通道
            // 实际应用中可能需要更复杂的通道映射
            for (int i = 0; i < originalFrameSize; ++i) {
                for (int c = 0; c < outputChannels; ++c) {
                    pcmFrame[i * outputChannels + c] = rawFrame[i * inputChannels + (c % inputChannels)];
                }
            }
        }
        //printf("%s %d\n", __FUNCTION__, __LINE__);
        // 执行重采样
        spx_uint32_t in_len = originalFrameSize;
        spx_uint32_t out_len = targetFrameSize;
        int resampleErr = speex_resampler_process_int(
            g_opus_encoder.resampler,
            0,
            pcmFrame.data(),
            &in_len,
            resampledFrame.data(),
            &out_len
        );
        //printf("%s %d\n", __FUNCTION__, __LINE__);
        if (resampleErr != RESAMPLER_ERR_SUCCESS) {
            std::cerr << "重采样失败: " << resampleErr << std::endl;
            continue;
        }
        //printf("%s %d\n", __FUNCTION__, __LINE__);
        // 检查重采样结果
        if (in_len != originalFrameSize || out_len != targetFrameSize) {
            std::cerr << "重采样样本数不匹配" << std::endl;
            continue;
        }
        //printf("%s %d\n", __FUNCTION__, __LINE__);
        // 编码 Opus 帧
        int encodedBytes = opus_encode(
            g_opus_encoder.encoder,
            resampledFrame.data(),
            targetFrameSize,
            opusFrame.data(),
            opusFrame.size()
        );
        //printf("%s %d\n", __FUNCTION__, __LINE__);
        if (encodedBytes < 0) {
            std::cerr << "帧 " << frameCount << " 编码失败: " << opus_strerror(encodedBytes) << std::endl;
            continue;
        }
        //printf("%s %d\n", __FUNCTION__, __LINE__);
        // 检查 Opus 数据缓冲区是否足够
        //if (totalEncodedBytes + encodedBytes > *opussize) {
        //    std::cerr << "Opus 数据缓冲区不足" << std::endl;
        //    break;
        //}

        // 将编码后的 Opus 数据复制到 opusdata 缓冲区
        //printf("totalEncodedBytes = %d\n", totalEncodedBytes);
        memcpy(opusdata + totalEncodedBytes, opusFrame.data(), encodedBytes);
        totalEncodedBytes += encodedBytes;
        //printf("%s %d\n", __FUNCTION__, __LINE__);
        frameCount++;
    }

    // 更新 opussize 为实际编码的数据大小
    *opussize = totalEncodedBytes;

    return frameCount * targetFrameSize;
}

int opus2pcm(unsigned char* opusdata, int opussize, unsigned char* pcmdata, int *pcmsize) {
    // 计算最大可能的 PCM 数据大小
    int maxFrameSize = 480; // Opus 最大帧大小为 120 ms，假设 48 kHz 采样率
    int maxPcmSize = maxFrameSize * g_opus_decoder.inputChannels * sizeof(opus_int16);
    std::vector<opus_int16> pcmFrame(maxPcmSize);

    // 计算目标 PCM 数据大小
    int targetFrameSize = g_opus_decoder.outputSampleRate * g_opus_decoder.duration_ms / 1000;
    int targetPcmSize = targetFrameSize * g_opus_decoder.outputChannels * sizeof(opus_int16);
    std::vector<opus_int16> resampledFrame(targetPcmSize);

    // 逐帧解码
    int totalBytesRead = 0;
    int totalPcmBytes = 0;
    while (totalBytesRead < opussize) {
        // 计算当前帧的大小
        size_t frameSize = std::min(static_cast<size_t>(maxFrameSize * sizeof(opus_int16) * g_opus_decoder.inputChannels), static_cast<size_t>(opussize - totalBytesRead));

        // 解码 Opus 帧
        int decodedSamples = opus_decode(g_opus_decoder.decoder, opusdata + totalBytesRead, frameSize, pcmFrame.data(), maxPcmSize, 0);
        if (decodedSamples < 0) {
            std::cerr << "帧 " << totalBytesRead / frameSize + 1 << " 解码失败: " << opus_strerror(decodedSamples) << std::endl;
            return -1;
        }

        // 计算解码后的 PCM 数据大小
        int decodedBytes = decodedSamples * sizeof(opus_int16) * g_opus_decoder.inputChannels;

        // 执行重采样
        spx_uint32_t in_len = decodedSamples;
        spx_uint32_t out_len = targetFrameSize;
        int resampleErr = speex_resampler_process_int(
            g_opus_decoder.resampler,
            0,
            pcmFrame.data(),
            &in_len,
            resampledFrame.data(),
            &out_len
        );

        if (resampleErr != RESAMPLER_ERR_SUCCESS) {
            std::cerr << "重采样失败: " << resampleErr << std::endl;
            return -1;
        }

        // 检查重采样结果
        if (in_len != decodedSamples || out_len != targetFrameSize) {
            std::cerr << "重采样样本数不匹配" << std::endl;
            return -1;
        }

        // 处理通道数不同的情况
        std::vector<opus_int16> finalPcmFrame(targetFrameSize * g_opus_decoder.outputChannels);
        if (g_opus_decoder.outputChannels == 1 && g_opus_decoder.inputChannels > 1) {
            // 多声道转单声道
            for (int i = 0; i < targetFrameSize; ++i) {
                opus_int32 sum = 0;
                for (int c = 0; c < g_opus_decoder.inputChannels; ++c) {
                    sum += resampledFrame[i * g_opus_decoder.inputChannels + c];
                }
                finalPcmFrame[i] = static_cast<opus_int16>(sum / g_opus_decoder.inputChannels);
            }
        } else if (g_opus_decoder.outputChannels == g_opus_decoder.inputChannels) {
            // 通道数相同，直接使用重采样后的数据
            memcpy(finalPcmFrame.data(), resampledFrame.data(), targetFrameSize * g_opus_decoder.outputChannels * sizeof(opus_int16));
        } else {
            // 通道数不同且不为单声道，需要进行通道数转换
            // 这里简单地将每个通道的数据复制到目标通道
            // 实际应用中可能需要更复杂的通道映射
            for (int i = 0; i < targetFrameSize; ++i) {
                for (int c = 0; c < g_opus_decoder.outputChannels; ++c) {
                    finalPcmFrame[i * g_opus_decoder.outputChannels + c] = resampledFrame[i * g_opus_decoder.inputChannels + (c % g_opus_decoder.inputChannels)];
                }
            }
        }

        // 计算最终 PCM 数据大小
        int finalPcmBytes = targetFrameSize * g_opus_decoder.outputChannels * sizeof(opus_int16);

        // 检查 PCM 数据缓冲区是否足够
        //if (totalPcmBytes + finalPcmBytes > *pcmsize) {
        //    std::cerr << "PCM 数据缓冲区不足" << std::endl;
        //    return -1;
        //}

        // 将处理后的 PCM 数据复制到 pcmdata 缓冲区
        memcpy(pcmdata + totalPcmBytes, finalPcmFrame.data(), finalPcmBytes);
        totalPcmBytes += finalPcmBytes;
        totalBytesRead += frameSize;
    }

    // 更新 pcmsize 为实际解码的数据大小
    *pcmsize = totalPcmBytes;

    return 0;
}

#ifdef TEST

// WAV 文件头结构
#pragma pack(push, 1)
struct WavHeader {
    char riff[4];       // "RIFF"
    uint32_t fileSize;  // 文件大小 - 8
    char wave[4];       // "WAVE"
};
#pragma pack(pop)

// WAV 块头结构
#pragma pack(push, 1)
struct WavChunkHeader {
    char id[4];       // 块标识符
    uint32_t size;    // 块大小
};
#pragma pack(pop)

int pcm2opus_main(int argc, char* argv[]) {

    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    const char* wavFilePath = argv[2];
    std::ifstream wavFile(wavFilePath, std::ios::binary);
    if (!wavFile) {
        std::cerr << "无法打开文件: " << wavFilePath << std::endl;
        return 1;
    }

    // 读取 WAV 文件头
    WavHeader header;
    wavFile.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));
    if (wavFile.fail() || std::memcmp(header.riff, "RIFF", 4) != 0 || std::memcmp(header.wave, "WAVE", 4) != 0) {
        std::cerr << "无效的 WAV 文件" << std::endl;
        return 1;
    }

    // 查找 fmt 块
    bool fmtFound = false;
    uint32_t dataSize = 0;
    while (!fmtFound) {
        WavChunkHeader chunkHeader;
        wavFile.read(reinterpret_cast<char*>(&chunkHeader), sizeof(WavChunkHeader));
        if (wavFile.fail()) {
            std::cerr << "读取 WAV 块头失败" << std::endl;
            return 1;
        }

        if (std::memcmp(chunkHeader.id, "fmt ", 4) == 0) {
            fmtFound = true;
            uint32_t fmtSize = chunkHeader.size;
            if (fmtSize < 16) {
                std::cerr << "无效的 fmt 块大小" << std::endl;
                return 1;
            }

            wavFile.read(reinterpret_cast<char*>(&audioFormat), sizeof(audioFormat));
            wavFile.read(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));
            wavFile.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));
            wavFile.read(reinterpret_cast<char*>(&byteRate), sizeof(byteRate));
            wavFile.read(reinterpret_cast<char*>(&blockAlign), sizeof(blockAlign));
            wavFile.read(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));

            // 打印 WAV 文件信息
            std::cout << "WAV 文件信息:" << std::endl;
            std::cout << "  采样率: " << sampleRate << std::endl;
            std::cout << "  通道数: " << numChannels << std::endl;
            std::cout << "  每样本位数: " << bitsPerSample << std::endl;

            // 跳过额外的格式信息
            if (fmtSize > 16) {
                wavFile.seekg(fmtSize - 16, std::ios::cur);
            }
        } else {
            // 跳过当前块
            wavFile.seekg(chunkHeader.size, std::ios::cur);
        }
    }

    // 查找 data 块
    bool dataFound = false;
    while (!dataFound) {
        WavChunkHeader chunkHeader;
        wavFile.read(reinterpret_cast<char*>(&chunkHeader), sizeof(WavChunkHeader));
        if (wavFile.fail()) {
            std::cerr << "读取 WAV 块头失败" << std::endl;
            return 1;
        }

        if (std::memcmp(chunkHeader.id, "data", 4) == 0) {
            dataFound = true;
            dataSize = chunkHeader.size;
        } else {
            // 跳过当前块
            wavFile.seekg(chunkHeader.size, std::ios::cur);
        }
    }

    // 初始化 Opus 编码器
    int outputSampleRate = 16000;
    int outputChannels = 1;
    int duration_ms = 60;

    std::cout << "原始采样率: " << sampleRate << " 通道数: "<< numChannels << std::endl;
    std::cout << "目标采样率: " << outputSampleRate << " 通道数: "<< outputChannels << std::endl;
    
    if (init_opus_encoder(sampleRate, numChannels, duration_ms, outputSampleRate, outputChannels) != 0) {
        std::cerr << "Opus 编码器初始化失败" << std::endl;
        return 1;
    }

    // 读取 PCM 数据
    std::vector<opus_int16> pcmData(dataSize / sizeof(opus_int16));
    wavFile.read(reinterpret_cast<char*>(pcmData.data()), dataSize);
    if (wavFile.fail()) {
        std::cerr << "读取 PCM 数据失败" << std::endl;
        return 1;
    }

    // 编码 PCM 数据为 Opus 码流
    int frameCount = 0;
    size_t totalBytesRead = 0;
    while (totalBytesRead < dataSize) {
        // 计算当前帧的大小
        size_t frameSize = std::min(static_cast<size_t>(sampleRate * duration_ms / 1000 * numChannels * sizeof(opus_int16)), dataSize - totalBytesRead);

        // 分配 Opus 数据缓冲区
        unsigned char opusData[4000];
        int opusSize = sizeof(opusData);

        // 编码当前帧
        int encodedBytes = pcm2opus(reinterpret_cast<unsigned char*>(pcmData.data()) + totalBytesRead, frameSize, opusData, &opusSize);
        if (encodedBytes < 0) {
            std::cerr << "编码帧失败" << std::endl;
            break;
        }

        // 生成文件名
        std::ostringstream filename;
        filename << "test" << std::setw(3) << std::setfill('0') << (frameCount + 1) << ".opus";

        // 写入文件
        std::ofstream frameFile(filename.str(), std::ios::binary);
        frameFile.write(reinterpret_cast<char*>(opusData), opusSize);
        frameFile.close();

        frameCount++;
        totalBytesRead += frameSize;
    }

    // 清理资源
    speex_resampler_destroy(g_opus_encoder.resampler);
    opus_encoder_destroy(g_opus_encoder.encoder);

    std::cout << "转换完成，共生成 " << frameCount << " 个帧文件" << std::endl;
    return 0;
}


// 写入 WAV 文件头
void writeWavHeader(std::ofstream& wavFile, int sampleRate, int channels, int pcmSize) {
    const int bitsPerSample = 16;
    const int byteRate = sampleRate * channels * bitsPerSample / 8;
    const int blockAlign = channels * bitsPerSample / 8;

    char riff[4] = {'R', 'I', 'F', 'F'};
    char wave[4] = {'W', 'A', 'V', 'E'};
    char fmt[4] = {'f', 'm', 't', ' '};
    char data[4] = {'d', 'a', 't', 'a'};

    int fileSize = 36 + pcmSize;
    int chunkSize = 16;

    wavFile.write(riff, 4);
    wavFile.write(reinterpret_cast<const char*>(&fileSize), 4);
    wavFile.write(wave, 4);
    wavFile.write(fmt, 4);
    wavFile.write(reinterpret_cast<const char*>(&chunkSize), 4);

    short audioFormat = 1; // PCM
    short numChannels = channels;

    wavFile.write(reinterpret_cast<const char*>(&audioFormat), 2);
    wavFile.write(reinterpret_cast<const char*>(&numChannels), 2);
    wavFile.write(reinterpret_cast<const char*>(&sampleRate), 4);
    wavFile.write(reinterpret_cast<const char*>(&byteRate), 4);
    wavFile.write(reinterpret_cast<const char*>(&blockAlign), 2);
    wavFile.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    int dataSize = pcmSize;

    wavFile.write(data, 4);
    wavFile.write(reinterpret_cast<const char*>(&dataSize), 4);
}

// 主函数
int opus2pcm_main(int argc, char* argv[]) {
    std::string outputFilename = argv[2];
    std::ofstream wavFile(outputFilename, std::ios::binary);
    if (!wavFile) {
        std::cerr << "无法打开输出文件: " << outputFilename << std::endl;
        return 1;
    }

    // 初始化 Opus 解码器
    int inputSampleRate = 16000; // 假设输入采样率为 16kHz
    int inputChannels = 1; // 假设输入通道数为 1
    int duration_ms = 60;
    int outputSampleRate = 16000; // 假设输出采样率为 16kHz
    int outputChannels = 1; // 假设输出通道数为 1

    if (opus_decoder_init(inputSampleRate, inputChannels, duration_ms, outputSampleRate, outputChannels) != 0) {
        std::cerr << "Opus 解码器初始化失败" << std::endl;
        return 1;
    }

    std::vector<opus_int16> allPcmData;
    int totalPcmSize = 0;

    for (int fileIndex = 1; ; ++fileIndex) {
        char filename[20];
        snprintf(filename, sizeof(filename), "test%03d.opus", fileIndex);

        std::ifstream opusFile(filename, std::ios::binary);
        if (!opusFile) {
            break; // 没有更多文件，退出循环
        }

        opusFile.seekg(0, std::ios::end);
        int opusSize = opusFile.tellg();
        opusFile.seekg(0, std::ios::beg);

        std::vector<unsigned char> opusData(opusSize);
        opusFile.read(reinterpret_cast<char*>(opusData.data()), opusSize);

        int maxPcmSize = 480 * outputChannels * sizeof(opus_int16); // 假设最大帧大小为 480 样本
        std::vector<opus_int16> pcmData(maxPcmSize);

        int pcmSize = maxPcmSize;
        int result = opus2pcm(opusData.data(), opusSize, reinterpret_cast<unsigned char*>(pcmData.data()), &pcmSize);
        if (result < 0) {
            std::cerr << "文件 " << filename << " 解码失败" << std::endl;
            continue;
        }

        allPcmData.insert(allPcmData.end(), pcmData.begin(), pcmData.begin() + pcmSize / sizeof(opus_int16));
        totalPcmSize += pcmSize;
    }

    // 写入 WAV 文件头
    writeWavHeader(wavFile, outputSampleRate, outputChannels, totalPcmSize);

    // 写入 PCM 数据
    wavFile.write(reinterpret_cast<char*>(allPcmData.data()), totalPcmSize);

    // 清理资源
    speex_resampler_destroy(g_opus_decoder.resampler);
    opus_decoder_destroy(g_opus_decoder.decoder);

    wavFile.close();

    std::cout << "转换完成，共生成 " << totalPcmSize << " 字节的 PCM 数据" << std::endl;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <opus2wav | wav2opus> <wavfile>" << std::endl;
        return 1;
    }

    if (argv[1][0] == 'o' || argv[1][0] == 'O')
        return opus2pcm_main(argc, argv);
    else
        return pcm2opus_main(argc, argv);
}

#endif // TEST
