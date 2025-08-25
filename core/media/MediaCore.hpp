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
#ifndef MEDIACORE_H_
#define MEDIACORE_H_

#include "string"
#include "list"
#include "thread"
#include "map"
#include "queue"
#include "ffmpeg_common.h"
#include "MediaRecord.hpp"
#include <mutex>

typedef struct PacketQueue {
    std::list<AVPacket*> pkt_list;
    int nb_packets;
    int size;
    //lock
    std::mutex mutex;
} PacketQueue;

enum MEDIA_Core_TYPE{
    MEDIA_TYPE_MEDIA_PLAY
};


enum REC_CTRL{
    REC_CTRL_NONE=-1,
    REC_CTRL_START,//开始录像
    REC_CTRL_STOP//停止录像
};

typedef struct MediaState{
    std::string filename; //文件名

    int exit_request;//1:exit

    // media type:media play,record play
    MEDIA_Core_TYPE media_type;

    const AVInputFormat *iformat;   //指向demuxer
    AVFormatContext *ic; //iformat的上下文

    //总时长
    int totalDuration;

    int video_stream_index;
    AVStream *video_st;
    int audio_stream_index;
    AVStream *audio_st;

    std::thread read_thr;//read thread
    std::thread write_thr;//write thread

    //data queue
    std::queue<AVPacket> dataQueue;


    const AVCodec *codec_video = NULL;
    const AVCodec *codec_audio = NULL;
    //decode
    AVCodecContext *decode_v_ctx = NULL;
    AVCodecContext *decode_a_ctx = NULL;
    AVFrame    *pFrame_video = NULL;
    AVFrame    *pFrame_audio = NULL;
    AVFrame    *pFrameRGB_video = NULL;
    AVPacket *packet  = NULL;
    unsigned char *out_buffer_video ;
    unsigned char *out_buffer_audio ;
    int out_buffer_audio_size;
    struct SwsContext *img_convert_ctx = NULL;

    void* user;

    AVRecord *avRecord = NULL;
    //录像通知
    enum REC_CTRL record_ctrl = REC_CTRL_NONE;

} MediaState;


//open stream
MediaState *MEDIA_stream_open(MEDIA_Core_TYPE media_t,std::string filename, void *uVideoOutput);

void MEDIA_stream_exit(MediaState *ms);


int MEDIA_stream_Record(MediaState *ms,REC_CTRL r_ctrl);


#endif // MEDIA_H
