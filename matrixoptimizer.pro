
#################### qt stuff #########################

TARGET = matrixoptimizer

QT += core gui widgets opengl

TEMPLATE = app

##################### flags ###########################

QMAKE_CXXFLAGS += --std=c++0x
QMAKE_CXXFLAGS_RELEASE += -O2 -DNDEBUG

###################### files ##########################

INCLUDEPATH += src

SOURCES += \
        src/main.cpp \
        src/gui/mainwindow.cpp \
    src/gui/projectorsetupwidget.cpp \
    src/projection/projector.cpp \
    src/gui/basic3dview.cpp \
    src/gui/projectorsetupview.cpp \
    src/math/timeline1d.cpp \
    src/gui/timeline1dview.cpp \
    src/tests/testtimeline.cpp \
    src/io/console.cpp \
    src/audio/audiosource.cpp \
    src/gui/painter/grid.cpp \
    src/gui/ruler.cpp

HEADERS += \
        src/gui/mainwindow.h \
    src/gui/projectorsetupwidget.h \
    src/types/vector.h \
    src/projection/projector.h \
    src/gui/basic3dview.h \
    src/gui/projectorsetupview.h \
    src/math/vector.h \
    src/math/constants.h \
    src/math/timeline1d.h \
    src/math/interpol.h \
    src/gui/timeline1dview.h \
    src/types/float.h \
    src/tests/testtimeline.h \
    src/io/console.h \
    src/audio/audiosource.h \
    src/gui/painter/grid.h \
    src/gui/util/viewspace.h \
    src/math/functions.h \
    src/gui/ruler.h

FORMS += \
        src/gui/mainwindow.ui \
    src/gui/projectorsetupwidget.ui
