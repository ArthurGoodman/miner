TEMPLATE = app
CONFIG += windows c++11
QT += core gui widgets

LIBS += -L../framework/release -L../neuro/release -lframework -lneuro -lgdi32 -lgdiplus
INCLUDEPATH += ../framework ../neuro

SOURCES += \
    main.cpp
