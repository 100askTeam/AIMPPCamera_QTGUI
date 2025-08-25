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
#include "MediaCore.hpp"
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include "iostream"
#include "unistd.h"
#include "map"
#include "QImage"
#include "MediaStream.hpp"
#include "QCoreApplication"
#include "QDir"

#include "../CameraPaintItem.h"
static MediaFile mf;

void MEDIA_stream_exit(MediaState *ms)
{
    if(ms->img_convert_ctx != nullptr)
    {
         sws_freeContext(ms->img_convert_ctx);
    }

    if(ms->pFrameRGB_video != nullptr)
    {
         av_frame_free(&ms->pFrameRGB_video);
    }

    if(ms->pFrame_video != nullptr)
    {
         av_frame_free(&ms->pFrame_video);
    }

    if(ms->pFrame_audio != nullptr)
    {
         av_frame_free(&ms->pFrame_audio);
    }

    if(ms->ic)
    {
        avformat_close_input(&ms->ic);
    }

    ms->video_stream_index = -1;
    ms->audio_stream_index = -1;
    ms->filename.clear();
    std::cout << "MEDIA_stream_exit success !" << std::endl;
}


MediaState *MEDIA_stream_open(MEDIA_Core_TYPE media_t,std::string filename,void *user)
{
    MediaState *ms;
    ms = (MediaState*)av_mallocz(sizeof (MediaState));
    if(!ms)
    {
        return nullptr;
    }

    ms->media_type = media_t;
    ms->video_stream_index = -1;
    ms->audio_stream_index = -1;
    ms->filename = filename;

    if (ms->filename.empty())
    {
        return nullptr;
    }

    if(ms->media_type == MEDIA_TYPE_MEDIA_PLAY)
    {
        ms->user = user;
        mf.InitMediaFile((void*)ms);
    }
    return ms;
}



int MEDIA_stream_Record(MediaState *ms,REC_CTRL r_ctrl)
{
    if(!ms)
    {
        qDebug("MediaState is null.\n");
        return -1;
    }

    //开始录像
    if(ms->avRecord == NULL && r_ctrl == REC_CTRL_START)
    {
        ms->avRecord = new AVRecord();
        //构造录像名字
        std::string recordFileName = "";
        {
            // 获取程序所在目录的绝对路径（不含.exe文件名）
            QString appDir = QCoreApplication::applicationDirPath();

            // 拼接配置文件的完整路径
            QString recPath = QDir(appDir).filePath("recordfiles");

            // 检查目录是否存在
            QDir dir(recPath);
            if (!dir.exists()) {
                dir.mkpath(".");
            }

            qDebug() << "录像文件路径：" << recPath;
            recordFileName = recPath.toStdString();

            // 时间命名
            // 如果需要按时间命名，可以使用如下代码
            time_t now = time(nullptr);
            char buf[64];
            strftime(buf, sizeof(buf), "/%Y%m%d%H%M%S.mp4", localtime(&now));
            recordFileName.append(buf);
        }
        ms->avRecord->setRecordFile(recordFileName);
        ms->avRecord->InitAVRecorder((void*)ms);

        //通知线程，启动录像功能
        ms->record_ctrl = REC_CTRL_START;
    }
    //停止录像
    else if(ms->avRecord && r_ctrl == REC_CTRL_STOP)
    {
        if(ms->avRecord->getAvRecordStatus() == AV_RECORD_STATUS_RUNING)
        {
            ms->avRecord->CloseAVRecorder();
            delete ms->avRecord;
            ms->avRecord = NULL;
        }
        ms->record_ctrl = REC_CTRL_STOP;
    }


    return 0;
}

