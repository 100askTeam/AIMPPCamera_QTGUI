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
#ifndef UPDATECONFIGCLIENT_H
#define UPDATECONFIGCLIENT_H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "updata_common.h"
#include "CameraConfig.h"

#if 0
class UpdateConfigCLient
{
public:
    UpdateConfigCLient();
    ~UpdateConfigCLient();

    int InitClient(std::string server_ip,int server_port);

private:
    //服务器端口
    int port = -1;
    //服务器ip地址
    std::string ip;

    // 初始化Winsock
    WSADATA wsaData;

    //存储当前最新的配置
    ConfigParams config;

private:
    int send_command(void *data = nullptr, size_t data_size = 0);

public:
    //启动命令
    int SendStartCMD();

    //启动命令
    int SendStopCMD();

    //重启命令
    int SendReStartCMD();

    //保存所有数据
    int SendDataAllConfig(ConfigParams &cp);

    ConfigParams GetConfigParams()
    {
        return config;
    }
};

#endif

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <string>

class UpdateConfigClient : public QObject
{
    Q_OBJECT
public:
    UpdateConfigClient(QObject *parent = nullptr);
    ~UpdateConfigClient() override;

    // 初始化客户端，设置服务器IP和端口
    int InitClient(std::string server_ip, int server_port);

    // 启动命令
    int SendStartCMD();

    // 停止命令
    int SendStopCMD();

    // 重启命令
    int SendReStartCMD();

    // 发送所有配置数据
    int SendDataAllConfig(ConfigParams &cp);

    // 获取当前配置
    ConfigParams GetConfigParams() { return config; }

private slots:
    // 处理网络请求响应
    void onReplyFinished(QNetworkReply *reply);

private:
    // 发送HTTP请求的内部方法
    int sendRequest(const std::string &endpoint, const QJsonObject &data = QJsonObject());

private:
    QNetworkAccessManager *manager;  // Qt网络访问管理器
    std::string serverIp;            // 服务器IP
    int serverPort;                  // 服务器端口
    ConfigParams config;             // 配置参数存储
    std::string baseUrl;             // 基础URL
};

#endif // UPDATECONFIGCLIENT_H
