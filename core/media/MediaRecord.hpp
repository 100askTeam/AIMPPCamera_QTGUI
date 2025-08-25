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
#ifndef MEDIARECORD_H
#define MEDIARECORD_H


#include "string"
#include "ffmpeg_common.h"

enum AV_RECORD_STATUS{
    AV_RECORD_STATUS_DESTROY = 0,//no
    AV_RECORD_STATUS_RUNING,      // 正在录像状态
    AV_RECORD_STATUS_PAUSE,       // 暂停录像状态
};

class AVRecord
{

private:
    AVStream *pVideoStream;
    AVStream *pAudioStream;

    const AVOutputFormat * avof;

    int videoIndex;
    int audioIndex;

    std::string recordFile;

    AV_RECORD_STATUS avRecordStatus;
    bool wait_key_frame = true; // 标志位，初始为true表示等待关键帧
    int64_t video_start_pts = AV_NOPTS_VALUE; // 用于记录视频开始时间戳
    int64_t audio_start_pts = AV_NOPTS_VALUE; // 用于记录音频开始时间戳
public:
    int frameIndex = 0;
    AVFormatContext *octx;


    //encodec
    const AVCodec *vCodec = nullptr;
    const AVCodec *aCodec = nullptr;
    AVCodecContext *enc_ctx = nullptr;
    AVCodecContext *aenc_ctx = nullptr;

    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;

    AVRecord();
    ~AVRecord();

    std::string getRecordFile() const;
    void setRecordFile(const std::string &newRecordFile);
    AV_RECORD_STATUS getAvRecordStatus() const;
    void setAvRecordStatus(AV_RECORD_STATUS newAvRecordStatus);

    int CloseAVRecorder();
    int InitAVRecorder(void *ms);

    int WriteRawPacket(AVPacket* pkt,AVStream *input_stream);
};

#endif // MEDIARECORD_H
