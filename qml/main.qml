import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import net.ask100.MainBackend 1.0
import net.ask100.CameraPaintItem 1.0
import QtQml 2.12  // 基础 QML 模块，包含 QUrl 等核心类型
//ListModel
import QtQml.Models 2.12

import "./main.js" as MainFunc

Window {
    id:mainWindow
    width: 1920
    height: 1080
    minimumWidth: 1920
    maximumWidth: 1920
    minimumHeight: 1080
    maximumHeight: 1080
    visible: true
    title: qsTr("100Ask")



    property int drawMode: 0 // 0:无, 1:越界检测(直线), 2:区域入侵检测(多边形)
    property color lineColor: "red"
    property color polyColor: "blue"


    // 定义默认颜色变量（便于统一修改）
    property color defaultColor: "#f0f0f0"  // 默认背景色（浅灰）
    property color pressedColor: "#f9f8f9"    // 按下时的颜色

    //分辨率列表
    property var resolutionList : ["1080P", "720P", "640P"];

    //AI功能列表
    property var aiFuncList : ["关闭AI功能","打开行人检测","打开目标检测" ];



    // 正确声明 ListModel
    ListModel {
        id: recFileListModel
    }

    MainBackend {
        id: backend
        onUpdateRecFileListSignal:{
            //收到C++后端更新录像文件列表的信号
            console.log("C++信号:更新录像文件列表")
            recFileListModel.clear();

            const fileList = backend.getRecFileList;
            for (var i = 0; i < fileList.length; i++) {
                const fullPath = fileList[i];
                console.log("文件:", fullPath);

                // 替换原来的 QUrl 用法
                // 处理 Windows 路径（\）和 Unix 路径（/）
                let separator = fullPath.indexOf('/') !== -1 ? '/' : '\\';
                let fileName = fullPath.split(separator).pop(); // 分割路径并取最后一部分

                // 存入模型：同时保存完整路径和文件名
                recFileListModel.append({
                                            filePath: fullPath,  // 完整路径（用于右键菜单操作）
                                            fileName: fileName  // 仅文件名（用于显示）
                                        });
            }
        }
    }



    // 窗口打开后的初始化函数
    function initAfterWindowOpen() {
        console.log("执行窗口打开后的初始化逻辑");
        var cam0Info = backend.getCameraInfo(0);
        MainFunc.initCamViewInfo(camView1,cam0Info);
        var cam1Info = backend.getCameraInfo(1);
        MainFunc.initCamViewInfo(camView2,cam1Info);
        var cam2Info = backend.getCameraInfo(2);
        MainFunc.initCamViewInfo(camView3,cam2Info);
        var cam3Info = backend.getCameraInfo(3);
        MainFunc.initCamViewInfo(camView4,cam3Info);

        //初始化rec files
        backend.updateRecFileList();


        //显示与隐藏
        initAIFuncSettingView();
    }

    // 监听窗口可见性变化
    onVisibleChanged: {
        if (visible) {  // 当窗口变为可见时
            console.log("窗口已打开并显示");
            // 在这里执行窗口打开后的初始化操作
            initAfterWindowOpen();
        } else {
            console.log("窗口已隐藏");
        }
    }

    Rectangle {
        id: fileListPanel
        width: 250
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        border.color: "#b9b9b9"

        Rectangle {
            id: titleBar
            width: parent.width
            height: 34
            color: "#f6f9fb"
            border.color: "#b9b9b9"
            Text {
                anchors.centerIn: parent
                text: "存储文件列表"
                font.pointSize: 14
                color: "#345"
            }
        }

        ListView {
            id: fileListView
            anchors.topMargin: 0
            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 8
            //绑定ListModel数据
            model: recFileListModel
            delegate: Rectangle {
                width: fileListView.width
                height: 32
                color: ListView.isCurrentItem ? "#e0eaff" : "transparent"
                border.color: "#cccccc"
                radius: 4

                Text {
                    text: fileName
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    font.pointSize: 10
                    color: "#222"
                }


                // 鼠标区域：检测左右键点击
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    // 左键点击选中项
                    onClicked: {
                        if (mouse.button === Qt.LeftButton) {
                            // 左键选中
                            fileListView.currentIndex = index;
                        } else if (mouse.button === Qt.RightButton) {
                            // 右键显示菜单
                            fileListView.currentIndex = index;
                            fileMenu.popup(mouse.x, mouse.y);
                        }
                    }

                    // 定义右键菜单（用 Menu 替代 ContextMenu）
                    Menu {
                        id: fileMenu

                        MenuItem {
                            text: "打开本地路径"
                            onTriggered: {
                                // 调用后端打开路径（需后端实现）
                                backend.openFileLocation(filePath);
                            }
                        }

                        MenuItem {
                            text: "删除文件"
                            onTriggered: {
                                // 发送信号，携带当前项的参数
                                deleteDialog.requestDelete(fileName, filePath, index);
                            }
                        }

                        MenuItem {
                            text: "使用vlc播放"
                            onTriggered: {
                                backend.playRecFile(filePath,"vlc");
                            }
                        }
                        MenuItem {
                            text: "使用ffplay播放"
                            onTriggered: {
                                backend.playRecFile(filePath,"ffplay");
                            }
                        }
                    }
                }
            }
            clip: true
            highlight: Rectangle { color: "#c8e6ff"; radius: 4 }
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
                width: 8
            }
        }
    }

    // 使用基础Dialog组件实现确认框
    Dialog {
        id: deleteDialog
        title: "确认删除"
        modal: true  // 模态对话框，阻止其他操作
        // 关键：将对话框居中到父元素（这里是root Item）
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No  // 标准按钮

        // 存储从信号接收的参数
        property string currentFileName: ""
        property string currentFilePath: ""
        property int currentIndex: -1

        // 定义接收参数的信号
        signal requestDelete(string fileName, string filePath, int index)

        // 绑定信号处理：接收参数并显示
        onRequestDelete: {
            currentFileName = fileName;
            currentFilePath = filePath;
            currentIndex = index;
            title = "确定删除 " + fileName + " 吗？"
            open(); // 显示对话框
        }

        // 对话框内容
        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16
            Text {
                text: "确定删除 " + currentFileName + " 吗？"
                font.bold: true
                color: "#333"
            }
        }

        // 点击确认按钮
        onAccepted: {
            backend.deleteFile(currentFilePath);
            recFileListModel.remove(currentIndex);
            deleteDialog.close();
        }

        // 点击取消按钮（可选，默认会关闭对话框）
        onRejected: {
            deleteDialog.close();
        }
    }


    // 摄像头多画面区
    Rectangle {
        id: cameraGrid
        anchors.left: fileListPanel.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 5
        width: 1300
        border.color: "#b9b9b9"

        GridLayout {
            id: camGrid
            anchors.fill: parent
            columns: 2
            rows: 2
            rowSpacing: 2
            columnSpacing: 2

            CameraView {
                title: "摄像头1"
                id:camView1
                enableDraw: false
                drawMode: mainWindow.drawMode
                selected: true

                // 接收子组件的绘制结束回调
                onDrawingFinished: {
                    console.log("父组件：子组件绘制结束");
                    updateShowPointTextEdit();
                }
            }

            CameraView {
                title: "摄像头2"
                id:camView2
                enableDraw: false
                drawMode: mainWindow.drawMode
                // 接收子组件的绘制结束回调
                onDrawingFinished: {
                    console.log("父组件：子组件绘制结束");
                    updateShowPointTextEdit();
                }
            }

            CameraView {
                title: "摄像头3"
                id:camView3
                enableDraw: false
                drawMode: mainWindow.drawMode
                // 接收子组件的绘制结束回调
                onDrawingFinished: {
                    console.log("父组件：子组件绘制结束");
                    updateShowPointTextEdit();
                }
            }

            CameraView {
                title: "摄像头4"
                id:camView4
                enableDraw: false
                drawMode: mainWindow.drawMode
                // 接收子组件的绘制结束回调
                onDrawingFinished: {
                    console.log("父组件：子组件绘制结束");
                    updateShowPointTextEdit();
                }
            }
        }
    }


    // 声明可写属性
    property int selectedCameraId: 1

    // 模拟摄像头切换的函数
    function updateCameraView(cameraId) {
        console.log("切换到摄像头:", cameraId)
        // 重置所有摄像头的选中状态
        camView1.selected = false
        camView2.selected = false
        camView3.selected = false
        camView4.selected = false

        // 设置当前选中的摄像头
        if(cameraId === 1) camView1.selected = true
        else if(cameraId === 2) camView2.selected = true
        else if(cameraId === 3) camView3.selected = true
        else if(cameraId === 4) camView4.selected = true
    }

    function updateDetectionMode()
    {
        if(selectedCameraId == 1)
        {
            camView1.drawMode = mainWindow.drawMode
            camView1.enableDraw = true
            camView1.selectDrawMode()
        }
        else if(selectedCameraId == 2)
        {
            camView2.drawMode = mainWindow.drawMode
            camView2.enableDraw = true
            camView2.selectDrawMode()
        }
        else if(selectedCameraId == 3)
        {
            camView3.drawMode = mainWindow.drawMode
            camView3.enableDraw = true
            camView3.selectDrawMode()
        }
        else if(selectedCameraId == 4)
        {
            camView4.drawMode = mainWindow.drawMode
            camView4.enableDraw = true
            camView4.selectDrawMode()
        }
    }

    function funcCamButton(buttonName,camViewIndex)
    {
        var cameraInfo = backend.getCameraInfo(cameraSelector.currentIndex);
        if(buttonName === "start")
        {
            console.log("启动,相机:",camViewIndex);
            if(camViewIndex === 0)
            {
                camView1.startCam(cameraInfo);
            }else if(camViewIndex === 1)
            {
                camView2.startCam(cameraInfo);
            }else if(camViewIndex === 2)
            {
                camView3.startCam(cameraInfo);
            }else if(camViewIndex === 3)
            {
                camView4.startCam(cameraInfo);
            }
        }
        else if(buttonName === "stop")
        {
            console.log("停止,相机:",camViewIndex);
            if(camViewIndex === 0)
            {
                camView1.stopCam(cameraInfo);
            }else if(camViewIndex === 1)
            {
                camView2.stopCam(cameraInfo);
            }else if(camViewIndex === 2)
            {
                camView3.stopCam(cameraInfo);
            }else if(camViewIndex === 3)
            {
                camView4.stopCam(cameraInfo);
            }
        }
        else if(buttonName === "restart")
        {
            console.log("重启,相机:",camViewIndex);
            if(camViewIndex === 0)
            {
                camView1.restartCam(cameraInfo);
            }else if(camViewIndex === 1)
            {
                camView2.restartCam(cameraInfo);
            }else if(camViewIndex === 2)
            {
                camView3.restartCam(cameraInfo);
            }else if(camViewIndex === 3)
            {
                camView4.restartCam(cameraInfo);
            }
        }
    }

    function recordBtnClicked(camViewIndex,recType)
    {
        if(camViewIndex === 0)
        {
            camView1.recordBtnClicked(recType);
        }else if(camViewIndex === 1)
        {
            camView2.recordBtnClicked(recType);
        }else if(camViewIndex === 2)
        {
            camView3.recordBtnClicked(recType);
        }else if(camViewIndex === 3)
        {
            camView4.recordBtnClicked(recType);
        }

        if(recType === "stop")
        {
            //刷新列表
            console.log("请求更新录像文件列表...")
            backend.updateRecFileList()
        }
    }

    function updateShowPointTextEdit()
    {
        var camViewIndex = cameraSelector.currentIndex
        var data = "";
        if(camViewIndex === 0)
        {
            data = camView1.getDetectionInfo(mainWindow.drawMode);
        }else if(camViewIndex === 1)
        {
            data = camView2.getDetectionInfo(mainWindow.drawMode);
        }else if(camViewIndex === 2)
        {
            data = camView3.getDetectionInfo(mainWindow.drawMode);
        }else if(camViewIndex === 3)
        {
            data = camView4.getDetectionInfo(mainWindow.drawMode);
        }

        const obj = JSON.parse(data);  // 先解析为JSON对象
        // 遍历坐标数组，将x和y转换为整数
        var show_data="";
        if (obj && Array.isArray(obj)) {
            // 遍历数组中的每个坐标对象
            obj.forEach(point => {
                // 取整处理（根据需要选择Math.round/floor/ceil）
                const x = Math.round(point.x);
                const y = Math.round(point.y);
                // 拼接成"(x,y)\n"格式，累加至show_data
                show_data += `(${x},${y})\n`;
            });
        }

        if(mainWindow.drawMode === 1)
        {
            row_yuejie_huizhi_textEdit.text = show_data
        }else if(mainWindow.drawMode === 2)
        {
            row_quyuruqin_textEdit.text = show_data
        }
    }

    //显示与隐藏
    function initAIFuncSettingView()
    {
        if(ai_func_switch.currentIndex === 0)
        {
            console.log("关闭AI功能，隐藏AI相关功能设置")
            row_yuyinbobao.visible = false;
            row_yinliangkongzhi.visible = false
            yuejiejiance_label.visible = false
            row_yuejiejiance.visible = false
            row_yuejie_huizhi_val.visible = false
            quyuruqinjiance_lable.visible = false
            row_quyuruqin.visible = false
            quyuruqin_val.visible = false
            row_jiancexuanxiang.visible = false

        }else if(ai_func_switch.currentIndex === 1)
        {
            console.log("打开行人检测")
            row_yuyinbobao.visible = true;
            row_yinliangkongzhi.visible = true;

            yuejiejiance_label.visible = true
            row_yuejiejiance.visible = true
            row_yuejie_huizhi_val.visible = true
            quyuruqinjiance_lable.visible = true
            row_quyuruqin.visible = true
            quyuruqin_val.visible = true
            row_jiancexuanxiang.visible = true

        }

        else if(ai_func_switch.currentIndex === 2)
        {
            console.log("打开目标检测")

            row_yuyinbobao.visible = true;
            row_yinliangkongzhi.visible = true;

            yuejiejiance_label.visible = false
            row_yuejiejiance.visible = false
            row_yuejie_huizhi_val.visible = false
            quyuruqinjiance_lable.visible = false
            row_quyuruqin.visible = false
            quyuruqin_val.visible = false
            row_jiancexuanxiang.visible = false
        }

        initAIJianCeXuanxiangView()
    }

    function initAIJianCeXuanxiangView()
    {
        if(ai_func_switch.currentIndex !== 1)
        {
            return ;
        }

        if(jiancexuanxiang_Id.currentIndex == 0)
        {
            quyuruqinjiance_lable.visible = false;
            row_quyuruqin.visible = false;
            quyuruqin_val.visible = false;

            yuejiejiance_label.visible = true;
            row_yuejiejiance.visible = true;
            row_yuejie_huizhi_val.visible = true;
        }
        else if(jiancexuanxiang_Id.currentIndex == 1)
        {
            quyuruqinjiance_lable.visible = true;
            row_quyuruqin.visible = true;
            quyuruqin_val.visible = true;

            yuejiejiance_label.visible = false;
            row_yuejiejiance.visible = false;
            row_yuejie_huizhi_val.visible = false;
        }

    }

    function saveDataAll()
    {

        var saveDataObj={};

        //分辨率
        var rId = resolutionId.currentIndex
        console.log("设置的分辨率: ", rId,resolutionList[rId])
        saveDataObj.resolutionId = rId;

        //AI总开关选项
        var aiFuncId = ai_func_switch.currentIndex
        console.log("设置的AI: ", aiFuncId,aiFuncList[aiFuncId])
        saveDataObj.aiFuncId = aiFuncId;
        if(aiFuncId === 0)
        {
            console.log("关闭AI功能.......")

        }else if(aiFuncId === 1)
        {
            console.log("开启行人检测,保存行人检测参数")
            var audio_switch = yuyinbobaoId.currentIndex;
            console.log("语音播报：",audio_switch===0?"打开":"关闭")
            var audio_val = yinliangkongzhi_val.value
            console.log("语音音量：",audio_val)

            saveDataObj.audio_switch = audio_switch;
            saveDataObj.audio_val = audio_val;
        }

        else if(aiFuncId === 2)
        {
            console.log("===================================================")
            console.log("开启目标检测,保存目标检测参数=================")
            var audio_switch = yuyinbobaoId.currentIndex;
            console.log("语音播报：",audio_switch===0?"打开":"关闭")
            var audio_val = yinliangkongzhi_val.value
            console.log("语音音量：",audio_val)

            saveDataObj.audio_switch = audio_switch;
            saveDataObj.audio_val = audio_val;

            var jinaceXuanxiang = jiancexuanxiang_Id.currentIndex
            console.log("检测选项：",jinaceXuanxiang===0?"越界检测":"区域入侵检测")
            saveDataObj.jinaceXuanxiang = jinaceXuanxiang;

            if(jinaceXuanxiang === 0)
            {
                console.log("获取越界检测坐标：")
                var data = camView1.getDetectionInfo(1);
                console.log("越界检测坐标：", data)
                saveDataObj.detectionInfo = data;
            }else if(jinaceXuanxiang === 1)
            {
                console.log("获取区域入侵坐标：")
                var data = camView1.getDetectionInfo(2);
                console.log("区域入侵检测坐标：", data)
                saveDataObj.detectionInfo = data;
            }
            console.log("===================================================")
        }

        //保存
        var camViewIndex = cameraSelector.currentIndex;
        if(camViewIndex === 0)
        {
            camView1.saveDataAll(saveDataObj)
        }else if(camViewIndex === 1)
        {
            camView2.saveDataAll(saveDataObj)
        }else if(camViewIndex === 2)
        {
            camView3.saveDataAll(saveDataObj)
        }else if(camViewIndex === 3)
        {
            camView4.saveDataAll(saveDataObj)
        }
    }


    // 定义弹出对话框
    Dialog {
        id: editDialog
        title: "编辑摄像头信息"
        // 设置对话框大小
        width: 400
        height: 400
        // 模态对话框，打开时阻止其他操作
        modal: true
        // 点击外部关闭对话框
        closePolicy: Dialog.CloseOnEscape | Dialog.CloseOnPressOutsideParent
        // 关键：将对话框居中到父元素（这里是root Item）
        anchors.centerIn: parent
        // 缓存摄像头信息，避免重复获取
        property var cameraInfo: null
        // 对话框打开时初始化数据
        onOpened: {
            // 只在打开时获取一次数据
            cameraInfo = backend.getCameraInfo(cameraSelector.currentIndex);
            console.log("初始化对话框数据:", JSON.stringify(cameraInfo));
            camNameText.text = cameraInfo.camName !== undefined ? cameraInfo.camName: ""
            camIPText.text = cameraInfo.camIP !== undefined ? cameraInfo.camIP: ""
            camPortText.text = cameraInfo.camPort !== undefined ? cameraInfo.camPort: ""
        }
        // 对话框内容
        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            Text {
                text: "当前选中的摄像头: " + cameraSelector.currentText
                font.bold: true
            }

            Row{
                width: parent.width  // 让Row宽度等于父容器（Column）宽度
                spacing: 8           // 增加元素间距
                Text {
                    id:camTagName
                    text: "设备名: "
                    font.bold: true
                }

                TextField {
                    id:camNameText
                    placeholderText: "输入摄像头名称"
                    width: parent.width - camTagName.width -20
                    // 显式启用文本交互功能
                    selectByMouse: true
                }
            }

            Row{
                width: parent.width  // 让Row宽度等于父容器（Column）宽度
                spacing: 8           // 增加元素间距
                Text {
                    id:camTagIp
                    text: "IP地址: "
                    font.bold: true
                }

                TextField {
                    id:camIPText
                    placeholderText: "输入摄像头IP地址"
                    width: parent.width - camTagIp.width -20
                    // 显式启用文本交互功能
                    selectByMouse: true
                }
            }

            Row{
                width: parent.width  // 让Row宽度等于父容器（Column）宽度
                spacing: 8           // 增加元素间距
                Text {
                    id:camTagPort
                    text: "端口号: "
                    font.bold: true
                }

                TextField {
                    id:camPortText
                    placeholderText: "输入摄像头端口"
                    width: parent.width - camTagPort.width - 20
                    // 显式启用文本交互功能
                    selectByMouse: true
                }
            }
        }

        // 对话框底部按钮
        standardButtons: Dialog.Ok | Dialog.Cancel

        // 处理确定按钮点击
        onAccepted: {
            console.log("摄像头信息已更新")
            console.log(camNameText.text,camIPText.text,camPortText.text)
            // 在这里添加保存摄像头信息的逻辑
            cameraInfo.camName = camNameText.text
            cameraInfo.camIP = camIPText.text
            cameraInfo.camPort = camPortText.text
            cameraInfo.camViewIndex = cameraSelector.currentIndex
            backend.setCameraInfo(cameraSelector.currentIndex,cameraInfo)
            initAfterWindowOpen()
        }

        // 处理取消按钮点击
        onRejected: {
            console.log("编辑已取消")
        }
    }

    Rectangle {
        id: sidePanel
        width: 356
        anchors.topMargin: 0
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: cameraGrid.right

        Column {
            id: column
            anchors.topMargin: 16
            anchors.rightMargin: 8
            anchors.fill: parent
            anchors.margins: 16
            spacing: 15

            //选择框
            // 摄像头选择下拉框


            Text { text: "当前选择的摄像头"; font.bold: true; font.pointSize: 13

            }


            Row {
                id: row
                width: 316
                height: 40
                spacing: 7
                layoutDirection: Qt.LeftToRight

                ComboBox {
                    id: cameraSelector
                    width: 208
                    currentIndex: 0
                    model: ["摄像头1", "摄像头2", "摄像头3", "摄像头4"]

                    // 选择变化时更新selectedCameraId
                    onCurrentIndexChanged: {
                        if (currentIndex >= 0) {
                            // 通过mainWindow引用属性
                            mainWindow.selectedCameraId = currentIndex + 1
                            console.log("已选择摄像头:", mainWindow.selectedCameraId)
                            updateCameraView(mainWindow.selectedCameraId)
                        }
                    }
                }

                Button {
                    text: "编辑信息"
                    onClicked:
                    {
                        console.log("编辑摄像头信息,相机:",cameraSelector.currentIndex);
                        // 显示对话框
                        editDialog.open()
                    }
                }
            }


            Row {
                spacing: 8
                Button {
                    text: "启动"
                    onClicked:
                    {
                        funcCamButton("start",cameraSelector.currentIndex)
                    }
                }
                Button {
                    text: "停止"
                    onClicked:
                    {
                        funcCamButton("stop",cameraSelector.currentIndex)
                    }
                }
                Button {
                    text: "重启"
                    onClicked:
                    {
                        funcCamButton("restart",cameraSelector.currentIndex)
                    }
                }
            }
            // 横线
            Rectangle {
                width: sidePanel.width - 15
                height: 3   // 横线粗细
                color: "#cccccc"  // 横线颜色
            }
            Text { text: "录像"; font.bold: true; font.pointSize: 13

            }

            Row {
                spacing: 8
                Button {
                    text: "启动录像"
                    id:start_recod_btn
                    enabled: !stop_recod_btn.enabled
                    onClicked: {
                        // 点击后切换状态：启动按钮不可点击，停止按钮可点击
                        start_recod_btn.enabled = false
                        stop_recod_btn.enabled = true
                        recordBtnClicked(cameraSelector.currentIndex,"start")
                    }
                }
                Button {
                    text: "停止录像"
                    id:stop_recod_btn
                    //默认不可点击
                    enabled: false
                    onClicked: {
                        // 点击后切换状态：停止按钮不可点击，启动按钮可点击
                        stop_recod_btn.enabled = false
                        start_recod_btn.enabled = true
                        recordBtnClicked(cameraSelector.currentIndex,"stop")
                    }
                }
            }
            // 横线
            Rectangle {
                width: sidePanel.width - 15
                height: 3   // 横线粗细
                color: "#cccccc"  // 横线颜色
            }
            Text {
                text: "参数配置"
                textFormat: Text.RichText
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                font.bold: true
                font.pointSize: 20
            }

            Row{
                spacing: 8
                height: 30
                Text {
                    text: "分辨率  ：";
                    font.pointSize: 10;
                    verticalAlignment: Text.AlignVCenter  // 文本垂直居中
                    height: parent.height
                }

                ComboBox {
                    id:resolutionId
                    model: resolutionList
                    currentIndex: 0
                    width: 100
                }
            }

            Row{
                spacing: 8
                height: 30
                Text {
                    text: "AI功能：";
                    font.pointSize: 10;
                    verticalAlignment: Text.AlignVCenter  // 文本垂直居中
                    height: parent.height
                }

                ComboBox {
                    id:ai_func_switch
                    model: aiFuncList
                    currentIndex: 0
                    width: 200
                    onCurrentIndexChanged: {
                        //显示与隐藏
                        initAIFuncSettingView()
                    }
                }
            }

            Row{
                spacing: 8
                height: 30
                id:row_yuyinbobao
                Text {
                    text: "语音播报：";
                    font.pointSize: 10;
                    verticalAlignment: Text.AlignVCenter  // 文本垂直居中
                    height: parent.height
                }

                ComboBox {
                    id:yuyinbobaoId
                    model: ["打开", "关闭"]
                    currentIndex: 0
                    width: 100
                }
            }

            Row{
                spacing: 8
                height: 30
                id:row_yinliangkongzhi
                Text {
                    text: "音量控制：";
                    font.pointSize: 10;
                    verticalAlignment: Text.AlignVCenter  // 文本垂直居中
                    height: parent.height
                }

                SpinBox {
                   id: yinliangkongzhi_val
                   from: 0          // 最小值
                   to: 100          // 最大值
                   value: 0        // 默认值
                   stepSize: 1      // 步长
                   width: 150
//                   tooltip: "请输入0-120之间的数字"
               }
            }

            Row{
                spacing: 8
                height: 30
                id:row_jiancexuanxiang
                Text {
                    text: "检测选项：";
                    font.pointSize: 10;
                    verticalAlignment: Text.AlignVCenter  // 文本垂直居中
                    height: parent.height
                }

                ComboBox {
                    id:jiancexuanxiang_Id
                    model: ["越界检测", "区域入侵检测"]
                    currentIndex: 0
                    width: 200
                    onCurrentIndexChanged: {
                        //显示与隐藏
                        initAIJianCeXuanxiangView()
                    }
                }
            }

            Text {
                text: "越界检测绘制:";
                font.pointSize: 11
                id:yuejiejiance_label
            }

            Row{
                spacing: 8
                id:row_yuejiejiance
                Button {
                    text: "开始绘制"
                    onClicked: {
                        mainWindow.drawMode = 1
                        console.log("[越界检测] drawMode=",mainWindow.drawMode)
                        updateDetectionMode();
                    }
                }
                Button {
                    text: "绘制结束"
                    onClicked: {
                        console.log("保存越界检测信息")
                        var data = camView1.getDetectionInfo(1);
                        const obj = JSON.parse(data);  // 先解析为JSON对象
                        // 遍历坐标数组，将x和y转换为整数
                        var show_data="";
                        if (obj && Array.isArray(obj)) {
                            // 遍历数组中的每个坐标对象
                            obj.forEach(point => {
                                // 取整处理（根据需要选择Math.round/floor/ceil）
                                const x = Math.round(point.x);
                                const y = Math.round(point.y);
                                // 拼接成"(x,y)\n"格式，累加至show_data
                                show_data += `(${x},${y})\n`;
                            });
                        }

                        console.log("转换后:",show_data)
                        row_yuejie_huizhi_textEdit.text = show_data
                    }
                }
            }

            Row{
                spacing: 8
                id:row_yuejie_huizhi_val
                Rectangle {
                    // 边框样式
                    width: sidePanel.width - 15
                    height: 300
                    border.width: 1
                    border.color: "#666666"
                    radius: 3                // 圆角
                    color: "white"           // 背景色

                   TextEdit {
                       id: row_yuejie_huizhi_textEdit
                       // 宽度减去滚动条的宽度（避免水平滚动）
                       width: sidePanel.width - 16
                       anchors.margins: 10
                       font.pixelSize: 20
                       wrapMode: TextEdit.Wrap  // 自动换行
                       text: qsTr("暂未绘制")
                   }
                }
            }

            Text {
                text: "区域入侵检测绘制:"
                font.pointSize: 11
                id:quyuruqinjiance_lable
            }

            Row {
                spacing: 8
                id:row_quyuruqin
                Button {
                    text: "开始绘制"
                    onClicked: {
                        mainWindow.drawMode = 2
                        updateDetectionMode();
                        console.log("[区域入侵检测] drawMode=",mainWindow.drawMode)
                    }
                }

                Button {
                    text: "绘制结束"
                    onClicked: {
                        console.log("保存区域入侵检测信息")
                        var data = camView1.getDetectionInfo(2);
                        const obj = JSON.parse(data);  // 先解析为JSON对象

                        // 遍历坐标数组，将x和y转换为整数
                        var show_data="";
                        if (obj && Array.isArray(obj)) {
                            // 遍历数组中的每个坐标对象
                            obj.forEach(point => {
                                // 取整处理（根据需要选择Math.round/floor/ceil）
                                const x = Math.round(point.x);
                                const y = Math.round(point.y);
                                // 拼接成"(x,y)\n"格式，累加至show_data
                                show_data += `(${x},${y})\n`;
                            });
                        }

                        console.log("转换后:",show_data)
                        row_quyuruqin_textEdit.text = show_data
                    }
                }
            }

            Row{
                spacing: 8
                id:quyuruqin_val

                Rectangle {
                    // 边框样式
                    width: sidePanel.width - 15
                    height: 300
                    border.width: 1
                    border.color: "#666666"
                    radius: 3                // 圆角
                    color: "white"           // 背景色
                    TextEdit {
                       id: row_quyuruqin_textEdit
                       width: sidePanel.width - 18
                       height: 255
                       anchors.margins: 10
                       font.pixelSize: 20
                       wrapMode: TextEdit.Wrap  // 自动换行
                       text: qsTr("暂未绘制")
                    }
                }
            }



            Row{
                spacing: 8
                Button {
                    text: "一键保存"
                    width: sidePanel.width - 15
                    // 确保按钮有足够高度显示背景
                    height: 30

                    // 明确设置背景填充整个按钮区域
                    background: Rectangle {
                        id: btnBackground
                        color:defaultColor
                        anchors.fill: parent  // 关键：填充整个按钮区域
                        radius: 4
                        border.width: 1  // 确保边框可见
                    }
                    onPressedChanged: {
                      // 根据pressed状态修改背景颜色
                      btnBackground.color = pressed ? pressedColor : defaultColor
                    }

                    onClicked: {
                        console.log("[一键保存]")
                        saveDataAll();
                    }
                }
            }

        }
    }
}




/*##^## Designer {
    D{i:5;invisible:true}D{i:4;invisible:true}D{i:6;invisible:true}D{i:3;invisible:true}
D{i:19;invisible:true}D{i:18;invisible:true}D{i:17;invisible:true}D{i:22;invisible:true}
D{i:23;invisible:true}D{i:24;invisible:true}D{i:25;invisible:true}D{i:21;invisible:true}
D{i:20;invisible:true}D{i:28;invisible:true}D{i:30;invisible:true}D{i:31;invisible:true}
D{i:29;invisible:true}D{i:33;invisible:true}D{i:34;invisible:true}D{i:32;invisible:true}
D{i:36;invisible:true}D{i:37;invisible:true}D{i:35;invisible:true}D{i:27;invisible:true}
D{i:26;invisible:true}D{i:41;anchors_height:40;anchors_width:283}D{i:38;invisible:true}
}
 ##^##*/
