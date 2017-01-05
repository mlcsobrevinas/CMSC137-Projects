QT += core network
QT -= gui

TARGET = Project1
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    server.cpp \
    client.cpp \
    header.cpp

HEADERS += \
    server.h \
    client.h \
    header.h
