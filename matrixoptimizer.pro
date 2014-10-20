
#################### qt stuff #########################

TARGET = matrixoptimizer

QT += core gui widgets opengl network xml

TEMPLATE = app

##################### flags ###########################

CONFIG += c++11

#QMAKE_CXXFLAGS_DEBUG += -pg

QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

#for glm version >= 0.9.5
DEFINES += GLM_FORCE_RADIANS

#disable for production until it works ...
DEFINES += MO_DISABLE_PROJECTOR_LENS_RADIUS

# for optirun bug
unix: { DEFINES += MO_DISABLE_OBJECT_TREE_DRAG }

##################### libs ############################

mac  { LIBS += -L/opt/local/lib/ -L/usr/local/lib/ -lglbinding -lportaudio -lportmidi -lsndfile -lgstreamer-1.0 -lgstapp-1.0 -lgobject-2.0 -lglib-2.0 }
else: unix: { LIBS += -lglbinding -lGLU -lGL -lX11 -lportaudio -lportmidi -lsndfile -lgstreamer-1.0 -lgstapp-1.0 -lgobject-2.0 -lglib-2.0 }
else: win32 { LIBS += -lkernel32 -lpsapi -lportaudio -lsndfile-1 -lgstreamer-1.0 -lgstapp-1.0 -lgobject-2.0 -lglib-2.0 }

###################### files ##########################

mac: { INCLUDEPATH += /opt/local/include \
                      /usr/local/include \
                      /opt/local/include/gstreamer-1.0 \
                      /opt/local/include/glib-2.0 \
                      /opt/local/lib/glib-2.0/include }

include(src/gui/gui.pri)
include(src/common.pri)
include(src/tests/tests.pri)

INCLUDEPATH += src

HEADERS += \
    src/engine/serverengine.h

SOURCES += \
    src/main.cpp \
    src/engine/serverengine.cpp

OTHER_FILES += $$BISON_FILES \
    TODO.txt \
    assets/shader/default.vert \
    assets/shader/default.frag \
    assets/model/camera.obj_ \
    assets/shader/framebufferdraw.vert \
    assets/shader/framebufferdraw.frag \
    assets/shader/framebuffercamera.vert \
    assets/shader/framebuffercamera.frag \
    assets/shader/test.vert \
    assets/shader/test.frag \
    assets/shader/textureoverlay.vert \
    assets/shader/textureoverlay.frag \
    assets/help/en/objects.html \
    assets/help/en/equation.html \
    assets/help/en/index.html \
    assets/help/en/style.css \
    assets/help/en/equationfunctions.html \
    assets/help/en/sequence.html \
    src/INSTALL.txt

####################### BISON PARSER #######################

#BISON_BIN = bison

#bison_comp.input = BISON_FILES
##bison_comp.output = ./${QMAKE_FILE_BASE}.cc
#bison_comp.commands = $$BISON_BIN ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_BASE}.cc --defines=./${QMAKE_FILE_BASE}.hh
#QMAKE_EXTRA_COMPILERS += bison_comp

