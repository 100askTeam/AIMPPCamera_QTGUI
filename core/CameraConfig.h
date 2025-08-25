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
#ifndef CAMERACONFIG_H
#define CAMERACONFIG_H
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QString>

class CameraConfig
{
private:
    //配置文件路径
    QString configPath{""};

    //配置信息
    QJsonObject  cameraConfig;

    //读取文件加载到内存中
    int InitCamConfig();
    //保存到文件
    int SaveCamConfig();

public:
    CameraConfig(QString path);

    //获取所有的相机配置信息
    QJsonObject GetCameraAllConfig();

    //获取指定Id的相机信息
    QJsonObject GetCameraByIdConfig(int camId);

    /**
     * @brief EditCameraConfig 修改信息
     * @param camId 要修改的camId
     * @param camInfo
     * @return
     */
    int EditCameraConfig(int camId,QJsonObject camInfo);
};

#endif // CAMERACONFIG_H
