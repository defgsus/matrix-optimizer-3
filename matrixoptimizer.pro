
#################### qt stuff #########################

TARGET = matrixoptimizer

QT += core gui widgets opengl network xml

TEMPLATE = app

##################### flags ###########################

CONFIG += c++11

#QMAKE_CXXFLAGS_DEBUG += -pg

QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

DEFINES += GLEW_MX

# for optirun bug
unix: { DEFINES += MO_DISABLE_OBJECT_TREE_DRAG }

##################### libs ############################

unix: { LIBS += -lGLEWmx -lGLU -lGL -lX11 -lportaudio -lsndfile }

win32: { LIBS += -lkernel32 -lpsapi -lportaudio -lsndfile-1 }

###################### files ##########################

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/gui/mainwindow.cpp \
    src/gui/basic3dview.cpp \
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
    src/gui/widget/spinbox.cpp \
    src/engine/renderer.cpp \
    src/gl/drawable.cpp \
    src/gl/shader.cpp \
    src/gl/shadersource.cpp \
    src/gl/opengl.cpp \
    src/gl/vertexarrayobject.cpp \
    src/gui/widget/basic3dwidget.cpp \
    src/gui/widget/geometrywidget.cpp \
    src/gui/geometrydialog.cpp \
    src/geom/objloader.cpp \
    src/geom/geometry.cpp \
    src/geom/geometryfactory.cpp \
    src/gl/texture.cpp \
    src/gl/framebufferobject.cpp \
    src/gl/screenquad.cpp \
    src/math/cubemapmatrix.cpp \
    src/geom/geometrycreator.cpp \
    src/geom/freecamera.cpp \
    src/img/image.cpp \
    src/io/files.cpp \
    src/object/param/modulator.cpp \
    src/object/param/modulatorfloat.cpp \
    src/gui/widget/modulatorwidget.cpp \
    src/gui/modulatordialog.cpp \
    src/io/currentthread.cpp \
    src/img/imagegenerator.cpp \
    src/object/lightsource.cpp \
    src/gl/lightsettings.cpp \
    src/object/audio/audiounit.cpp \
    src/object/audio/filterunit.cpp \
    src/object/scene_audio.cpp \
    src/audio/tool/envelopefollower.cpp \
    src/audio/tool/multifilter.cpp \
    src/audio/tool/waveform.cpp \
    src/audio/tool/wavetablegenerator.cpp \
    src/object/audio/envelopeunit.cpp \
    src/gui/widget/envelopewidget.cpp \
    src/gui/widget/transportwidget.cpp \
    src/io/lockedoutput.cpp \
    src/object/modulatorobject.cpp \
    src/object/modulatorobjectfloat.cpp \
    src/object/audio/filterbankunit.cpp \
    src/object/param/parameterint.cpp \
    src/audio/tool/chebychevfilter.cpp \
    src/object/textureoverlay.cpp \
    src/gl/rendersettings.cpp \
    src/object/param/parameterfilename.cpp \
    src/io/filetypes.cpp \
    src/gui/widget/groupwidget.cpp \
    src/geom/geometrymodifier.cpp \
    src/geom/geometrymodifierscale.cpp \
    src/geom/geometrymodifiertesselate.cpp \
    src/geom/geometrymodifierchain.cpp \
    src/gui/widget/geometrymodifierwidget.cpp \
    src/geom/geometrymodifiercreate.cpp \
    src/geom/geometrymodifiertranslate.cpp \
    src/geom/geometrymodifierrotate.cpp \
    src/geom/geometrymodifiernormalize.cpp \
    src/geom/geometrymodifiernormals.cpp \
    src/geom/geometrymodifierconvertlines.cpp \
    src/geom/geometrymodifiervertexgroup.cpp \
    src/geom/geometrymodifierremove.cpp \
    src/geom/geometrymodifiervertexequation.cpp \
    src/geom/geometrymodifierprimitiveequation.cpp \
    src/geom/geometrymodifierextrude.cpp \
    src/geom/geometryfactorysettings.cpp \
    src/gui/widget/audiounitwidget.cpp \
    src/gui/audiolinkwindow.cpp \
    src/gui/audiolinkview.cpp \
    src/gui/widget/audiounitconnectorwidget.cpp \
    src/gui/painter/audiolinkviewoverpaint.cpp \
    src/gui/painter/objecttreeviewoverpaint.cpp \
    src/object/sprite.cpp \
    src/object/util/texturesetting.cpp \
    src/geom/geometrymodifiertexcoords.cpp \
    src/gui/widget/doublespinboxclean.cpp \
    src/geom/geometrymodifiertexcoordequation.cpp \
    src/geom/objexporter.cpp \
    src/gui/geometryexportdialog.cpp \
    src/object/util/colorpostprocessingsetting.cpp \
    src/audio/tool/soundfile.cpp \
    src/audio/tool/soundfilemanager.cpp \
    src/object/param/parametertext.cpp \
    src/gui/texteditdialog.cpp \
    src/gui/sceneconvertdialog.cpp \
    src/math/intersection.cpp \
    src/gui/widget/domepreviewwidget.cpp \
    src/gui/projectorsetupdialog.cpp \
    src/projection/domesettings.cpp \
    src/projection/projectorsettings.cpp \
    src/projection/projectormapper.cpp \
    src/projection/projectionsystemsettings.cpp \
    src/projection/camerasettings.cpp \
    src/network/networkmanager.cpp \
    src/gui/helpdialog.cpp \
    src/gui/widget/helptextbrowser.cpp \
    src/io/povrayexporter.cpp \
    src/gl/scenedebugrenderer.cpp \
    src/gl/compatibility.cpp \
    src/object/microphonegroup.cpp \
    src/audio/audiomicrophone.cpp \
    src/gui/widget/equationdisplaywidget.cpp \
    src/gui/equationdisplaydialog.cpp \
    src/io/equationpresets.cpp \
    src/io/equationpreset.cpp \
    src/io/helpsystem.cpp \
    src/tests/testhelpsystem.cpp \
    src/io/helpexporterhtml.cpp \
    src/io/helpexporterlatex.cpp \
    src/gui/networkdialog.cpp \
    src/network/tcpserver.cpp \
    src/network/netlog.cpp

