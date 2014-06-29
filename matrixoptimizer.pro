
#################### qt stuff #########################

TARGET = matrixoptimizer

QT += core gui widgets opengl

TEMPLATE = app

##################### flags ###########################

QMAKE_CXXFLAGS += --std=c++0x
QMAKE_CXXFLAGS_RELEASE += -O2 -DNDEBUG

# for optirun bug
DEFINES += MO_DISABLE_OBJECT_TREE_DRAG

##################### libs ############################

unix: { LIBS += -lX11 }

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
    src/gui/ruler.cpp \
    src/gui/timeline1drulerview.cpp \
    src/gui/util/viewspace.cpp \
    src/io/xmlstream.cpp \
    src/tests/testxmlstream.cpp \
    src/io/datastream.cpp \
    src/model/qobjecttreemodel.cpp \
    src/gui/qobjectinspector.cpp \
    src/object/object.cpp \
    src/tool/stringmanip.cpp \
    src/model/objecttreemodel.cpp \
    src/object/objectfactory.cpp \
    src/io/application.cpp \
    src/object/parameter.cpp \
    src/object/object3d.cpp \
    src/object/soundsource.cpp \
    src/object/microphone.cpp \
    src/object/camera.cpp \
    src/object/dummy.cpp \
    src/gui/objecttreeview.cpp \
    src/model/objecttreemimedata.cpp \
    src/gl/window.cpp \
    src/gl/context.cpp \
    src/object/scene.cpp \
    src/object/objectgl.cpp \
    src/gl/manager.cpp \
    src/gl/model.cpp \
    src/io/init.cpp

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
    src/gui/ruler.h \
    src/io/log.h \
    src/io/error.h \
    src/io/streamoperators_qt.h \
    src/gui/timeline1drulerview.h \
    src/io/xmlstream.h \
    src/tests/testxmlstream.h \
    src/io/datastream.h \
    src/model/qobjecttreemodel.h \
    src/gui/qobjectinspector.h \
    src/doc.h \
    src/object/object.h \
    src/tool/stringmanip.h \
    src/model/objecttreemodel.h \
    src/object/objectfactory.h \
    src/io/application.h \
    src/object/parameter.h \
    src/object/object3d.h \
    src/object/soundsource.h \
    src/object/microphone.h \
    src/object/camera.h \
    src/object/dummy.h \
    src/gui/objecttreeview.h \
    src/model/objecttreemimedata.h \
    src/gl/window.h \
    src/gl/context.h \
    src/object/scene.h \
    src/object/objectgl.h \
    src/gl/manager.h \
    src/gl/model.h \
    src/io/init.h \
    src/gl/openglfunctions.h

FORMS += \
    src/gui/projectorsetupwidget.ui

RESOURCES += \
    icons.qrc
