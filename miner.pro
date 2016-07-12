TEMPLATE = app
CONFIG += windows c++11
QT += core gui widgets

LIBS += -L../neuro/release -lneuro
INCLUDEPATH += ../neuro

SOURCES += \
    main.cpp \
    minerwindow.cpp

HEADERS += \
    minerwindow.h
