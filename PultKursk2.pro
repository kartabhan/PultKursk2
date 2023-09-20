#-------------------------------------------------
#
# Project created by QtCreator 2015-04-30T09:06:07
#
#-------------------------------------------------

QT       += core gui widgets network
#DEFINES  += QT_NO_DEBUG_OUTPUT
#CONFIG += console

TARGET = PultQT
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp \
    clientsocketadapter.cpp \
    net/isocketadapter.cpp \
    net/socketadapter.cpp \
    dataproc/recordstorage.cpp \
    dataproc/showdata.cpp \
    recordblackbox.cpp \
    graph/graphstorage/graphstorage.cpp \
    graph/graphstorage/graphblackbox.cpp \
    util.cpp \
    graph/avtwindow.cpp \
    verticallabel.cpp \
    siglabel.cpp \
    dataproc/failstorage.cpp

HEADERS  += mainwidget.h \
    clientsocketadapter.h \
    net/isocketadapter.h \
    net/socketadapter.h \
    dataproc/recordstorage.h \
    dataproc/showdata.h \
    recordblackbox.h \
    graph/defines.h \
    graph/graphstorage/graphblackbox.h \
    graph/graphstorage/graphstorage.h \
    util.h \
    debug.h \
    graph/avtwindow.h \
    verticallabel.h \
    siglabel.h \
    dataproc/failstorage.h

FORMS    += \
    form.ui \
    graph/avtwindow.ui



OTHER_FILES +=

RESOURCES += \
    icons.qrc
