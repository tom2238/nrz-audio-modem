TARGET = encoder
INCLUDEPATH += ../common
LIBS += -L../common -lcommon

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.c \

HEADERS += \
    main.h \
