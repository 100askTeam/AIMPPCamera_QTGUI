QT += quick network
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/CameraPaintItem.cpp \
    core/main.cpp \
    core/mainBackend.cpp \
    core/media/MediaCore.cpp \
    core/UpdateConfigCLient.cpp \
    core/media/ffmpeg_common.cpp \
    core/CameraConfig.cpp \
    core/media/MediaRecord.cpp \
    core/media/MediaStream.cpp

RESOURCES += qml.qrc

# 设置FFmpeg库的根目录
FFMPEG_DIR = $$PWD/ffmpeg
message("FFmpeg  path: " $$FFMPEG_DIR)

#x86或x64
win32 {
  # 检测系统位数
    contains(QMAKE_HOST.arch, x86_64) {
        # 64位系统配置
        message("Building for 64-bit Windows")
        FFMPEG_LIB_DIR = $$FFMPEG_DIR/x64/lib
        FFMPEG_INS_DIR = $$FFMPEG_DIR/x64/include
    } else {
        # 32位系统配置
        message("Building for 32-bit Windows")
        FFMPEG_LIB_DIR = $$FFMPEG_DIR/x86/lib
        FFMPEG_INS_DIR = $$FFMPEG_DIR/x86/include
    }
}

# 添加FFmpeg头文件路径
INCLUDEPATH += $$FFMPEG_INS_DIR
message("FFmpeg  path: " $$INCLUDEPATH)
# 添加FFmpeg库文件路径
LIBS += -L$$FFMPEG_LIB_DIR

# 添加需要链接的FFmpeg库
LIBS += -lavformat \
        -lavcodec \
        -lavutil \
        -lswscale \
        -lswresample \
        -lavdevice \
        -lavfilter

LIBS += -lws2_32  # 链接Winsock库

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    core/CameraPaintItem.h \
    core/mainBackend.h \
    core/media/ffmpeg_common.h \
    core/media/MediaCore.hpp \
    core/UpdateConfigCLient.h \
    core/updata_common.h \
    core/CameraConfig.h \
    core/media/MediaRecord.hpp \
    core/media/MediaStream.hpp

#

SUBDIRS +=

DISTFILES += \
    CameraView.qml \
    qml/main.js
