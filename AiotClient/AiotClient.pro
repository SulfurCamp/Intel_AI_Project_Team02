QT       += core gui network charts sql webenginewidgets multimedia multimediawidgets
QT       += widgets network sql charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwidget.cpp \
    sockclient.cpp \
    tab1data.cpp \
    #tab1socketclient.cpp \
    tab2camstream.cpp

HEADERS += \
    mainwidget.h \
    sockclient.h \
    tab1data.h \
    #tab1socketclient.h \
    tab2camstream.h

FORMS += \
    mainwidget.ui \
    tab1data.ui \
    #tab1socketclient.ui \
    tab2camstream.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += /usr/local/include/opencv4
LIBS += `pkg-config opencv4 --cflags --libs`


RESOURCES += \
    Images.qrc
