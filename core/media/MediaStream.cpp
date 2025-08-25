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
#include "MediaStream.hpp"
#include "MediaCore.hpp"
#include "../CameraPaintItem.h"
#include <iostream>
#include <QDebug>
#include <QImage>
#include "ffmpeg_common.h"

using namespace std;

MediaFile::MediaFile()
{

}

void media_play_read_thread (void*arg) {
    MediaState *ms = (MediaState*)arg;
    CameraPaintItem* uVideoOutput = (CameraPaintItem*)(ms->user);

    for (;;) {
        //检测是否退出
        if(ms->exit_request)
        {
            std::cout << "exit media_play_read_thread()" << std::endl;
            goto end;
        }

        //read media data
        int ret = -1;
        // Ensure ms->packet is allocated before use
        if (!ms->packet) {
            ms->packet = av_packet_alloc();
            if (!ms->packet) {
                qDebug("Failed to allocate AVPacket");
                goto end;
            }
        }
        while (((ret=av_read_frame(ms->ic, ms->packet)) >= 0)
               && ms->exit_request == 0){

                //如果是视频数据
                if (ms->packet->stream_index == ms->video_stream_index){

                    //保存H264到mp4
                    if(ms->avRecord && ms->avRecord->getAvRecordStatus() == AV_RECORD_STATUS_RUNING)
                    {
                      ms->avRecord->WriteRawPacket(ms->packet, ms->ic->streams[ms->video_stream_index]);
                    }


                    //解码一帧视频数据
                    ret = avcodec_send_packet(ms->decode_v_ctx, ms->packet);

                    if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        cout << "avcodec_send_packet: " << ret ;
                        goto end;
                    }

                    AVStream *stream=ms->ic->streams[ms->video_stream_index];
                    while (avcodec_receive_frame(ms->decode_v_ctx, ms->pFrame_video) == 0) {
                        //获取到 pFrame 解码数据，在这里进行格式转换，然后进行渲染
                        sws_scale(ms->img_convert_ctx,
                                    (const unsigned char* const*)ms->pFrame_video->data,
                                  ms->pFrame_video->linesize,
                                  0,
                                  ms->decode_v_ctx->height,
                                          ms->pFrameRGB_video->data,
                                  ms->pFrameRGB_video->linesize);

                        QImage img((uchar*)ms->pFrameRGB_video->data[0],ms->pFrame_video->width,ms->pFrame_video->height,QImage::Format_RGB32);

                        //播放每一帧视频
                        uVideoOutput->setImage(img);

                        //SDL_Delay(10);
                        av_frame_unref(ms->pFrame_video);
                    }
                }
                else if(ms->packet->stream_index == ms->audio_stream_index){

                    //保存AAC到mp4
                    if(ms->avRecord && ms->avRecord->getAvRecordStatus() == AV_RECORD_STATUS_RUNING)
                    {
                      ms->avRecord->WriteRawPacket(ms->packet, ms->ic->streams[ms->audio_stream_index]);
                    }
                }
        }

        if(ret < 0)
        {
            goto end;
        }
    }

end:

    MEDIA_stream_exit(ms);
}

int MediaFile::InitMediaFile(void *_ms)
{
    MediaState *ms = (MediaState *)_ms;
    AVFormatContext *ic = NULL;

    int ret = -1;
    ret = DecoderInit(ms->filename,
                   &ic,
                   &ms->video_stream_index,
                   &ms->audio_stream_index,
                   &ms->codec_video,
                   &ms->codec_audio,
                   &ms->decode_v_ctx,
                   &ms->decode_a_ctx);

    if(ret < 0){
        av_log(NULL, AV_LOG_FATAL, "DecoderInit error.\n");
        return -1;
    }

    //get media info
    SWSInit((void**)&ms->img_convert_ctx,
               &ms->out_buffer_video,
               &ms->pFrameRGB_video,
               ms->decode_v_ctx,
               ic->streams[ms->video_stream_index]);

    ms->pFrame_video = av_frame_alloc();
    ms->packet = av_packet_alloc();
    ms->ic = ic;

    //create read thread
    ms->read_thr = std::thread(media_play_read_thread,ms);
    ms->read_thr.detach(); //分离线程，不阻塞
    return 1;
}
