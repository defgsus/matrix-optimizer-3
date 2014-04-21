
#################### qt stuff #########################

TARGET = matrixoptimizer

QT += core gui widgets opengl

TEMPLATE = app

##################### flags ###########################

QMAKE_CXXFLAGS += --std=c++0x

###################### files ##########################

INCLUDEPATH += src

SOURCES += \
        src/main.cpp \
        src/gui/mainwindow.cpp \
    src/gui/projectorsetupwidget.cpp \
    src/projection/projector.cpp \
    src/gui/basic3dviewer.cpp

HEADERS += \
        src/gui/mainwindow.h \
    src/gui/projectorsetupwidget.h \
    src/types/vector.h \
    src/projection/projector.h \
    src/gui/basic3dviewer.h

FORMS += \
        src/gui/mainwindow.ui \
    src/gui/projectorsetupwidget.ui
