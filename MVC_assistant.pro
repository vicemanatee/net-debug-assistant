#-------------------------------------------------
#
# Project created by QtCreator 2023-02-13T17:01:33
#
#-------------------------------------------------

QT       += core gui network

QMAKE_CXXFLAGS += -gstabs+

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MVC_assistant
TEMPLATE = app


SOURCES += main.cpp\
        controller.cpp \
    view.cpp \
    model_client.cpp \
    model.cpp \
    sendfileinthread.cpp \
    sendontime.cpp \
    model_udpconnnect.cpp \
    sendbydialog.cpp

HEADERS  += controller.h \
    model.h \
    view.h \
    model_client.h \
    sendfileinthread.h \
    sendontime.h \
    model_udpconnnect.h \
    sendbydialog.h

FORMS    += controller.ui \
    clientSendFileInThread.ui \
    sendontime.ui \
    sendbydialog.ui

RESOURCES += \
    resource.qrc
