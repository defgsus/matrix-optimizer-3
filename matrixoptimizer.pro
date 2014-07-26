
#################### qt stuff #########################

TARGET = matrixoptimizer

QT += core gui widgets opengl

TEMPLATE = app

##################### flags ###########################

CONFIG += c++11

QMAKE_CXXFLAGS_RELEASE += -O2 -DNDEBUG

# for optirun bug
unix: { DEFINES += MO_DISABLE_OBJECT_TREE_DRAG }

##################### libs ############################

unix: { LIBS += -lX11 -lportaudio -lsndfile }

win32: { LIBS += -lkernel32 -lpsapi -lportaudio -lsndfile-1 }

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
    src/io/init.cpp \
    src/object/model3d.cpp \
    src/gui/parameterview.cpp \
    src/types/float.cpp \
    src/io/applicationtime.cpp \
    src/gui/objectview.cpp \
    src/object/sequence.cpp \
    src/object/sequences.cpp \
    src/object/sequencefloat.cpp \
    src/gui/sequenceview.cpp \
    src/gui/sequencefloatview.cpp \
    src/gui/painter/valuecurve.cpp \
    src/gui/painter/sequenceoverpaint.cpp \
    src/gui/generalsequencefloatview.cpp \
    src/math/noiseperlin.cpp \
    src/math/funcparser/parser.cpp \
    src/object/track.cpp \
    src/gui/trackview.cpp \
    src/gui/sequencer.cpp \
    src/gui/widget/sequencewidget.cpp \
    src/gui/trackheader.cpp \
    src/gui/widget/trackheaderwidget.cpp \
    src/gui/widget/doublespinbox.cpp \
    src/gui/widget/equationeditor.cpp \
    src/tool/syntaxhighlighter.cpp \
    src/model/objecttreesortproxy.cpp \
    src/object/trackfloat.cpp \
    src/object/transform/axisrotation.cpp \
    src/object/transform/transformation.cpp \
    src/object/transform/translation.cpp \
    src/object/transform/scale.cpp \
    src/object/param/parameter.cpp \
    src/object/param/parameterfloat.cpp \
    src/gui/widget/timebar.cpp \
    src/gui/trackviewoverpaint.cpp \
    src/gui/widget/spacer.cpp \
    src/tool/actionlist.cpp \
    src/tool/enumnames.cpp \
    src/object/transform/look.cpp \
    src/object/transform/lookat.cpp \
    src/object/transform/shear.cpp \
    src/object/transform/mix.cpp \
    src/io/memory.cpp \
    src/object/param/parameterselect.cpp \
    src/object/group.cpp \
    src/gui/util/objectmenu.cpp \
    src/io/settings.cpp \
    src/gui/util/scenesettings.cpp \
    src/audio/audiodevices.cpp \
    src/audio/audiodevice.cpp \
    src/gui/audiodialog.cpp \
    src/gui/objectinfodialog.cpp \
    src/gui/splashscreen.cpp \
    src/audio/wavetablegenerator.cpp \
    src/gui/widget/spinbox.cpp \
    src/engine/renderer.cpp \
    src/audio/waveform.cpp \
    src/gl/geometry.cpp \
    src/gl/geometryfactory.cpp \
    src/gl/drawable.cpp \
    src/gl/openglfunctions.cpp

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
    src/io/init.h \
    src/gl/openglfunctions.h \
    src/object/model3d.h \
    src/gui/parameterview.h \
    src/io/applicationtime.h \
    src/gui/objectview.h \
    src/object/sequence.h \
    src/object/sequences.h \
    src/object/sequencefloat.h \
    src/gui/sequenceview.h \
    src/gui/sequencefloatview.h \
    src/gui/painter/valuecurve.h \
    src/gui/painter/sequenceoverpaint.h \
    src/gui/generalsequencefloatview.h \
    src/math/random.h \
    src/math/noiseperlin.h \
    src/math/funcparser/functions.h \
    src/math/funcparser/parser.h \
    src/math/funcparser/parser_defines.h \
    src/math/funcparser/parser_program.h \
    src/object/track.h \
    src/gui/trackview.h \
    src/gui/sequencer.h \
    src/gui/widget/sequencewidget.h \
    src/gui/trackheader.h \
    src/gui/widget/trackheaderwidget.h \
    src/gui/widget/doublespinbox.h \
    src/gui/widget/equationeditor.h \
    src/tool/syntaxhighlighter.h \
    src/model/objecttreesortproxy.h \
    src/object/trackfloat.h \
    src/object/object_fwd.h \
    src/object/transform/axisrotation.h \
    src/object/transform/translation.h \
    src/object/transform/transformation.h \
    src/object/transform/scale.h \
    src/object/param/parameter.h \
    src/object/param/parameterfloat.h \
    src/gui/widget/timebar.h \
    src/gui/trackviewoverpaint.h \
    src/gui/widget/spacer.h \
    src/tool/actionlist.h \
    src/tool/enumnames.h \
    src/object/transform/look.h \
    src/object/transform/lookat.h \
    src/object/transform/shear.h \
    src/object/transform/mix.h \
    src/io/memory.h \
    src/object/param/parameterselect.h \
    src/object/group.h \
    src/gui/util/objectmenu.h \
    src/io/settings.h \
    src/gui/util/scenesettings.h \
    src/audio/configuration.h \
    src/types/int.h \
    src/audio/audiodevices.h \
    src/audio/audiodevice.h \
    src/gui/audiodialog.h \
    src/gui/objectinfodialog.h \
    src/gui/splashscreen.h \
    src/audio/wavetable.h \
    src/audio/wavetablegenerator.h \
    src/gui/widget/spinbox.h \
    src/engine/renderer.h \
    src/audio/waveform.h \
    src/gl/geometry.h \
    src/gl/geometryfactory.h \
    src/gl/drawable.h

BISON_FILES = \
    src/math/funcparser/grammar.y

FORMS += \
    src/gui/projectorsetupwidget.ui

RESOURCES += \
    icons.qrc \
    images.qrc

OTHER_FILES += $$BISON_FILES \
    TODO.txt

####################### BISON PARSER #######################

#BISON_BIN = bison

#bison_comp.input = BISON_FILES
##bison_comp.output = ./${QMAKE_FILE_BASE}.cc
#bison_comp.commands = $$BISON_BIN ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_BASE}.cc --defines=./${QMAKE_FILE_BASE}.hh
#QMAKE_EXTRA_COMPILERS += bison_comp
