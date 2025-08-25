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
#include "mainBackend.h"
#include "QCoreApplication"
#include "QDir"
#include <QDebug>
#include <qdebug.h>

//MainBackend::MainBackend(QObject *parent)
//    : QObject{parent}
//{}

MainBackend::MainBackend()
{
  // 获取程序所在目录的绝对路径（不含.exe文件名）
  QString appDir = QCoreApplication::applicationDirPath();

  // 拼接配置文件的完整路径
  QString configPath = QDir(appDir).filePath("CameraConfig.json");

  // 调试输出路径，确认是否正确（发布时可删除）
  qDebug() << "配置文件路径：" << configPath;
    camCnf = new CameraConfig(configPath);
//    QJsonObject data;
//    data["camName"] = "门口";
//    data["camIP"] = "192.168.2.241";
//    data["camPort"] = 9000;
//    camCnf->EditCameraConfig(0,data);
}

MainBackend::~MainBackend()
{
    if(camCnf)
    {
        delete camCnf;
    }
}

QStringList MainBackend::getRecFileList() const {
     // 读取rec列表
    //构造录像名字
    std::string recordFilePath = "";
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
        recordFilePath = recPath.toStdString();
    }

    // 遍历.mp4文件
    QDir dir(recordFilePath.c_str());
    QStringList filters;
    filters << "*.mp4";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList fileList = dir.entryInfoList();

    //文件列表
    QStringList recFileList_;

    for (const QFileInfo &fileInfo : fileList) {
        qDebug() << "search rec file:" << fileInfo.absoluteFilePath();
        recFileList_.push_back(fileInfo.absoluteFilePath());
    }

    return recFileList_;
}

// 将QJsonObject转换为QVariantMap并返回给QML
QVariantMap MainBackend::getCameraInfo(int camId)
{
//    Q_UNUSED(camId); // 实际项目中根据camId查询对应数据

    // 核心：将QJsonObject转换为QVariantMap
    QJsonObject camInfo = camCnf->GetCameraByIdConfig(camId);

    QVariantMap map;
    for (auto it = camInfo.begin(); it != camInfo.end(); ++it) {
        map[it.key()] = it.value().toVariant();
    }
    return map;
}

int MainBackend::setCameraInfo(int camId,QVariantMap camInfo)
{
    QJsonObject camJsonData;

    //QVariantMap to QJsonObject
    // QVariantMap 转换为 QJsonObject
    // 遍历QVariantMap中的所有键值对
    for (auto it = camInfo.begin(); it != camInfo.end(); ++it) {
        const QString& key = it.key();
        const QVariant& value = it.value();

        // 根据值的类型转换为对应的QJsonValue
        if (value.type() == QVariant::String) {
            camJsonData[key] = value.toString();
        } else if (value.type() == QVariant::Int) {
            camJsonData[key] = value.toInt();
        } else if (value.type() == QVariant::Bool) {
            camJsonData[key] = value.toBool();
        } else if (value.type() == QVariant::Double) {
            camJsonData[key] = value.toDouble();
        } else {
            // 处理其他类型（如嵌套的QVariantMap）
            if (value.canConvert<QVariantMap>()) {
                // 如果是嵌套的QVariantMap，递归转换为QJsonObject
                QVariantMap nestedMap = value.toMap();
                QJsonObject nestedJson;
                for (auto nestedIt = nestedMap.begin(); nestedIt != nestedMap.end(); ++nestedIt) {
                    nestedJson[nestedIt.key()] = QJsonValue::fromVariant(nestedIt.value());
                }
                camJsonData[key] = nestedJson;
            } else {
                // 不支持的类型，可根据需要处理（忽略或记录警告）
                qWarning() << "不支持的类型转换，键:" << key;
            }
        }
    }
    camCnf->EditCameraConfig(camId,camJsonData);
    return 0;
}


int MainBackend::updateRecFileList()
{
    emit updateRecFileListSignal();
}
#include <QDesktopServices>
#include <QUrl>
void MainBackend::openFileLocation(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.path()));
}

#include <QProcess>

// 自动搜索VLC播放器路径
QString findVlcPath() {
    // 检查系统PATH环境变量
    QString pathEnv = qgetenv("PATH");
    QStringList pathList = pathEnv.split(QDir::listSeparator());
    foreach (const QString& path, pathList) {
        QString exePath = QDir(path).filePath("vlc.exe");
        if (QFile::exists(exePath)) {
            return exePath;
        }
    }

    // 未找到VLC
    return QString();
}

void MainBackend::deleteFile(const QString& filePath)
{
    if (QFile::exists(filePath)) {
        if (QFile::remove(filePath)) {
            qDebug() << "成功删除文件:" << filePath;
        } else {
            qWarning() << "删除文件失败:" << filePath;
        }
    } else {
        qWarning() << "文件不存在:" << filePath;
    }
}

void MainBackend::playRecFile(const QString& filePath,const QString& usePlaySoftName)
{
    // 检查文件是否存在
    if (!QFile::exists(filePath)) {
        qWarning() << "文件不存在:" << filePath;
        return;
    }

    QProcess* playerProcess = new QProcess(this);
    QString program;
    QStringList arguments;

    if (usePlaySoftName == "vlc") {
        qDebug() << "VLC 播放文件:" << filePath;
        // VLC播放器：尝试自动查找或指定路径
        // 可根据实际情况修改VLC可执行文件路径
//        program = "vlc";  // 若已添加到系统PATH，直接使用程序名
        // 自动查找VLC路径
        program = findVlcPath();
        // 若未添加到PATH，需指定完整路径，例如：
        // program = "C:/Program Files/VideoLAN/VLC/vlc.exe";

        // 添加file:///协议前缀（处理本地文件路径）
        QString urlPath = QUrl::fromLocalFile(filePath).toString();

        // VLC参数：直接播放文件
        arguments << "--play-and-exit" << urlPath;  // 传递带协议的路径
    }
    else if (usePlaySoftName == "ffplay") {
        qDebug() << "ffplay 播放文件:" << filePath;
        // FFplay播放器：尝试自动查找或指定路径
        program = "ffplay";  // 若已添加到系统PATH，直接使用程序名
        // 若未添加到PATH，需指定完整路径，例如：
        // program = "D:/ffmpeg/bin/ffplay.exe";

        // FFplay参数：播放完成后自动退出
        arguments << "-autoexit" << filePath;
    }
    else {
        qWarning() << "不支持的播放器:" << usePlaySoftName;
        delete playerProcess;
        return;
    }

    // 启动播放器
    playerProcess->start(program, arguments);

    // 检查启动是否成功
    if (!playerProcess->waitForStarted()) {
        qWarning() << "启动播放器失败:" << playerProcess->errorString();
        delete playerProcess;
        return;
    }

    // 设置进程结束后自动释放资源
    connect(playerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            playerProcess, &QProcess::deleteLater);
}
