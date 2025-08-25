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
// CameraPaintItem.cpp
#include "CameraPaintItem.h"
#include <QPainter>
#include <QVariant>
#include <QDebug>
#include <QThread>

CameraPaintItem::CameraPaintItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
//    ,m_img("E:\\project\\QTProject\\baiwenwang_qt\\logo.100ask.png")
    setFlag(ItemHasContents, true);
    
    
    QObject::connect(this, &CameraPaintItem::requestUpdate,
                     this, &CameraPaintItem::procUpdate);

    
}

void CameraPaintItem::paint(QPainter *painter)
{
    // 设置抗锯齿
    painter->setRenderHint(QPainter::Antialiasing);

    // 先绘制图像
    if (!m_img.isNull()) {
//        qDebug() << "正在绘制image=[" << m_img.width() <<"x"<< m_img.height() << "]";
        // 1. 计算缩放后的图像大小（核心：提前确定基础尺寸）
        QSize scaledSize = m_img.size().scaled(width(), height(), Qt::KeepAspectRatio);

        // 2. 计算可用绘制区域（targetRect：图像实际显示的区域，即边框内的有效区域）
        // （这里12是左右边距，40是顶部边距，确保图像在控件内有合理留白）
        QRect targetRect(12, 40, scaledSize.width()-12-12, scaledSize.height());
        //targetRect.moveCenter(rect().center()); // 居中显示

        // 3. 设置裁剪区域：限制所有绘制操作仅在targetRect内生效
        painter->setClipRect(targetRect);

        // 绘制图像（在可用区域内）
        painter->drawImage(targetRect, m_img);


        // --------------- 绘制蓝色边框（图像外部）---------------
        // 边框线宽（可调整）
        int borderWidth = 3;
        // 边框颜色（蓝色）
        painter->setPen(QPen(Qt::blue, borderWidth));
        // 边框位置：在图像区域外围，留出1-2像素间距（避免与图像重叠）
        QRect borderRect = targetRect.adjusted(
            -borderWidth - 1,  // 左边向左扩展（线宽+间距）
            -borderWidth - 1,  // 上边向上扩展
            borderWidth + 1,   // 右边向右扩展
            borderWidth + 1    // 下边向下扩展
        );
        // 绘制矩形边框
        painter->drawRect(borderRect);
        // ------------------------------------------------------

        // 保存当前变换状态
        painter->save();

        //将原点移到targetRect的左上角，后续绘制坐标以该点为(0,0)
        painter->translate(targetRect.topLeft());

        // 缩放：将图像原始坐标映射到当前显示尺寸（确保绘制比例正确）
//        painter->scale((qreal)scaledSize.width() / m_img.width(),
//                      (qreal)scaledSize.height() / m_img.height());

        qreal scaleX = (qreal)targetRect.width() / m_img.width();
        qreal scaleY = (qreal)targetRect.height() / m_img.height();
        painter->scale(scaleX, scaleY);

        // 在图像上绘制线条（使用图像原始坐标）
        painter->setPen(QPen(Qt::red, 2/scaleX));
        painter->setPen(QPen(Qt::red, 2/scaleX));
        for (const QLineF &line : m_lines) {
            painter->drawLine(line);
        }

        // 恢复坐标系
        painter->restore();
    } else {
        // 图像为空时的默认绘制
        qDebug() << "img is null.";
        //painter->fillRect(rect(), Qt::black);
        painter->setPen(Qt::white);
        //painter->drawText(rect(), Qt::AlignCenter, "No Image");
    }

    // 绘制标签（始终在最上层）
    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 11));
    painter->drawText(QPointF(8, 20), m_label);
}

void CameraPaintItem::setLabel(const QString& txt)
{
    if (m_label != txt) {
        m_label = txt;
        update();
        emit labelChanged();
    }
}

QVariantList CameraPaintItem::lines() const
{
    QVariantList lst;
    for (const QLineF &line : m_lines) {
        QVariantList pair;
        pair << line.p1() << line.p2();
        lst << QVariant::fromValue(pair);
    }
    return lst;
}

void CameraPaintItem::setLines(const QVariantList &lst)
{
    QVector<QLineF> v;
    for (const QVariant &var : lst) {
        QVariantList pair = var.toList();
        if (pair.size() == 2) {
            QPointF p1 = pair[0].toPointF();
            QPointF p2 = pair[1].toPointF();
            v << QLineF(p1, p2);
        }
    }
    m_lines = v;
    update();
    emit linesChanged();
}


