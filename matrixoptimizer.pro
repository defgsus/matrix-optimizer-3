
TARGET = matrixoptimizer

QT += core gui widgets

TEMPLATE = app


SOURCES += \
        src/main.cpp \
        src/gui/mainwindow.cpp \
    src/gui/projectorsetupwidget.cpp

HEADERS += \
        src/gui/mainwindow.h \
    src/gui/projectorsetupwidget.h \
    src/types/vector.h

FORMS += \
        src/gui/mainwindow.ui \
    src/gui/projectorsetupwidget.ui
