
#################### qt stuff #########################

TARGET = matrixoptimizer

!host_build:QMAKE_MAC_SDK = macosx10.10

QT += core gui widgets opengl network xml

TEMPLATE = app

##################### flags ###########################

CONFIG += c++11

#QMAKE_CXXFLAGS_DEBUG += -pg

QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

#DEFINES += MO_DISABLE_AUDIO

#for glm version >= 0.9.5
DEFINES += GLM_FORCE_RADIANS

#disable for production until it works ...
DEFINES += MO_DISABLE_PROJECTOR_LENS_RADIUS

#as long as it is not really used, avoid this dependency
DEFINES += MO_DISABLE_GST

#thought would be nice, but the dependencies are a bit broken for ubuntu 14
DEFINES += MO_DISABLE_CGAL

# dear macies, see for yourself if you need those features
mac { DEFINES += \
#require audio input and output devices to be separate devices
                MO_REQUIRE_SEPARATE_AUDIO \
#                MO_DISABLE_ANGELSCRIPT \
                # tracker-model player library
                MO_DISABLE_DUMB \
                # linux audio plugins
                MO_DISABLE_LADSPA \
                # neuro-imaging io library
                MO_DISABLE_NIFTI \
                # shapefiles
                MO_DISABLE_SHP
}

windows { DEFINES += \
                MO_DISABLE_LADSPA \
                MO_DISABLE_DUMB \
                MO_DISABLE_NIFTI \
                MO_DISABLE_SHP
}

# for optirun bug (XXX old and obsolete by now)
unix: { DEFINES += MO_DISABLE_OBJECT_TREE_DRAG }

##################### libs ############################

mac {
LIBS += -L/opt/local/lib/ \
        -L/usr/local/lib/ \
        -lglbinding       \
        -lportaudio       \
        -lportmidi        \
        -lsndfile         \
        -lgstreamer-1.0   \
        -lgstapp-1.0      \
        -lgobject-2.0     \
        -lglib-2.0        \
        -langelscript
}

unix {
LIBS += -lglbinding \
        -lGLU -lGL -lX11 \
        -lportaudio -lportmidi -lsndfile -ldumb \
        -langelscript \
        -lshp \
#        -lgstreamer-1.0 -lgstapp-1.0 -lgobject-2.0 -lglib-2.0 \
        -ldl    # dynamic linking
#        -lCGAL \
}

win32 {
LIBS += -lkernel32 -lpsapi \
        -lopengl32 -lglu32 -lglbinding \
        -lportaudio -lPortMidi -lsndfile-1 \
        -langelscript
        #-lgstreamer-1.0 -lgstapp-1.0 -lgobject-2.0 -lglib-2.0
}

###################### files ##########################

INCLUDEPATH += src

mac: { INCLUDEPATH += /opt/local/include \
                      /usr/local/include \
                      /usr/local/include/gstreamer-1.0 \
                      /usr/local/include/glib-2.0 \
                      /usr/local/lib/glib-2.0/include \
                      /opt/local/include/gstreamer-1.0 \
                      /opt/local/include/glib-2.0 \
                      /opt/local/lib/glib-2.0/include }

linux: { INCLUDEPATH += /usr/include/gstreamer-1.0 \
                        /usr/include/glib-2.0 \
                        /usr/lib/x86_64-linux-gnu/glib-2.0/include }

include(src/gui/gui.pri)
include(src/common.pri)
include(src/client.pri)
include(src/tests/tests.pri)
include(other_files.pri)


####################### BISON PARSER #######################

#BISON_BIN = bison

#bison_comp.input = BISON_FILES
##bison_comp.output = ./${QMAKE_FILE_BASE}.cc
#bison_comp.commands = $$BISON_BIN ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_BASE}.cc --defines=./${QMAKE_FILE_BASE}.hh
#QMAKE_EXTRA_COMPILERS += bison_comp