int CameraPaintItem::startCam(QString camInfo)
{
   qDebug() << "run startCam().camInfo=" <<  camInfo;

   if(is_run_flag)
   {
       qDebug() << "已启动，无需再次启动.";
       return -1;
   }

   is_run_flag = true;

   // 将QString转换为QByteArray
     QByteArray jsonData = camInfo.toUtf8();

     // 解析JSON字符串
     QJsonParseError parseError;
     QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

     // 检查解析是否成功
     if (parseError.error != QJsonParseError::NoError) {
         qDebug() << "JSON解析错误:" << parseError.errorString();
         return -1; // 解析失败返回错误码
     }

     // 转换为QJsonObject
     if (!jsonDoc.isObject()) {
         qDebug() << "JSON不是一个对象";
         return -2;
     }

     QJsonObject jsonObj = jsonDoc.object();

     // 现在可以使用jsonObj获取各种字段了
     // 示例: 获取名为"cameraId"的字段
     if (jsonObj.contains("camName")) {
         QJsonValue camName = jsonObj["camName"];
         qDebug() << "camName:" << camName.toString();
         camName_ = camName.toString();
     }

     if (jsonObj.contains("camIP")) {
         QJsonValue camIP = jsonObj["camIP"];
         qDebug() << "camIP:" << camIP.toString();
         camIP_ = camIP.toString();
     }

     if (jsonObj.contains("camPort")) {
         QJsonValue camPort = jsonObj["camPort"];
         qDebug() << "camPort:" << camPort.toString();
         camPort_ = camPort.toString();
     }

     // 根据需要处理其他字段...
   cli.InitClient(camIP_.toStdString(), camPort_.toInt());
   int ret  = cli.SendStartCMD();
   qDebug() << "SendStartCMD ret:" << ret;
   if(ret == 0)
   {
       std::thread thr([this]{
           QThread::sleep(5);
           //不断的尝试
//           for (int i = 0;i<5;i++)
           {
               //open stream
               std::string filename = "rtsp://"+camIP_.toStdString()+":8554/ch0";
//               qDebug() << "run MEDIA_stream_open:" << i << "num,connect:" << filename.c_str();

               if(!ms)
               {
                   qDebug() << "run MEDIA_stream_open:" << filename.c_str();
                   ms = MEDIA_stream_open(MEDIA_TYPE_MEDIA_PLAY,filename, this);
               }
           }
       });
       thr.detach();
   }

   return 0;
}

int CameraPaintItem::stopCam(QString camInfo)
{
    qDebug() << "run stopCam().";
    if(!is_run_flag)
    {
        qDebug() << "还没启动，无法停止.";
        return -1;
    }
    is_run_flag = false;

    if(ms)
    {
        ms->exit_request = 1;
        av_free(ms);
        ms = nullptr;
    }

    cli.SendStopCMD();
    return 0;
}

int CameraPaintItem::restartCam(QString camInfo)
{
    qDebug() << "run restartCam().";
    if(!is_run_flag)
    {
        qDebug() << "还没启动，无法停止.";
        return -1;
    }

    //断开流
    if(ms)
    {
        ms->exit_request = 1;
        av_free(ms);
        ms = nullptr;
    }


    cli.SendReStartCMD();

    std::thread thr([this]{
        QThread::sleep(5);
        {
            //open stream
            std::string filename = "rtsp://"+camIP_.toStdString()+":8554/ch0";
            if(!ms)
            {
                qDebug() << "run MEDIA_stream_open:" << filename.c_str();

                ms = MEDIA_stream_open(MEDIA_TYPE_MEDIA_PLAY,filename, this);
            }
        }
    });
    thr.detach();
    return 0;
}

static QList<QPointF> PointJsonStr2QList(QString pointsJsonStr)
{
    QList<QPointF> points; // 用于存储解析后的坐标点

    // 1. 将JSON字符串转换为QJsonDocument
   QJsonDocument doc = QJsonDocument::fromJson(pointsJsonStr.toUtf8());
   if (doc.isNull()) {
       qDebug() << "JSON解析失败";
       return points; // 解析失败返回错误码
   }

   // 2. 确认是JSON数组（因为数据是数组形式）
   if (!doc.isArray()) {
       qDebug() << "JSON格式错误，不是数组";
       return points;
   }
   QJsonArray jsonArray = doc.array();

   // 3. 遍历数组，提取每个点的x和y坐标
   foreach (const QJsonValue &val, jsonArray) {
       if (val.isObject()) {
           QJsonObject obj = val.toObject();
           // 提取x和y（注意JSON中是double类型）
           double x = obj["x"].toDouble();
           double y = obj["y"].toDouble();
           points.append(QPointF(x, y)); // 添加到列表
       }
   }

   return points;
}