HEADERS += \
    src/gui/mainwindow.h \
    src/types/vector.h \
    src/gui/basic3dview.h \
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
    src/gui/widget/spinbox.h \
    src/engine/renderer.h \
    src/gl/drawable.h \
    src/gl/shader.h \
    src/gl/shadersource.h \
    src/gl/cameraspace.h \
    src/gl/opengl.h \
    src/gl/vertexarrayobject.h \
    src/gui/widget/basic3dwidget.h \
    src/gui/widget/geometrywidget.h \
    src/gl/opengl_fwd.h \
    src/gui/geometrydialog.h \
    src/math/hash.h \
    src/geom/objloader.h \
    src/geom/geometry.h \
    src/geom/geometryfactory.h \
    src/gl/texture.h \
    src/gl/framebufferobject.h \
    src/gl/screenquad.h \
    src/math/cubemapmatrix.h \
    src/geom/geometrycreator.h \
    src/geom/freecamera.h \
    src/img/image.h \
    src/io/files.h \
    src/object/param/modulator.h \
    src/object/param/modulatorfloat.h \
    src/gui/widget/modulatorwidget.h \
    src/gui/modulatordialog.h \
    src/io/currentthread.h \
    src/img/imagegenerator.h \
    src/object/lightsource.h \
    src/gl/lightsettings.h \
    src/object/audio/audiounit.h \
    src/object/audio/filterunit.h \
    src/audio/audio_fwd.h \
    src/object/scenelock_p.h \
    src/audio/tool/envelopefollower.h \
    src/audio/tool/multifilter.h \
    src/audio/tool/waveform.h \
    src/audio/tool/wavetable.h \
    src/audio/tool/wavetablegenerator.h \
    src/object/audio/envelopeunit.h \
    src/gui/widget/envelopewidget.h \
    src/gui/widget/transportwidget.h \
    src/tool/locklessqueue.h \
    src/io/lockedoutput.h \
    src/object/modulatorobject.h \
    src/object/modulatorobjectfloat.h \
    src/object/audio/filterbankunit.h \
    src/object/param/parameterint.h \
    src/audio/tool/chebychevfilter.h \
    src/object/textureoverlay.h \
    src/gl/rendersettings.h \
    src/object/param/parameterfilename.h \
    src/io/filetypes.h \
    src/gui/widget/groupwidget.h \
    src/geom/geometrymodifier.h \
    src/geom/geometrymodifierscale.h \
    src/geom/geometrymodifiertesselate.h \
    src/geom/geometrymodifierchain.h \
    src/gui/widget/geometrymodifierwidget.h \
    src/geom/geometrymodifiercreate.h \
    src/geom/geometrymodifiertranslate.h \
    src/geom/geometrymodifierrotate.h \
    src/geom/geometrymodifiernormalize.h \
    src/geom/geometrymodifiernormals.h \
    src/geom/geometrymodifierconvertlines.h \
    src/geom/geometrymodifiervertexgroup.h \
    src/geom/geometrymodifierremove.h \
    src/geom/geometrymodifiervertexequation.h \
    src/geom/geometrymodifierprimitiveequation.h \
    src/geom/geometrymodifierextrude.h \
    src/geom/geometryfactorysettings.h \
    src/gui/widget/audiounitwidget.h \
    src/gui/audiolinkwindow.h \
    src/gui/audiolinkview.h \
    src/gui/widget/audiounitconnectorwidget.h \
    src/gui/painter/audiolinkviewoverpaint.h \
    src/gui/painter/objecttreeviewoverpaint.h \
    src/object/sprite.h \
    src/object/util/texturesetting.h \
    src/geom/geometrymodifiertexcoords.h \
    src/gui/widget/doublespinboxclean.h \
    src/geom/geometrymodifiertexcoordequation.h \
    src/geom/objexporter.h \
    src/gui/geometryexportdialog.h \
    src/object/util/colorpostprocessingsetting.h \
    src/audio/tool/soundfile.h \
    src/audio/tool/soundfilemanager.h \
    src/object/param/parametertext.h \
    src/gui/texteditdialog.h \
    src/gui/sceneconvertdialog.h \
    src/math/intersection.h \
    src/gui/widget/domepreviewwidget.h \
    src/gui/projectorsetupdialog.h \
    src/projection/domesettings.h \
    src/projection/projectorsettings.h \
    src/projection/projectormapper.h \
    src/projection/projectionsystemsettings.h \
    src/projection/camerasettings.h \
    src/network/networkmanager.h \
    src/gui/helpdialog.h \
    src/gui/widget/helptextbrowser.h \
    src/io/povrayexporter.h \
    src/gl/scenedebugrenderer.h \
    src/gl/compatibility.h \
    src/object/microphonegroup.h \
    src/audio/audiomicrophone.h \
    src/gui/widget/equationdisplaywidget.h \
    src/gui/equationdisplaydialog.h \
    src/io/equationpresets.h \
    src/io/equationpreset.h \
    src/io/helpsystem.h \
    src/tests/testhelpsystem.h \
    src/io/helpexporterhtml.h \
    src/io/helpexporterlatex.h \
    src/gui/networkdialog.h \
    src/network/tcpserver.h \
    src/network/netlog.h

BISON_FILES = \
    src/math/funcparser/grammar.y

FORMS +=

RESOURCES += \
    icons.qrc \
    images.qrc \
    shaders.qrc \
    models.qrc \
    help.qrc

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
    assets/help/en/sequence.html

####################### BISON PARSER #######################

#BISON_BIN = bison

#bison_comp.input = BISON_FILES
##bison_comp.output = ./${QMAKE_FILE_BASE}.cc
#bison_comp.commands = $$BISON_BIN ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_BASE}.cc --defines=./${QMAKE_FILE_BASE}.hh
#QMAKE_EXTRA_COMPILERS += bison_comp
