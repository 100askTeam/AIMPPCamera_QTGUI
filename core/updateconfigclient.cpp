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
#include "UpdateConfigCLient.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QUrl>
#include <QDebug>

UpdateConfigClient::UpdateConfigClient(QObject *parent)
    : QObject(parent), manager(new QNetworkAccessManager(this))
{
    // 连接网络响应信号
    connect(manager, &QNetworkAccessManager::finished,
            this, &UpdateConfigClient::onReplyFinished);
}

UpdateConfigClient::~UpdateConfigClient()
{
}

int UpdateConfigClient::InitClient(std::string server_ip, int server_port)
{
    if (server_port <= 0 || server_port > 65535) {
        qWarning() << "Invalid server port:" << server_port;
        return -1;
    }

    serverIp = server_ip;
    serverPort = server_port;
    baseUrl = QString("http://%1:%2").arg(QString::fromStdString(server_ip))
                                    .arg(server_port).toStdString();

    qDebug() << "Client initialized, server:" << QString::fromStdString(baseUrl);
    return 0;
}

int UpdateConfigClient::SendStartCMD()
{
    QJsonObject data;
    data["command"] = "start";
    return sendRequest("control", data);
}

int UpdateConfigClient::SendStopCMD()
{
    QJsonObject data;
    data["command"] = "stop";
    return sendRequest("control", data);
}

int UpdateConfigClient::SendReStartCMD()
{
    QJsonObject data;
    data["command"] = "restart";
    return sendRequest("control", data);
}

// 辅助函数：将ConfigParams转换为QJsonObject
QJsonObject configParamsToJson(const ConfigParams &cp) {
    QJsonObject json;

    // 主图像参数
    json["main_pic_width"] = cp.main_pic_width;
    json["main_pic_height"] = cp.main_pic_height;
    json["main_encode_bitrate"] = cp.main_encode_bitrate;
    json["main_framerate"] = cp.main_framerate;
    json["main_product_mode"] = cp.main_product_mode;

    // 实时视频参数
    json["rt_vi_pic_width"] = cp.rt_vi_pic_width;
    json["rt_vi_pic_height"] = cp.rt_vi_pic_height;
    json["rt_vi_drop_frame_count"] = cp.rt_vi_drop_frame_count;
    json["rt_vi_raw_store_num"] = cp.rt_vi_raw_store_num;
    json["rt_vi_dst_store_interval"] = cp.rt_vi_dst_store_interval;

    // 存储参数
    json["storage_folder"] = QString(cp.storage_folder);  // char数组转QString
    json["storage_rec_duration"] = cp.storage_rec_duration;
    json["storage_rec_file_cnt"] = cp.storage_rec_file_cnt;

    // 叠加层参数
    json["overlay_x"] = cp.overlay_x;
    json["overlay_y"] = cp.overlay_y;

    // AI功能参数
    json["nn_enable"] = cp.nn_enable;
    json["nn_type"] = cp.nn_type;
    json["nn_frame_rate"] = cp.nn_frame_rate;
    json["nn_drawOrl_enable"] = cp.nn_drawOrl_enable;
    json["nn_alarm_enable"] = cp.nn_alarm_enable;
    json["nn_alarm_mode"] = cp.nn_alarm_mode;
    json["nn_alarm_channel"] = cp.nn_alarm_channel;
    json["nn_alarm_volume"] = cp.nn_alarm_volume;

    // 区域入侵检测参数
    json["nn_alarm_rects_x0"] = cp.nn_alarm_rects_x0;
    json["nn_alarm_rects_y0"] = cp.nn_alarm_rects_y0;
    json["nn_alarm_rects_x1"] = cp.nn_alarm_rects_x1;
    json["nn_alarm_rects_y1"] = cp.nn_alarm_rects_y1;
    json["nn_alarm_rects_x2"] = cp.nn_alarm_rects_x2;
    json["nn_alarm_rects_y2"] = cp.nn_alarm_rects_y2;
    json["nn_alarm_rects_x3"] = cp.nn_alarm_rects_x3;
    json["nn_alarm_rects_y3"] = cp.nn_alarm_rects_y3;
    json["nn_alarm_rects_direction"] = cp.nn_alarm_rects_direction;

    // 越界检测参数
    json["nn_alarm_lines_x1"] = cp.nn_alarm_lines_x1;
    json["nn_alarm_lines_y1"] = cp.nn_alarm_lines_y1;
    json["nn_alarm_lines_x2"] = cp.nn_alarm_lines_x2;
    json["nn_alarm_lines_y2"] = cp.nn_alarm_lines_y2;
    json["nn_alarm_lines_direction"] = cp.nn_alarm_lines_direction;

    return json;
}


int UpdateConfigClient::SendDataAllConfig(ConfigParams &cp)
{
    return sendRequest("config", configParamsToJson(cp));
}

int UpdateConfigClient::sendRequest(const std::string &endpoint, const QJsonObject &data)
{
    if (baseUrl.empty()) {
        qWarning() << "Client not initialized, call InitClient first";
        return -1;
    }

    // 构建完整URL
    QString urlStr = QString("%1/%2").arg(QString::fromStdString(baseUrl))
                                    .arg(QString::fromStdString(endpoint));
    QUrl url(urlStr);
    QNetworkRequest request(url);

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 发送POST请求
    QByteArray postData = QJsonDocument(data).toJson();
    manager->post(request, postData);

    qDebug() << "Sending request to:" << urlStr << "Data:" << postData;
    return 0;
}

void UpdateConfigClient::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Request error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    // 读取响应数据
    QByteArray responseData = reply->readAll();
    qDebug() << "Received response:" << responseData;

    // 解析JSON响应
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
    if (parseError.error == QJsonParseError::NoError && !jsonDoc.isNull()) {
        if (jsonDoc.isObject()) {
            QJsonObject responseObj = jsonDoc.object();

            // 如果是配置响应，更新本地配置
            if (reply->url().path().contains("config")) {
//                config.fromJson(responseObj);
                qDebug() << "Config updated successfully";
            }
            // 可以根据不同的接口路径处理不同的响应
        }
    } else {
        qWarning() << "Invalid JSON response:" << parseError.errorString();
    }

    reply->deleteLater();
}


