
TARGET = matrixoptimizer

QT += core gui widgets

TEMPLATE = app

INCLUDEPATH += src

SOURCES += \
        src/main.cpp \
        src/gui/mainwindow.cpp \
    src/gui/projectorsetupwidget.cpp \
    src/projection/projector.cpp

HEADERS += \
        src/gui/mainwindow.h \
    src/gui/projectorsetupwidget.h \
    src/types/vector.h \
    src/projection/projector.h

FORMS += \
        src/gui/mainwindow.ui \
    src/gui/projectorsetupwidget.ui
