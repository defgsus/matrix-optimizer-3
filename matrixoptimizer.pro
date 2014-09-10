
#################### qt stuff #########################

TARGET = matrixoptimizer

QT += core gui widgets opengl network xml

TEMPLATE = app

##################### flags ###########################

CONFIG += c++11

#QMAKE_CXXFLAGS_DEBUG += -pg

QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

DEFINES += GLEW_MX

#for glm version >= 0.9.5
DEFINES += GLM_FORCE_RADIANS

# for optirun bug
unix: { DEFINES += MO_DISABLE_OBJECT_TREE_DRAG }

##################### libs ############################

unix: { LIBS += -lGLEWmx -lGLU -lGL -lX11 -lportaudio -lportmidi -lsndfile }

win32: { LIBS += -lkernel32 -lpsapi -lportaudio -lsndfile-1 }

###################### files ##########################

include(src/gui/gui.pri)
include(src/common.pri)
include(src/tests/tests.pri)

INCLUDEPATH += src

SOURCES += \
    src/main.cpp

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