//开始录像
int CameraPaintItem::startRecord()
{
    qDebug() << "run startRecord()";
    if(ms)
    {
        qDebug() << "run MEDIA_stream_Record: REC_CTRL_START";
        MEDIA_stream_Record(ms,REC_CTRL_START);
    }
}

//停止录像
int CameraPaintItem::stopRecord()
{
    qDebug() << "run stopRecord()";
    if(ms)
    {
        qDebug() << "run MEDIA_stream_Record: REC_CTRL_STOP";
        MEDIA_stream_Record(ms,REC_CTRL_STOP);
    }
}

int CameraPaintItem::saveDataAll(QString jsonDataObj)
{
    qDebug() << "run saveDataAll()--------->" << jsonDataObj;

    //解析json
    QJsonDocument doc = QJsonDocument::fromJson(jsonDataObj.toUtf8());
    if (!doc.isObject()) {
        qDebug() << "JSON格式错误，不是对象";
        return -1;
    }
    QJsonObject jsonObj = doc.object();

    // 处理jsonObj中的数据

    // 四种情况：
    // run saveDataAll()---------> "{\"resolutionId\":0,\"aiFuncId\":0}"
    // run saveDataAll()---------> "{\"resolutionId\":0,\"aiFuncId\":1,\"audio_switch\":0,\"audio_val\":3}"
    // run saveDataAll()---------> "{\"resolutionId\":0,\"aiFuncId\":2,\"audio_switch\":0,\"audio_val\":3,\"jinaceXuanxiang\":1,\"detectionInfo\":\"[]\"}"
    // run saveDataAll()---------> "{\"resolutionId\":0,\"aiFuncId\":2,\"audio_switch\":0,\"audio_val\":3,\"jinaceXuanxiang\":0,\"detectionInfo\":\"[]\"}"
    ConfigParams cp;
    memset(&cp,0, sizeof (cp));

    // 解析 resolutionId
    int resolutionId = jsonObj.value("resolutionId").toInt();
    qDebug() << "resolutionId:" << resolutionId;
    int w,h;
    //default
    w = 1920;
    h = 1080;
    if(resolutionId == 0)
    {
        qDebug() << "设置的分辨率: 1920 x 1080";
        w = 1920;
        h = 1080;
    }else if(resolutionId == 1)
    {
        qDebug() << "设置的分辨率: 1280 x 720";
        w = 1280;
        h = 720;
    }else if(resolutionId == 2)
    {
        qDebug() << "设置的分辨率: 640 × 480";
        w = 640;
        h = 480;
    }

    cp.main_pic_width = w;
    cp.main_pic_height = h;
    //默认打开
    cp.nn_enable = 1;

    // 解析 aiFuncId
    int aiFuncId = jsonObj.value("aiFuncId").toInt();//0:关闭AI功能,1:开启行人检测,2:开启目标检测
    if(aiFuncId == 0)//0:关闭AI功能
    {
        //关闭
        cp.nn_enable = 0;
    }else if(aiFuncId == 1)//1:开启行人检测
    {
        {
            cp.nn_type = 0;//人形检测
            cp.nn_drawOrl_enable = 1;

            cp.nn_alarm_enable = 1;//区域入侵检测

            // 解析可选 audio_switch/audio_val
            int audio_switch = jsonObj.contains("audio_switch") ? jsonObj.value("audio_switch").toInt() : -1;
            int audio_val = jsonObj.contains("audio_val") ? jsonObj.value("audio_val").toInt() : -1;
            qDebug() << "audio_switch:" << audio_switch;
            qDebug() << "audio_val:" << audio_val;

            cp.nn_alarm_volume = audio_switch == 0 ? audio_val : 0;
        }

        {
            // 解析可选 jinaceXuanxiang
            int jinaceXuanxiang = jsonObj.contains("jinaceXuanxiang") ? jsonObj.value("jinaceXuanxiang").toInt() : -1;

            // 解析可选 detectionInfo，一般是字符串如 "[]"
            QString detectionInfo;
            if (jsonObj.contains("detectionInfo")) {
                detectionInfo = jsonObj.value("detectionInfo").toString();
            }

            QList<QPointF> points = PointJsonStr2QList(detectionInfo);

            qDebug() << "解析后的坐标点数量：" << points.size();

            for (const QPointF &p : points) {
                qDebug() << "x:" << p.x() << " y:" << p.y();
            }

            //解析越界检测坐标
            if(jinaceXuanxiang == 0)
            {
                cp.nn_alarm_enable = 1;
                cp.nn_alarm_mode = 0;//0:越界检测 1：区域入侵检测
                if(points.size() != 2)
                {
                    qDebug() << "绘制的线段坐标点数量不合法,需要必须等于2点坐标.";
                    return -1;
                }

                cp.nn_alarm_lines_x1 = points[0].x();
                cp.nn_alarm_lines_y1 = points[0].y();
                cp.nn_alarm_lines_x2 = points[1].x();
                cp.nn_alarm_lines_y2 = points[1].y();
            }
            //解析区域入侵坐标
            else if(jinaceXuanxiang == 1)
            {
                cp.nn_alarm_enable = 1;
                cp.nn_alarm_mode = 1;//0:越界检测 1：区域入侵检测
                if(points.size() != 4)
                {
                    qDebug() << "绘制的线段坐标点数量不合法,需要必须等于4点坐标.";
                    return -1;
                }
                int processCount = qMin(points.size(), 4);
                for (int i = 0; i < processCount; ++i) {
                      int x = qRound(points[i].x());
                      int y = qRound(points[i].y());

                      switch(i) {
                          case 0:
                              cp.nn_alarm_rects_x0 = x;
                              cp.nn_alarm_rects_y0 = y;
                              break;
                          case 1:
                              cp.nn_alarm_rects_x1 = x;
                              cp.nn_alarm_rects_y1 = y;
                              break;
                          case 2:
                              cp.nn_alarm_rects_x2 = x;
                              cp.nn_alarm_rects_y2 = y;
                              break;
                          case 3:
                              cp.nn_alarm_rects_x3 = x;
                              cp.nn_alarm_rects_y3 = y;
                              break;
                          default:
                              break;
                      }
                  }
            }
        }
    }else if(aiFuncId == 2)//2:开启目标检测
    {
        cp.nn_type = 2;//目标检测
        cp.nn_drawOrl_enable = 1;

        // 解析可选 audio_switch/audio_val
        int audio_switch = jsonObj.contains("audio_switch") ? jsonObj.value("audio_switch").toInt() : -1;
        int audio_val = jsonObj.contains("audio_val") ? jsonObj.value("audio_val").toInt() : -1;
        qDebug() << "audio_switch:" << audio_switch;
        qDebug() << "audio_val:" << audio_val;

        cp.nn_alarm_volume = audio_switch == 0 ? audio_val : 0;
    }

    // TODO: 根据解析到的数据做后续处理
    // 例如设置成员变量，保存到文件，或者调用其它方法

    cli.SendDataAllConfig(cp);
    return 0;
}

void CameraPaintItem::procUpdate()
{
    update();
}

void CameraPaintItem::setImage(QImage img)
{
//     qDebug() << "setImage() 在GUI线程处理图像，线程ID:" << QThread::currentThreadId();
//    qDebug() << "收到image=[" << img.width() <<"x"<< img.height() << "]的AI功能";
//    saveImageForDebug(img);
//    m_img = img.copy();
    m_img = std::move(img);
    emit imageOriginalSizeChanged(); // 新增
    emit requestUpdate();
}


#include <QDateTime>
#include <QDir>

void CameraPaintItem::saveImageForDebug(const QImage &image)
{
    if (image.isNull()) {
        qDebug() << "错误: 尝试保存空图像";
        return;
    }

    // 生成唯一的文件名（包含时间戳）
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    QString filePath = QString("debug_image_%1.png").arg(timestamp);

    // 保存图像
    bool saved = image.save(filePath, "PNG");

    if (saved) {
        qDebug() << "调试图像已保存到:" << QDir::currentPath() + "/" + filePath;
    } else {
        qDebug() << "错误: 无法保存图像，可能原因:";
//                 << image.lastErrorString();
    }
}

