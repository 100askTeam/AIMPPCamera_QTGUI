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
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
//#include <QTextCodec>
#include "mainBackend.h"
#include "CameraPaintItem.h"

/**
 * 注册class到qml
 *
 * @brief RegisterClass
 */
void RegisterClassToQml(){
    //注册
    qmlRegisterType<MainBackend>("net.ask100.MainBackend", 1, 0, "MainBackend");
    qmlRegisterType<CameraPaintItem>("net.ask100.CameraPaintItem", 1, 0, "CameraPaintItem");
}


int main(int argc, char *argv[])
{
    // Qt全局编码设置
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QGuiApplication app(argc, argv);

    RegisterClassToQml();

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
