TEMPLATE = app
TARGET = demobrowser
QT += webenginewidgets network widgets printsupport
CONFIG += c++11

qtHaveModule(uitools):!embedded: QT += uitools
else: DEFINES += QT_NO_UITOOLS

HEADERS += \
    browserapplication.h \
    browsermainwindow.h \
    searchlineedit.h \
    tabwidget.h \
    urllineedit.h \
    webrtc.h \
    webview.h \
    screenshotter.h \
    fixedelement.h \
    mousetracker.h

SOURCES += \
    browserapplication.cpp \
    browsermainwindow.cpp \
    searchlineedit.cpp \
    tabwidget.cpp \
    urllineedit.cpp \
    webrtc.cpp \
    webview.cpp \
    main.cpp \
    screenshotter.cpp \
    fixedelement.cpp \
    mousetracker.cpp

#LIBS += -LC:/Qt/openssl-1.0.2j-32bit-release-dll-vs2015/lib -lssleay32 -llibeay32
#INCLUDEPATH += C:/Qt/openssl-1.0.2j-32bit-release-dll-vs2015/include
#DEPENDPATH += C:/Qt/openssl-1.0.2j-32bit-release-dll-vs2015/include

RESOURCES += ../data.qrc ../htmls/htmls.qrc \
    ../resources.qrc

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

mac {
    ICON = ../data/demobrowser.icns
    QMAKE_INFO_PLIST = ../data/Info_mac.plist
    TARGET = Demobrowser
}

RC_FILE = ../demobrowser.rc

EXAMPLE_FILES = \
    ./data/Info_mac.plist ./data/demobrowser.icns ./data/demobrowser.ico ./data/demobrowser.rc

# install
target.path = build
INSTALLS += target

DISTFILES +=

FORMS += \
    ../ui/passworddialog.ui \
    ../ui/proxy.ui
