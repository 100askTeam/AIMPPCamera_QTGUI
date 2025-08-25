import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import net.ask100.CameraPaintItem 1.0

Rectangle {
    id: root
    property string title: "摄像头"
    property alias paintItem: camPaint
    property bool enableDraw: false
    property int drawMode: 0                   // 1:直线 2:多边形 0:无
    property var linePoints: []
    property var polyPoints: []
    property color lineColor: "red"
    property color polyColor: "yellow"
    property var lines: []

    Layout.fillWidth: true
    Layout.fillHeight: true
    //color: "transparent"
    //color: "#b9b9b9"
//    border.color: "#b9b9b9"

    property bool selected: false
    border.color: selected ? "#FF5722" : "#b9b9b9"
    border.width: selected ? 3 : 1

    // 定义绘制结束的信号：用于子组件与父组件通讯传递消息
    signal drawingFinished()

    onDrawModeChanged:{
         console.log(title,"--->CameraView drawMode updated:", drawMode,",enableDraw=",root.enableDraw)  // 增加日志验证
    }

    function selectDrawMode()
    {
        root.linePoints = []
        root.lines = [] // 清空所有线条
        root.polyPoints = []
        root.lines = [] // 清空所有线条

        drawCanvas.requestPaint()
    }

    function startCam(cameraInfo)
    {
        console.log(title,"--->正在启动...",JSON.stringify(cameraInfo))
        camPaint.startCam(JSON.stringify(cameraInfo));
    }


    function stopCam(cameraInfo)
    {
        console.log(title,"--->正在关闭...",JSON.stringify(cameraInfo))
        camPaint.stopCam(JSON.stringify(cameraInfo));
    }

    function restartCam(cameraInfo)
    {
        console.log(title,"--->正在重新启动...",JSON.stringify(cameraInfo))
        camPaint.restartCam(JSON.stringify(cameraInfo));
    }

    function getDetectionInfo(modeId)
    {
        console.log("getDetectionInfo--->modeId：",modeId)
        //越界检测坐标
        if(modeId === 1)
        {
            console.log(title,"--->正在Get越界检测坐标...",JSON.stringify(root.linePoints))
            return JSON.stringify(root.linePoints)
        }
        //区域入侵坐标
        else if(modeId === 2)
        {
            console.log(title,"--->正在Get区域入侵坐标...",JSON.stringify(root.polyPoints))
            return JSON.stringify(root.polyPoints)
        }
    }

    function recordBtnClicked(recType)
    {
        if(recType === "start")
        {
            console.log("开始录像")
            camPaint.startRecord();
        }else if(recType === "stop")
        {
            console.log("停止录像")

            camPaint.stopRecord();
        }
    }

    function saveDataAll(data)
    {
        console.log("saveDataAll,", data)
        camPaint.saveDataAll(JSON.stringify(data))
    }

    Text {
        text: root.title
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 8
        anchors.topMargin: 8
        font.pointSize: 11
        //color: "white"
    }

    CameraPaintItem {
        z: 1
        id: camPaint
        anchors.fill: parent
        lines: root.lines
    }

//    MouseArea {
//        id: drawArea
//        anchors.fill: parent
////        enabled: root.enableDraw && (root.drawMode === 1 || root.drawMode === 2)
//        enabled: root.enableDraw
//        property bool ready: false
//        acceptedButtons: Qt.LeftButton | Qt.RightButton

//        onClicked: {
//            console.log(title,"--->onClicked .root.drawMode=",root.drawMode);

//            const availableRect = camPaint.availableRect;
//            const imgSize = camPaint.imageOriginalSize;
//            if (imgSize.width === 0 || imgSize.height === 0) return;

//            // 1. 计算有效区域内的相对坐标（减去边距偏移）
//            const relX = mouse.x - availableRect.x; // 相对于有效区域左边界的X
//            const relY = mouse.y - availableRect.y; // 相对于有效区域上边界的Y

//            // 2. 计算缩放比例（有效区域尺寸 / 图像原始尺寸）
//            const scaleX = availableRect.width / imgSize.width;
//            const scaleY = availableRect.height / imgSize.height;

//            // 3. 转换为图像原始坐标（相对坐标 / 缩放比例）
//            const originalX = relX / scaleX;
//            const originalY = relY / scaleY;


//            if (root.drawMode === 1) { // 直线
//                 if (root.linePoints.length < 2) {
//                    console.log(title,"正在绘制直线，点数量小于2个，绘制点,root.linePoints.length=",root.linePoints.length);
//                    root.linePoints.push(Qt.point(originalX, originalY))
//                    var arr = root.lines
//                    arr.push([root.linePoints[0], root.linePoints[0]])
//                    root.lines = arr
//                  } else if (root.linePoints.length === 2) {
//                    var newLine = [root.linePoints[0], root.linePoints[1]]
//                    var arr = root.lines
//                    arr.push(newLine)
//                    root.lines = arr
//                    root.drawMode = -1
//                  }
//            } else if (root.drawMode === 2) { // 多边形
////                root.polyPoints.push(Qt.point(mouse.x, mouse.y))
//                root.polyPoints.push(Qt.point(originalX, originalY));
//                var arr = root.lines
//                if (root.polyPoints.length === 1) {
//                    arr.push([root.polyPoints[0], root.polyPoints[0]]);
//                } else {
//                    var newLine = [root.polyPoints[root.polyPoints.length - 2], root.polyPoints[root.polyPoints.length - 1]];
//                    arr.push(newLine)
//                }
//                root.lines = arr
//                // 不再需要手动发射信号，QML会自动处理
//            }
//        }

//        onDoubleClicked: {
//            if (root.drawMode === 2 && root.polyPoints.length > 2) {
//                root.drawMode = -1
//                // 不再需要手动发射信号，QML会自动处理
//            }
//        }

//        onPressed: {
//            if (mouse.button === Qt.RightButton && root.drawMode === 2 && root.polyPoints.length > 2) {
//                root.drawMode = -1
//                // 不再需要手动发射信号，QML会自动处理
//            }
//        }
//    }

//    MouseArea {
//        id: drawArea
//        anchors.fill: parent
//        enabled: root.enableDraw
//        property bool ready: false
//        acceptedButtons: Qt.LeftButton | Qt.RightButton

//        onClicked: {
//            console.log(title,"--->onClicked .root.drawMode=",root.drawMode);

//            const availableRect = camPaint.availableRect;
//            const imgSize = camPaint.imageOriginalSize;
//            if (imgSize.width === 0 || imgSize.height === 0) return;

//            // 1. 计算有效区域内的相对坐标（减去边距偏移）
//            const relX = mouse.x - availableRect.x;
//            const relY = mouse.y - availableRect.y;

//            // 2. 计算缩放比例
//            const scaleX = availableRect.width / imgSize.width;
//            const scaleY = availableRect.height / imgSize.height;

//            // 3. 转换为图像原始坐标
//            const originalX = relX / scaleX;
//            const originalY = relY / scaleY;


//            if (root.drawMode === 1) { // 直线
//                 if (root.linePoints.length < 2) {
//                    console.log(title,"正在绘制直线，点数量小于2个，绘制点,root.linePoints.length=",root.linePoints.length);
//                    root.linePoints.push(Qt.point(originalX, originalY))
//                    var arr = root.lines
//                    arr.push([root.linePoints[0], root.linePoints[0]])
//                    root.lines = arr
//                  } else if (root.linePoints.length === 2) {
//                    var newLine = [root.linePoints[0], root.linePoints[1]]
//                    var arr = root.lines
//                    arr.push(newLine)
//                    root.lines = arr
//                    root.drawMode = -1
//                  }
//            } else if (root.drawMode === 2) { // 多边形（限制4个点）
//                // 限制最多4个点，超过则不处理
//                if (root.polyPoints.length < 4) {
//                    root.polyPoints.push(Qt.point(originalX, originalY));
//                    var arr = root.lines

//                    // 绘制当前点与上一点的连线
//                    if (root.polyPoints.length === 1) {
//                        arr.push([root.polyPoints[0], root.polyPoints[0]]);
//                    } else {
//                        var newLine = [
//                            root.polyPoints[root.polyPoints.length - 2],
//                            root.polyPoints[root.polyPoints.length - 1]
//                        ];
//                        arr.push(newLine);
//                    }

//                    // 当达到4个点时，自动闭合多边形并结束绘制
//                    if (root.polyPoints.length === 4) {
//                        // 添加第4个点到第1个点的连线（闭合多边形）
//                        arr.push([root.polyPoints[3], root.polyPoints[0]]);
//                        root.lines = arr;
//                        // 结束绘制模式
//                        root.drawMode = -1;
//                        console.log("已绘制4个点，自动闭合多边形并结束绘制");
//                    } else {
//                        root.lines = arr;
//                    }
//                } else {
//                    console.log("已达到最大4个点，无法继续添加");
//                }
//            }
//        }

//        onDoubleClicked: {
//            // 保持双击结束绘制的功能（适用于少于4个点的情况）
//            if (root.drawMode === 2 && root.polyPoints.length > 2) {
//                // 双击时闭合多边形（如果已有2个以上点）
//                root.lines.push([root.polyPoints[root.polyPoints.length - 1], root.polyPoints[0]]);
//                root.drawMode = -1;
//                console.log("双击闭合多边形并结束绘制");
//            }
//        }

//        onPressed: {
//            if (mouse.button === Qt.RightButton && root.drawMode === 2 && root.polyPoints.length > 2) {
//                // 右键闭合多边形（适用于少于4个点的情况）
//                root.lines.push([root.polyPoints[root.polyPoints.length - 1], root.polyPoints[0]]);
//                root.drawMode = -1;
//                console.log("右键闭合多边形并结束绘制");
//            }
//        }
//    }


    MouseArea {
        id: drawArea
        anchors.fill: parent
        enabled: root.enableDraw
        property bool ready: false
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            console.log(title,"--->onClicked .root.drawMode=",root.drawMode);

            const availableRect = camPaint.availableRect;
            const imgSize = camPaint.imageOriginalSize;
            if (imgSize.width === 0 || imgSize.height === 0) return;

            // 1. 计算有效区域内的相对坐标（减去边距偏移）
            const relX = mouse.x - availableRect.x;
            const relY = mouse.y - availableRect.y;

            // 2. 计算缩放比例
            const scaleX = availableRect.width / imgSize.width;
            const scaleY = availableRect.height / imgSize.height;

            // 3. 转换为图像原始坐标
            const originalX = relX / scaleX;
            const originalY = relY / scaleY;


            if (root.drawMode === 1) { // 直线（优化：第2个点自动连接并结束）
                root.polyPoints = [];
                // 限制最多2个点，超过则不处理
                if (root.linePoints.length < 2) {
                    // 添加当前点到坐标数组
                    root.linePoints.push(Qt.point(originalX, originalY));
                    console.log("直线绘制：添加第", root.linePoints.length, "个点");

                    // 当添加第2个点时，自动连接成线段并结束绘制
                    if (root.linePoints.length === 2) {
                        // 清除临时线条，添加实际线段（第1点到第2点）
                        var arr = root.lines;
                        // 移除之前的临时点（如果有）
                        if (arr.length > 0 && arr[arr.length - 1][0] === arr[arr.length - 1][1]) {
                            arr.pop();
                        }
                        // 添加正式线段
                        arr.push([root.linePoints[0], root.linePoints[1]]);
                        root.lines = arr;

                        // 结束绘制模式
                        root.drawMode = -1;
                        console.log("已绘制2个点，自动连接成线段并结束绘制");

                        // 直线绘制结束，触发信号通知父组件
                        root.drawingFinished(1, root.polyPoints);
                    } else {
                        // 第1个点时，添加临时点（自身到自身，便于预览）
                        var tempArr = root.lines;
                        tempArr.push([root.linePoints[0], root.linePoints[0]]);
                        root.lines = tempArr;
                    }

                    // 请求重绘以显示完整线段
                    drawCanvas.requestPaint();
                } else {
                    console.log("直线已达到最大2个点，无法继续添加");
                }
            } else if (root.drawMode === 2) { // 多边形（保持之前的优化逻辑）
                root.linePoints = [];
                if (root.polyPoints.length < 4) {
                    root.polyPoints.push(Qt.point(originalX, originalY));
                    var arr = root.lines;

                    if (root.polyPoints.length === 1) {
                        arr.push([root.polyPoints[0], root.polyPoints[0]]);
                    } else {
                        var newLine = [
                            root.polyPoints[root.polyPoints.length - 2],
                            root.polyPoints[root.polyPoints.length - 1]
                        ];
                        arr.push(newLine);
                    }

                    if (root.polyPoints.length === 4) {
                        arr.push([root.polyPoints[3], root.polyPoints[0]]);
                        root.lines = arr;
                        root.drawMode = -1;
                        console.log("已绘制4个点，自动闭合多边形并结束绘制");

                        // 直线绘制结束，触发信号通知父组件
                        root.drawingFinished();
                    } else {
                        root.lines = arr;
                    }

                    // 请求重绘以显示完整线段
                    drawCanvas.requestPaint();
                } else {
                    console.log("已达到最大4个点，无法继续添加");
                }
            }
        }

        onDoubleClicked: {
            if (root.drawMode === 2 && root.polyPoints.length > 2) {
                root.lines.push([root.polyPoints[root.polyPoints.length - 1], root.polyPoints[0]]);
                root.drawMode = -1;
                console.log("双击闭合多边形并结束绘制");
            }
        }

        onPressed: {
            if (mouse.button === Qt.RightButton && root.drawMode === 2 && root.polyPoints.length > 2) {
                root.lines.push([root.polyPoints[root.polyPoints.length - 1], root.polyPoints[0]]);
                root.drawMode = -1;
                console.log("右键闭合多边形并结束绘制");
            }
        }
    }

    Canvas {
        id: drawCanvas
        anchors.fill: parent
        z: 2

        // 样式常量
        property int lineWidth: 2
        property int pointRadius: 5  // 点的半径
        property string defaultLineColor: root.lineColor
        property string defaultPolyColor: root.polyColor

        onPaint: {
            const ctx = getContext("2d");
            ctx.resetTransform();
            ctx.clearRect(0, 0, width, height);

            const availableRect = camPaint.availableRect;
            const imgSize = camPaint.imageOriginalSize;

            // 核心校验
            if (availableRect.isEmpty || imgSize.width <= 0 || imgSize.height <= 0) {
                console.log("[绘制终止] 无效区域或空图像");
                return;
            }

            // 计算缩放比例
            const scaleX = availableRect.width / imgSize.width;
            const scaleY = availableRect.height / imgSize.height;

            // 裁剪到有效区域
            ctx.beginPath();
            ctx.rect(availableRect.x, availableRect.y, availableRect.width, availableRect.height);
            ctx.clip();

            // 坐标转换工具函数
            function toDrawCoord(originalPoint) {
                return {
                    x: originalPoint.x * scaleX + availableRect.x,
                    y: originalPoint.y * scaleY + availableRect.y
                };
            }

            // 1. 绘制直线（重点修复第一个点预览）
            if (root.linePoints.length > 0) {

                // 无论几个点，先设置样式
                ctx.strokeStyle = defaultLineColor;
                ctx.fillStyle = defaultLineColor;  // 填充色与线条色一致
                ctx.lineWidth = lineWidth;

                // 转换第一个点坐标
                const p1Draw = toDrawCoord(root.linePoints[0]);

                // 绘制第一个点（始终显示，无论是1个还是2个点的情况）
                ctx.beginPath();
                ctx.arc(p1Draw.x, p1Draw.y, pointRadius, 0, 2 * Math.PI);
                ctx.fill();  // 填充圆点
                ctx.stroke();  // 绘制边框
                console.log("[直线点预览] 第1个点坐标:", JSON.stringify(p1Draw));

                // 如果有第二个点，绘制线段
                if (root.linePoints.length === 2) {
                    const p2Draw = toDrawCoord(root.linePoints[1]);

                    // 绘制第二个点
                    ctx.beginPath();
                    ctx.arc(p2Draw.x, p2Draw.y, pointRadius, 0, 2 * Math.PI);
                    ctx.fill();
                    ctx.stroke();
                    console.log("[直线点预览] 第2个点坐标:", JSON.stringify(p2Draw));

                    // 绘制连接线段
                    ctx.beginPath();
                    ctx.moveTo(p1Draw.x, p1Draw.y);
                    ctx.lineTo(p2Draw.x, p2Draw.y);
                    ctx.stroke();
                    console.log("[直线绘制] 线段已连接");
                }
            }

            // 2. 绘制多边形
            if (root.polyPoints.length > 0) {
                ctx.strokeStyle = defaultPolyColor;
                ctx.fillStyle = defaultPolyColor;
                ctx.lineWidth = lineWidth;
                ctx.beginPath();

                // 绘制所有点并连接
                const firstDraw = toDrawCoord(root.polyPoints[0]);
                ctx.moveTo(firstDraw.x, firstDraw.y);

                // 绘制第一个点
                ctx.beginPath();
                ctx.arc(firstDraw.x, firstDraw.y, pointRadius, 0, 2 * Math.PI);
                ctx.fill();
                ctx.stroke();

                for (let i = 1; i < root.polyPoints.length; i++) {
                    const pDraw = toDrawCoord(root.polyPoints[i]);
                    // 绘制当前点
                    ctx.beginPath();
                    ctx.arc(pDraw.x, pDraw.y, pointRadius, 0, 2 * Math.PI);
                    ctx.fill();
                    ctx.stroke();
                    // 连接到上一个点
                    ctx.beginPath();
                    ctx.moveTo(toDrawCoord(root.polyPoints[i-1]).x, toDrawCoord(root.polyPoints[i-1]).y);
                    ctx.lineTo(pDraw.x, pDraw.y);
                    ctx.stroke();
                }

                // 闭合多边形（如果绘制完成）
                if (root.drawMode === -1 && root.polyPoints.length >= 3) {
                    ctx.beginPath();
                    ctx.moveTo(toDrawCoord(root.polyPoints[root.polyPoints.length-1]).x,
                               toDrawCoord(root.polyPoints[root.polyPoints.length-1]).y);
                    ctx.lineTo(firstDraw.x, firstDraw.y);
                    ctx.stroke();
                    console.log("[多边形闭合] 自动连接终点到起点");
                }
            }

            console.log("[绘制完成] 直线点数量:", root.linePoints.length, "多边形点数量:", root.polyPoints.length);
        }
    }

    Connections {
        target: root
        onLinePointsChanged: drawCanvas.requestPaint()
        onPolyPointsChanged: drawCanvas.requestPaint()
        onDrawModeChanged: {
            if (root.drawMode === 1)
            {
                root.linePoints = []
                root.lines = [] // 清空所有线条
            }
            if (root.drawMode === 2) {
                root.polyPoints = []
                root.lines = [] // 清空所有线条
            }

            drawCanvas.requestPaint()
        }
        onAvailableRectChanged: drawCanvas.requestPaint()
    }
}
