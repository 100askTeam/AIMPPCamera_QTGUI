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
#include "CameraConfig.h"
#include <QFile>

CameraConfig::CameraConfig(QString path)
{
    this->configPath = path;
    InitCamConfig();
}


//读取文件加载到内存中
int CameraConfig::InitCamConfig()
{
    // 打开文件
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug() << "无法打开文件:" << configPath << "错误:" << file.errorString();
      //return -1;
      // 尝试创建并写入默认配置
         if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
             // 写入默认摄像头配置内容
             QTextStream out(&file);
             out << "{\n";
             out << "    \"cameras\": [\n";
             out << "        {\n";
             out << "            \"camIP\": \"192.168.2.111\",\n";
             out << "            \"camName\": \"摄像头1\",\n";
             out << "            \"camPort\": \"9000\",\n";
             out << "            \"camViewIndex\": 0\n";
             out << "        },\n";
             out << "        {\n";
             out << "            \"camIP\": \"192.168.1.122\",\n";
             out << "            \"camName\": \"摄像头2\",\n";
             out << "            \"camPort\": \"9000\",\n";
             out << "            \"camViewIndex\": 1\n";
             out << "        },\n";
             out << "        {\n";
             out << "            \"camIP\": \"192.168.1.127\",\n";
             out << "            \"camName\": \"摄像头3\",\n";
             out << "            \"camPort\": \"9000\",\n";
             out << "            \"camViewIndex\": 2\n";
             out << "        },\n";
             out << "        {\n";
             out << "            \"camIP\": \"192.168.2.110\",\n";
             out << "            \"camName\": \"摄像头4\",\n";
             out << "            \"camPort\": \"9000\",\n";
             out << "            \"camViewIndex\": 3\n";
             out << "        }\n";
             out << "    ]\n";
             out << "}\n";
             file.close();
             qDebug() << "新配置文件创建成功";
             return 0;
         } else {
             qDebug() << "创建文件失败:" << file.errorString();
             return -2; // 创建文件失败
         }
    }

    // 读取文件内容
    QByteArray jsonData = file.readAll();
    file.close();

    // 解析JSON数据
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
      qDebug() << "JSON解析错误:" << parseError.errorString()
               << "位置:" << parseError.offset;
      return -1;
    }

    if (!doc.isObject()) {
      qDebug() << "JSON文件内容不是一个对象";
      return -1;
    }

    this->cameraConfig = doc.object();
    return 0;
}


//保存到文件
int CameraConfig::SaveCamConfig()
{
    // 检查配置是否有效
    if (cameraConfig.isEmpty()) {
        qDebug() << "配置为空，不进行保存";
        return -1;
    }

    // 打开文件（如果不存在则创建，存在则覆盖）
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QFile::Truncate)) {
        qDebug() << "无法打开文件进行写入:" << configPath << "错误:" << file.errorString();
        return -1;
    }

    // 将JSON对象转换为带缩进的字符串（便于阅读）
    QJsonDocument doc(cameraConfig);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    // 写入文件并检查写入大小
    qint64 bytesWritten = file.write(jsonData);
    if (bytesWritten != jsonData.size()) {
        qDebug() << "文件写入不完整，预期:" << jsonData.size() << "实际写入:" << bytesWritten;
        file.close();
        return -1;
    }

    // 确保数据刷新到磁盘
    file.flush();
    file.close();

    qDebug() << "配置已成功保存到:" << file.fileName();
    return 0;
}

//获取所有的相机配置信息
QJsonObject CameraConfig::GetCameraAllConfig(){

   return cameraConfig;
}

//获取指定Id的相机信息
QJsonObject CameraConfig::GetCameraByIdConfig(int camId){
    qDebug("获取相机信息, camId=%d \n", camId);

    // 1. 检查是否包含"cameras"键且类型为数组
    if (!cameraConfig.contains("cameras") || !cameraConfig["cameras"].isArray()) {
        qDebug("错误: 配置中没有有效的cameras数组\n");
        return QJsonObject();
    }

    // 2. 转换为QJsonArray
    QJsonArray cameras = cameraConfig["cameras"].toArray();

    // 3. 遍历数组，对比camViewIndex
    for (int i = 0; i < cameras.size(); ++i) {
        QJsonValue camValue = cameras.at(i);

        // 检查元素是否为对象
        if (!camValue.isObject()) {
            qDebug("警告: 索引%d处不是有效的JSON对象，跳过\n", i);
            continue;
        }

        QJsonObject camObj = camValue.toObject();

        // 检查是否包含camViewIndex字段且类型正确
        if (!camObj.contains("camViewIndex") || !camObj["camViewIndex"].isDouble()) {
            qDebug("警告: 索引%d处的摄像头缺少有效的camViewIndex，跳过\n", i);
            continue;
        }

        // 对比camViewIndex值
        int currentViewIndex = camObj["camViewIndex"].toInt();
        if (currentViewIndex == camId) {
            qDebug("找到匹配的摄像头，索引=%d\n", i);
            return camObj;
        }
    }

    // 4. 未找到匹配项
    qDebug("未找到camViewIndex=%d的摄像头\n", camId);
    return QJsonObject();
}

/**
 * @brief EditCameraConfig 修改信息
 * @param camId 要修改的camId
 * @param camInfo
 * @return
 */
int CameraConfig::EditCameraConfig(int camId,QJsonObject camInfo)
{
    qDebug("EditCameraConfig camId=%d \n", camId);
    if (!cameraConfig.contains("cameras") || !cameraConfig["cameras"].isArray()) {
       qDebug("错误: 配置中没有有效的cameras数组\n");
       return -1;
   }
    QJsonArray cameras = cameraConfig["cameras"].toArray();
    bool isFound = false;

    for (int i = 0; i < cameras.size(); ++i) {
        QJsonValue camValue = cameras.at(i);

        if (!camValue.isObject()) {
            continue;
        }

        QJsonObject camObj = camValue.toObject();

        if (!camObj.contains("camViewIndex") || !camObj["camViewIndex"].isDouble()) {
            continue;
        }

        // 对比camViewIndex值
        int currentViewIndex = camObj["camViewIndex"].toInt();
        if (currentViewIndex == camId) {
            qDebug("找到匹配的摄像头，索引=%d,进行修改\n", i);
            // 仅更新提供的字段，保留原有其他字段
            for (auto it = camInfo.begin(); it != camInfo.end(); ++it) {
                camObj[it.key()] = it.value();
            }

            // 将修改后的对象写回数组
            cameras.replace(i, camObj);
            isFound = true;
        }
    }

    if (!isFound) {
        qDebug("未找到camViewIndex=%d的摄像头，无法修改\n", camId);
        return -2;
    }

     cameraConfig["cameras"] = cameras;

    SaveCamConfig();
    return 0;
}
