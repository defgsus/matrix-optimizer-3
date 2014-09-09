
#################### qt stuff #########################

TARGET = matrixoptimizer_client

QT += core gui widgets opengl network xml

TEMPLATE = app

##################### flags ###########################

CONFIG += c++11

#QMAKE_CXXFLAGS_DEBUG += -pg

QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

DEFINES += GLEW_MX MO_CLIENT

#for glm version >= 0.9.5
DEFINES += GLM_FORCE_RADIANS

# for optirun bug
unix: { DEFINES += MO_DISABLE_OBJECT_TREE_DRAG }

##################### libs ############################

unix: { LIBS += -lGLEWmx -lGLU -lGL -lX11 -lportaudio -lportmidi -lsndfile }

win32: { LIBS += -lkernel32 -lpsapi -lportaudio -lportmidi -lsndfile-1 }

###################### files ##########################

include(src/common.pri)
include(src/client.pri)

INCLUDEPATH += src



####################### BISON PARSER #######################

#BISON_BIN = bison

#bison_comp.input = BISON_FILES
##bison_comp.output = ./${QMAKE_FILE_BASE}.cc
#bison_comp.commands = $$BISON_BIN ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_BASE}.cc --defines=./${QMAKE_FILE_BASE}.hh
#QMAKE_EXTRA_COMPILERS += bison_comp

HEADERS += \
    src/network/tcpclient.h

SOURCES += \
    src/network/tcpclient.cpp
