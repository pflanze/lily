TEMPLATE = app
TARGET = lilyExample

QT = core gui
QT += webkit webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += lily/bin/lily.a -lm -lstdc++
PRE_TARGETDEPS += lily/bin/lily.a
INCLUDEPATH += lily/src
QMAKE_CXXFLAGS = -std=c++11 -O0 -gdwarf-4 -g3


SOURCES += \
	main.cpp gutil.cpp mainwindow2.cpp gReader.cpp

FORMS += \
    mainwindow2.ui

HEADERS += \
    mainwindow2.h main.h gutil.h gReader.h

