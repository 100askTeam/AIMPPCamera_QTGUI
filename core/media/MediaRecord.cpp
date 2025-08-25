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
#include "MediaRecord.hpp"
#include <unistd.h>
#include <qdebug.h>
#include "ffmpeg_common.h"
#include <QDebug>
#include "MediaCore.hpp"
#include <libavutil/time.h>

AVRecord::AVRecord()
{
    // init param
    octx = NULL;
    pVideoStream = NULL;
    pAudioStream = NULL;
    avof = NULL;
    videoIndex = -1;
    audioIndex = -1;

    // 初始化编码器相关
    vCodec = nullptr;
    aCodec = nullptr;
    enc_ctx = nullptr;
    aenc_ctx = nullptr;
    frame = nullptr;
    pkt = nullptr;
    wait_key_frame = true; // 初始为true表示等待关键帧

    recordFile = "";
    avRecordStatus = AV_RECORD_STATUS_DESTROY;
}

AVRecord::~AVRecord()
{
    // 确保正确清理所有资源
    CloseAVRecorder();
}

std::string AVRecord::getRecordFile() const
{
    return recordFile;
}

void AVRecord::setRecordFile(const std::string &newRecordFile)
{
    recordFile = newRecordFile;
}

AV_RECORD_STATUS AVRecord::getAvRecordStatus() const
{
    return avRecordStatus;
}

void AVRecord::setAvRecordStatus(AV_RECORD_STATUS newAvRecordStatus)
{
    avRecordStatus = newAvRecordStatus;
}

int AVRecord::WriteRawPacket(AVPacket* pkt, AVStream *input_stream) {
    if (getAvRecordStatus() != AV_RECORD_STATUS_RUNING || !octx) return -1;

    AVPacket* new_pkt = nullptr;
    bool is_valid_packet = false;

    // 获取输入包的流信息
    AVStream* in_stream = nullptr;
    AVStream* out_stream = nullptr;

    // 验证流索引并获取对应的输入输出流
    if (input_stream && pkt->stream_index == input_stream->index && pkt->stream_index == this->videoIndex && octx->streams[this->videoIndex]) {
        in_stream = input_stream;
        out_stream = octx->streams[this->videoIndex];
    } else if (input_stream && pkt->stream_index == input_stream->index && pkt->stream_index == this->audioIndex && octx->streams[this->audioIndex]) {
        in_stream = input_stream;
        out_stream = octx->streams[this->audioIndex];
    } else {
        qDebug("Unknown stream index: %d\n", pkt->stream_index);
        return -1;
    }

    // 视频流处理（关键帧逻辑保留）
    if (pkt->stream_index == this->videoIndex) {
        bool is_key_frame = (pkt->flags & AV_PKT_FLAG_KEY) != 0;

        if (wait_key_frame && !is_key_frame) {
            qDebug("Waiting for key frame, skipping non-key frame packet.");
            return 0;
        }

        if (wait_key_frame && is_key_frame) {
            wait_key_frame = false;
        }
        is_valid_packet = true;
    }
    // 音频流处理（确保在视频关键帧之后）
    else if (!wait_key_frame && pkt->stream_index == this->audioIndex) {
        is_valid_packet = true;
    }

    if (is_valid_packet && in_stream && out_stream) {
        new_pkt = av_packet_clone(pkt);
        // 时间戳归一化：分别为音视频流记录起始时间戳，确保各自从0开始
        if (pkt->stream_index == this->videoIndex) {
            if (video_start_pts == AV_NOPTS_VALUE && new_pkt->pts != AV_NOPTS_VALUE)
            video_start_pts = new_pkt->pts;
            if (video_start_pts != AV_NOPTS_VALUE) {
            if (new_pkt->pts != AV_NOPTS_VALUE)
                new_pkt->pts -= video_start_pts;
            if (new_pkt->dts != AV_NOPTS_VALUE)
                new_pkt->dts -= video_start_pts;
            }
        } else if (pkt->stream_index == this->audioIndex) {
            if (audio_start_pts == AV_NOPTS_VALUE && new_pkt->pts != AV_NOPTS_VALUE)
            audio_start_pts = new_pkt->pts;
            if (audio_start_pts != AV_NOPTS_VALUE) {
            if (new_pkt->pts != AV_NOPTS_VALUE)
                new_pkt->pts -= audio_start_pts;
            if (new_pkt->dts != AV_NOPTS_VALUE)
                new_pkt->dts -= audio_start_pts;
            }
        }

        // 转换时间基
        if (new_pkt->pts != AV_NOPTS_VALUE)
            new_pkt->pts = av_rescale_q(new_pkt->pts, in_stream->time_base, out_stream->time_base);
        if (new_pkt->dts != AV_NOPTS_VALUE)
            new_pkt->dts = av_rescale_q(new_pkt->dts, in_stream->time_base, out_stream->time_base);

        new_pkt->duration = av_rescale_q(new_pkt->duration, in_stream->time_base, out_stream->time_base);

        // 设置流索引
        new_pkt->stream_index = (pkt->stream_index == this->videoIndex) ? this->videoIndex : this->audioIndex;
        new_pkt->pos = -1; // 输出文件中位置由muxer自动处理

        qDebug("WriteRawPacket: Writing packet to stream index %d, pts: %lld, dts: %lld",
               new_pkt->stream_index, new_pkt->pts, new_pkt->dts);
    }

    if (new_pkt) {
        int ret = av_interleaved_write_frame(octx, new_pkt);
        av_packet_free(&new_pkt);
        return ret;
    }

    return -1;
}

