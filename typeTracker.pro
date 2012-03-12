SOURCES += src/*.cpp

HEADERS += inc/*.h

FORMS += *.ui

INCLUDEPATH += inc

#LIBS += "C:/Program Files/Microsoft SDKs/Windows/v7.0A/Lib/User32.lib"
win32:LIBS += User32.lib

CONFIG += qt
CONFIG += qwt
CONFIG += debug

QT += sql
