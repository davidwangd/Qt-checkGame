#-------------------------------------------------
#
# Project created by QtCreator 2017-09-05T01:42:57
#
#-------------------------------------------------

QT       += core gui
QT += network multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = checkGame
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    logic.cpp \
    board.cpp

HEADERS  += mainwindow.h \
    const.h \
    logic.h \
    board.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
