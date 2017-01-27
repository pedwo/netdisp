#
# netdisp project file
#

greaterThan(QT_MAJOR_VERSION, 4): cache

TEMPLATE = app

TARGET = netdisp

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += release

target.path = /usr/bin
INSTALLS += target

INCLUDEPATH += GeneratedFiles

MOC_DIR = GeneratedFiles
OBJECTS_DIR = Objects
UI_DIR = GeneratedFiles
RCC_DIR = GeneratedFiles

SOURCES += main.cpp \
           cpu_idle.cpp \
           netdisp.cpp

HEADERS += netdisp.h

