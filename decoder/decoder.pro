TARGET = decoder
INCLUDEPATH += ../common
LIBS += -L../common -lcommon

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c \
        parsewav.c \
        readbits.c

HEADERS += \
    main.h \
    parsewav.h \
    readbits.h
