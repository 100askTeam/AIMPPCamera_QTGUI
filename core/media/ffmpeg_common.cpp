// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2008-2025 100askTeam : Dongshan WEI <weidongshan@100ask.net> 
 * Discourse:  https://forums.100ask.net
 */
 
/*  Copyright (C) 2008-2025 深圳百问网科技有限公司
 *  All rights reserved
 *
 * 免责声明: 百问网编写的文档, 仅供学员学习使用, 可以转发或引用(请保留作者信息),禁止用于商业用途！
 * 免责声明: 百问网编写的程序, 用于商业用途请遵循GPL许可, 百问网不承担任何后果！
 * 
 * 本程序遵循GPL V2协议, 请遵循协议
 * 百问网学习平台   : https://www.100ask.net
 * 百问网交流社区   : https://forums.100ask.net
 * 百问网官方B站    : https://space.bilibili.com/275908810
 * 百问网官方淘宝   : https://100ask.taobao.com
 * 联系我们(E-mail) : weidongshan@100ask.net
 *
 *          版权所有，盗版必究。
 * 
 */
#include "ffmpeg_common.h"
#include <iostream>
#include <stdio.h>
#include <QDebug>

int DecoderInit(std::string url,
                             AVFormatContext **ictx,
                             int *v_index,
                             int *a_index,
                             const AVCodec **_v_codec,
                             const AVCodec **_a_codec,
                             AVCodecContext **_v_decode_ctx,
                             AVCodecContext **_a_decode_ctx)
{

    AVFormatContext *ic = nullptr;
    const AVCodec *v_codec = nullptr;
    const AVCodec *a_codec = nullptr;
    AVCodecContext *v_decode_ctx = nullptr;
    AVCodecContext *a_decode_ctx = nullptr;
    int ret = 0;
    char errbuf[AV_ERROR_MAX_STRING_SIZE]; // 定义错误信息缓冲区

    AVDictionary *options = nullptr;

    // 设置超时参数（单位：微秒）
    // 超时时间2秒 = 2,000,000微秒
    av_dict_set(&options, "stimeout", "2000000", 0);   // 通用超时（部分协议有效）
    av_dict_set(&options, "rw_timeout", "2000000", 0); // 读写超时

    // 针对RTSP协议的额外设置
    av_dict_set(&options, "rtsp_transport", "tcp", 0); // 使用TCP传输（可选）
    av_dict_set(&options, "timeout", "2000000", 0);    // RTSP专用超时

    // 1. 创建解复用上下文并打开输入
    ret = avformat_open_input(&ic, url.c_str(), nullptr, &options);
    if (ret < 0)
    {
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE); // 使用 av_strerror 替代
        av_log(nullptr, AV_LOG_ERROR, "无法打开输入: %s\n", errbuf);
        return ret;
    }

    // 2. 获取流信息
    ret = avformat_find_stream_info(ic, nullptr);
    if (ret < 0)
    {
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        av_log(nullptr, AV_LOG_ERROR, "无法获取流信息: %s\n", errbuf);
        goto fail;
    }

    *ictx = ic;

    av_dump_format(ic, 0, url.c_str(), 0);

    // 3. 查找最佳视频流
    *v_index = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, &v_codec, 0);
    if (*v_index >= 0)
    {
        v_decode_ctx = avcodec_alloc_context3(v_codec);
        if (!v_decode_ctx)
        {
            ret = AVERROR(ENOMEM);
            av_log(nullptr, AV_LOG_ERROR, "无法分配视频解码器上下文\n");
            goto fail;
        }

        ret = avcodec_parameters_to_context(v_decode_ctx, ic->streams[*v_index]->codecpar);
        if (ret < 0)
        {
            av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
            av_log(nullptr, AV_LOG_ERROR, "无法复制视频参数: %s\n", errbuf);
            goto fail;
        }

        // 设置低延迟选项
        av_opt_set(v_decode_ctx->priv_data, "tune", "zerolatency", 0);

        ret = avcodec_open2(v_decode_ctx, v_codec, nullptr);
        if (ret < 0)
        {
            av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
            av_log(nullptr, AV_LOG_ERROR, "无法打开视频解码器: %s\n", errbuf);
            goto fail;
        }

        *_v_codec = v_codec;
        *_v_decode_ctx = v_decode_ctx;
    }

    // 4. 查找最佳音频流
    *a_index = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, *v_index, &a_codec, 0);
    if (*a_index >= 0)
    {
        a_decode_ctx = avcodec_alloc_context3(a_codec);
        if (!a_decode_ctx)
        {
            ret = AVERROR(ENOMEM);
            av_log(nullptr, AV_LOG_ERROR, "无法分配音频解码器上下文\n");
            goto fail;
        }

        ret = avcodec_parameters_to_context(a_decode_ctx, ic->streams[*a_index]->codecpar);
        if (ret < 0)
        {
            av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
            av_log(nullptr, AV_LOG_ERROR, "无法复制音频参数: %s\n", errbuf);
            goto fail;
        }

        ret = avcodec_open2(a_decode_ctx, a_codec, nullptr);
        if (ret < 0)
        {
            av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
            av_log(nullptr, AV_LOG_ERROR, "无法打开音频解码器: %s\n", errbuf);
            goto fail;
        }

        *_a_codec = a_codec;
        *_a_decode_ctx = a_decode_ctx;
    }

    return 0;

fail:
    // 错误处理：释放已分配的资源
    if (v_decode_ctx)
        avcodec_free_context(&v_decode_ctx);
    if (a_decode_ctx)
        avcodec_free_context(&a_decode_ctx);
    if (ic)
    {
        avformat_close_input(&ic);
        avformat_free_context(ic);
    }

    return ret;
}

int SWSInit(void **swCtx,
                         unsigned char **out_buffer,
                         AVFrame **outFrame,
                         AVCodecContext *decode_ctx,
                         AVStream *stream)
{

    if (!decode_ctx || !stream)
    {
        return -1;
    }

    if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        *out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, decode_ctx->width, decode_ctx->height, 1));

        AVFrame *oFrame = av_frame_alloc();
        av_image_fill_arrays(
            oFrame->data,
            oFrame->linesize,
            *out_buffer,
            AV_PIX_FMT_RGB32,
            decode_ctx->width,
            decode_ctx->height,
            1);

        // 初始化img_convert_ctx结构 对H264做图像转换
        struct SwsContext *img_convert_ctx = sws_getContext(decode_ctx->width,
                                                            decode_ctx->height,
                                                            decode_ctx->pix_fmt, // AVPixelFormat
                                                            decode_ctx->width,
                                                            decode_ctx->height,
                                                            AV_PIX_FMT_RGB32,
                                                            SWS_BICUBIC,
                                                            NULL, NULL, NULL);

        *outFrame = oFrame;
        *swCtx = (void *)img_convert_ctx;
    }
    else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
    {
    }

    return 0;
}
