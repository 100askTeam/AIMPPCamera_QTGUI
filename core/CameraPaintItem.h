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
// CameraPaintItem.h
#pragma once
#include <QQuickPaintedItem>
#include <QPointF>
#include <QVector>
#include <QImage>
#include "media/MediaCore.hpp"
#include "updateconfigclient.h"

class CameraPaintItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(QVariantList lines READ lines WRITE setLines NOTIFY linesChanged)
    // 暴露可用区域给QML（QRect在QML中会被转换为rect类型）
    Q_PROPERTY(QRect availableRect READ getAvailableRect NOTIFY availableRectChanged)

    //图像原始尺寸属性
    Q_PROPERTY(QSize imageOriginalSize READ imageOriginalSize NOTIFY imageOriginalSizeChanged)

public:
    explicit CameraPaintItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    QString label() const { return m_label; }
    void setLabel(const QString& txt);

    QVariantList lines() const;
    void setLines(const QVariantList &lst);

    void setImage(QImage img);
    void saveImageForDebug(const QImage &image);

signals:
    void labelChanged();
    void linesChanged();
    void requestUpdate();
    // 当可用区域变化时触发（如窗口大小改变、图像更换）
    void availableRectChanged();
    void imageOriginalSizeChanged();


protected:
    // 重写此方法，当控件大小变化时通知QML更新可用区域
   void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override {
       QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
       emit availableRectChanged(); // 大小变化时触发信号
   }

public slots:
    int startCam(QString camInfo);
    int stopCam(QString camInfo);
    int restartCam(QString camInfo);
    void procUpdate();

    //开始录像
    int startRecord();
    //停止录像
    int stopRecord();

    //保存全部数据
    int saveDataAll(QString jsonDataObj);

    QRect getAvailableRect() const {
        if (m_img.isNull()) return QRect();
        QSize scaledSize = m_img.size().scaled(width(), height(), Qt::KeepAspectRatio);
        return QRect(12, 40, scaledSize.width() - 24, scaledSize.height());
    }

    // 实现
    QSize imageOriginalSize() const {
        return m_img.isNull() ? QSize() : m_img.size();
    }

private:
    QString m_label;
    QVector<QLineF> m_lines;
    QImage m_img;
    MediaState *ms = nullptr;

    //camIp
    QString camName_ = "";
    QString camIP_ = "";
    QString camPort_ = "";

    UpdateConfigClient cli;


    //是否已启动
    bool is_run_flag{false};
};
