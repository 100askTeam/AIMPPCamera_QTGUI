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
#ifndef UPDATA_COMMON_H_
#define UPDATA_COMMON_H_

// 配置文件参数结构体
// typedef struct __attribute__((packed)) {
typedef struct  ConfigParams{
    int main_pic_width;
    int main_pic_height;
    int main_encode_bitrate;
    int main_framerate;
    int main_product_mode;
    int rt_vi_pic_width;
    int rt_vi_pic_height;
    int rt_vi_drop_frame_count;
    int rt_vi_raw_store_num;
    int rt_vi_dst_store_interval;
    char storage_folder[256];
    int storage_rec_duration;
    int storage_rec_file_cnt;
    int overlay_x;
    int overlay_y;
    int nn_enable;              // AI功能开关
    int nn_type;                // AI类型：0：人形检测/2：目标检测
    int nn_frame_rate;          // AI检测帧率
    int nn_drawOrl_enable;      //是否绘制检测结果
    int nn_alarm_enable;        //区域入侵开关（仅在人形检测时有效）
    int nn_alarm_mode;          //0:越界检测 1：区域入侵检测
    int nn_alarm_channel;       //警报通道(默认为0 上位机可不用)
    int nn_alarm_volume;        //警报音量(可使用滑动条设置音量大小)
    int nn_alarm_rects_x0;      //如果点击区域入侵检测可在画面中绘制多边形
    int nn_alarm_rects_y0;
    int nn_alarm_rects_x1;
    int nn_alarm_rects_y1;
    int nn_alarm_rects_x2;
    int nn_alarm_rects_y2;
    int nn_alarm_rects_x3;
    int nn_alarm_rects_y3;
    int nn_alarm_rects_direction; //触发方向 0：双向，1：进入，2：离开
    int nn_alarm_lines_x1;       //如果点击越界入侵检测可在画面中绘制直线
    int nn_alarm_lines_y1;
    int nn_alarm_lines_x2;
    int nn_alarm_lines_y2;
    int nn_alarm_lines_direction; //触发方向 0：双向，1：左->右，2：右->左
} ConfigParams;

// 命令类型枚举
typedef enum {
    CMD_START = 0,        // 启动命令
    CMD_STOP,         // 停止命令
    CMD_RESTART,       // 重启命令
    CMD_CONFIG_SETTINGS, // 配置设置命令
} CommandType;

#endif