TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        frame.c \
        main.c \
        parsewav.c \
        readbits.c

HEADERS += \
    frame.h \
    main.h \
    parsewav.h \
    readbits.h
