TEMPLATE = app
TARGET = demobrowser
QT += webenginewidgets network widgets printsupport
CONFIG += c++11

qtHaveModule(uitools):!embedded: QT += uitools
else: DEFINES += QT_NO_UITOOLS

HEADERS += \
    browserapplication.h \
    browsermainwindow.h \
    modelmenu.h \
    searchlineedit.h \
    tabwidget.h \
    urllineedit.h \
    webview.h \
    screenshotter.h \
    fixedelement.h \
    mousetracker.h

SOURCES += \
    browserapplication.cpp \
    browsermainwindow.cpp \
    modelmenu.cpp \
    searchlineedit.cpp \
    tabwidget.cpp \
    urllineedit.cpp \
    webview.cpp \
    main.cpp \
    screenshotter.cpp \
    fixedelement.cpp \
    mousetracker.cpp

LIBS += -LC:/Qt/openssl-1.0.2j-32bit-release-dll-vs2015/lib -lssleay32 -llibeay32
INCLUDEPATH += C:/Qt/openssl-1.0.2j-32bit-release-dll-vs2015/include
DEPENDPATH += C:/Qt/openssl-1.0.2j-32bit-release-dll-vs2015/include

RESOURCES += data/data.qrc htmls/htmls.qrc \
    resources.qrc

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

win32 {
   RC_FILE = demobrowser.rc
}

mac {
    ICON = demobrowser.icns
    QMAKE_INFO_PLIST = Info_mac.plist
    TARGET = Demobrowser
}

EXAMPLE_FILES = \
    Info_mac.plist demobrowser.icns demobrowser.ico demobrowser.rc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/demobrowser
INSTALLS += target

DISTFILES +=

FORMS += \
    passworddialog.ui \
    proxy.ui
