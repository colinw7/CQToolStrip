TEMPLATE = lib

TARGET = CQToolStrip

DEPENDPATH += .

QT += widgets

CONFIG += staticlib

# Input
HEADERS += \
../include/CQToolStrip.h \
../include/CQFrameMenu.h \

SOURCES += \
CQToolStrip.cpp \
CQFrameMenu.cpp \

OBJECTS_DIR = ../obj

DESTDIR = ../lib

INCLUDEPATH += \
. \
../include \
../../CQUtil/include \
