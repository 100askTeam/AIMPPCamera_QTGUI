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
#ifndef MAINBACKEND_H
#define MAINBACKEND_H

#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QImage>
#include "CameraConfig.h"

class MainBackend : public QObject
{
    Q_OBJECT

public:
//    explicit MainBackend(QObject *parent = nullptr);
    MainBackend();
    ~MainBackend();

    //获取相机配置列表
    // 提供一个QML可调用的方法，返回QVariantMap（兼容QML的对象类型）
    Q_INVOKABLE QVariantMap getCameraInfo(int camId);

    //保存信息
    Q_INVOKABLE int setCameraInfo(int camId,QVariantMap camInfo);


    // 声明一个可读取的 QStringList 属性
    Q_PROPERTY(QStringList getRecFileList READ getRecFileList NOTIFY recFileListChanged)

    //获取文件列表
    QStringList getRecFileList() const;


signals:
    //更新record filename list 信号(qml收到信号后,再主动获取数据)
    void updateRecFileListSignal();

signals:
    //文件列表属性变更信号
    void recFileListChanged();


public slots:
    int updateRecFileList();
    void openFileLocation(const QString& filePath);
    void playRecFile(const QString& filePath,const QString& usePlaySoftName);
    void deleteFile(const QString& filePath);

private:
    //配置信息
    CameraConfig *camCnf;
};

#endif // MAINBACKEND_H