int AVRecord::CloseAVRecorder()
{
    this->setAvRecordStatus(AV_RECORD_STATUS_DESTROY);
    sleep(2);

    // 释放编码器上下文
    if (this->enc_ctx) {
        avcodec_free_context(&this->enc_ctx);
        this->enc_ctx = nullptr;
    }
    
    if (this->aenc_ctx) {
        avcodec_free_context(&this->aenc_ctx);
        this->aenc_ctx = nullptr;
    }

    // 释放包和帧
    if (this->pkt) {
        av_packet_free(&this->pkt);
        this->pkt = nullptr;
    }
    
    if (this->frame) {
        av_frame_free(&this->frame);
        this->frame = nullptr;
    }

    // 关闭输出上下文
    if (this->octx)
    {
        av_write_trailer(this->octx);
        if (this->octx && !(this->octx->oformat->flags & AVFMT_NOFILE))
        {
            avio_close(this->octx->pb);
        }
        avformat_free_context(this->octx);
        this->octx = nullptr;
    }

    // 重置变量
    this->videoIndex = -1;
    this->audioIndex = -1;
    this->pVideoStream = nullptr;
    this->pAudioStream = nullptr;
    this->avof = nullptr;
    this->vCodec = nullptr;
    this->aCodec = nullptr;

    
    qDebug("CloseAVRecorder() [%s] success! \n", this->getRecordFile().c_str());

    return 1;
}

int AVRecord::InitAVRecorder(void *ms_)
{
    MediaState *ms = (MediaState *)ms_;
    qDebug("in InitAVRecorder \n");
    char errbuf[AV_ERROR_MAX_STRING_SIZE];

    // 基础校验
    if (!ms || !ms->ic) {
        qDebug("InitAVRecorder: invalid MediaState or format context\n");
        return -1;
    }

    int ret = -1;

    // 推断输出格式
    this->avof = av_guess_format(nullptr, this->getRecordFile().c_str(), nullptr);
    if (!this->avof) {
        qDebug("av_guess_format error \n");
        return -1;
    }

    // 分配输出上下文
    ret = avformat_alloc_output_context2(&(this->octx), avof, NULL, this->getRecordFile().c_str());
    if (!this->octx) {
        qDebug("avformat_alloc_output_context2 error \n");
        return -1;
    }

    // 创建输出流并从输入拷贝参数（走复用直通，不重新编码）
    AVStream *inV = (ms->video_stream_index >= 0) ? ms->ic->streams[ms->video_stream_index] : nullptr;
    AVStream *inA = (ms->audio_stream_index >= 0) ? ms->ic->streams[ms->audio_stream_index] : nullptr;

    if (inV) {
        this->pVideoStream = avformat_new_stream(this->octx, nullptr);
        if (!this->pVideoStream) {
            qDebug("Failed to add video stream\n");
            return -1;
        }
        if (avcodec_parameters_copy(this->pVideoStream->codecpar, inV->codecpar) < 0) {
            qDebug("Failed to copy video codec parameters\n");
            return -1;
        }
        this->pVideoStream->codecpar->codec_tag = 0; // 让muxer自行选择
        this->pVideoStream->time_base = AVRational{1, 90000}; // 与WriteData保持一致
        this->videoIndex = this->pVideoStream->index;
    }

    if (inA) {
        this->pAudioStream = avformat_new_stream(this->octx, nullptr);
        if (!this->pAudioStream) {
            qDebug("Failed to add audio stream\n");
            return -1;
        }
        if (avcodec_parameters_copy(this->pAudioStream->codecpar, inA->codecpar) < 0) {
            qDebug("Failed to copy audio codec parameters\n");
            return -1;
        }
        this->pAudioStream->codecpar->codec_tag = 0;
        int sr = this->pAudioStream->codecpar->sample_rate > 0 ? this->pAudioStream->codecpar->sample_rate : 48000;
        this->pAudioStream->time_base = AVRational{1, sr}; // 与WriteData保持一致
        this->audioIndex = this->pAudioStream->index;
    }

    if (this->octx->nb_streams <= 0) {
        qDebug("InitAVRecorder: no output streams created\n");
        return -1;
    }

    std::string recordPath = this->getRecordFile();
    if (recordPath.empty()) {
        qDebug("Record file path is empty\n");
        return -1;
    }
    qDebug("Output file path: %s\n", recordPath.c_str());

    // 打开IO
    if (!(this->octx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&this->octx->pb, recordPath.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
            qDebug("Could not open output file '%s': %s\n", recordPath.c_str(), errbuf);
            return ret;
        }
    }

    // 写文件头
    AVDictionary *opts = nullptr;
    // 可选优化：快速开始
    // av_dict_set(&opts, "movflags", "faststart+use_metadata_tags", 0);
    ret = avformat_write_header(this->octx, &opts);
    if (ret < 0) {
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        qDebug("Error writing header: %s\n", errbuf);
        av_dict_free(&opts);
        return ret;
    }
    av_dict_free(&opts);

    // 状态切换为运行
    this->setAvRecordStatus(AV_RECORD_STATUS_RUNING);

    // dump
    return 0;
}

