#-------------------------------------------------
#
# Project created by QtCreator 2013-11-10T00:40:11
#
#-------------------------------------------------

QT       += core

QT       -= gui

QT += xml

TARGET = manag-dev
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += 3rdparty/libmodbus 3rdparty/qModMaster

SOURCES += main.cpp \
    transferthread.cpp \
    corebycetoolthread.cpp \
    hardware.cpp \
    module.cpp \
    3rdparty/libmodbus/modbus-tcp.c \
    3rdparty/libmodbus/modbus-rtu.c \
    3rdparty/libmodbus/modbus-data.c \
    3rdparty/libmodbus/modbus.c \
    eswitch.cpp

HEADERS += \
    transferthread.h \
    corebycetoolthread.h \
    hardware.h \
    module.h \
    3rdparty/libmodbus/modbus-version.h \
    3rdparty/libmodbus/modbus-tcp-private.h \
    3rdparty/libmodbus/modbus-tcp.h \
    3rdparty/libmodbus/modbus-rtu-private.h \
    3rdparty/libmodbus/modbus-rtu.h \
    3rdparty/libmodbus/modbus-private.h \
    3rdparty/libmodbus/modbus.h \
    3rdparty/libmodbus/config.h \
    eswitch.h

