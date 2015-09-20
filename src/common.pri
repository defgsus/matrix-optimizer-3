RESOURCES += \
    images.qrc \
    shaders.qrc \
    models.qrc \
    $$PWD/../templates.qrc

HEADERS += \
    src/types/vector.h \
    src/math/vector.h \
    src/math/constants.h \
    src/math/timeline1d.h \
    src/math/interpol.h \
    src/types/float.h \
    src/io/console.h \
    src/audio/audiosource.h \
    src/math/functions.h \
    src/io/log.h \
    src/io/error.h \
    src/io/streamoperators_qt.h \
    src/io/xmlstream.h \
    src/io/datastream.h \
    src/model/qobjecttreemodel.h \
    src/doc.h \
    src/object/object.h \
    src/tool/stringmanip.h \
    src/object/objectfactory.h \
    src/io/application.h \
    src/object/soundsource.h \
    src/object/microphone.h \
    src/object/camera.h \
    src/object/dummy.h \
    src/model/objecttreemimedata.h \
    src/gl/window.h \
    src/gl/context.h \
    src/object/scene.h \
    src/object/objectgl.h \
    src/gl/manager.h \
    src/io/init.h \
    src/object/model3d.h \
    src/io/applicationtime.h \
    src/math/random.h \
    src/math/noiseperlin.h \
    src/math/funcparser/functions.h \
    src/math/funcparser/parser.h \
    src/math/funcparser/parser_defines.h \
    src/math/funcparser/parser_program.h \
    src/tool/syntaxhighlighter.h \
    src/object/object_fwd.h \
    src/object/transform/axisrotation.h \
    src/object/transform/translation.h \
    src/object/transform/transformation.h \
    src/object/transform/scale.h \
    src/object/param/parameter.h \
    src/object/param/parameterfloat.h \
    src/tool/actionlist.h \
    src/tool/enumnames.h \
    src/object/transform/look.h \
    src/object/transform/lookat.h \
    src/object/transform/shear.h \
    src/object/transform/mix.h \
    src/io/memory.h \
    src/object/param/parameterselect.h \
    src/object/group.h \
    src/io/settings.h \
    src/audio/configuration.h \
    src/types/int.h \
    src/audio/audiodevices.h \
    src/audio/audiodevice.h \
    src/engine/renderer.h \
    src/gl/drawable.h \
    src/gl/shader.h \
    src/gl/shadersource.h \
    src/gl/cameraspace.h \
    src/gl/opengl.h \
    src/gl/vertexarrayobject.h \
    src/gl/opengl_fwd.h \
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
    src/io/files.h \
    src/object/param/modulator.h \
    src/object/param/modulatorfloat.h \
    src/io/currentthread.h \
    src/object/lightsource.h \
    src/gl/lightsettings.h \
    src/audio/audio_fwd.h \
    src/object/scenelock_p.h \
    src/audio/tool/envelopefollower.h \
    src/audio/tool/multifilter.h \
    src/audio/tool/waveform.h \
    src/audio/tool/wavetable.h \
    src/audio/tool/wavetablegenerator.h \
    src/tool/locklessqueue.h \
    src/io/lockedoutput.h \
    src/object/param/parameterint.h \
    src/audio/tool/chebychevfilter.h \
    src/object/textureoverlay.h \
    src/gl/rendersettings.h \
    src/object/param/parameterfilename.h \
    src/io/filetypes.h \
    src/geom/geometrymodifier.h \
    src/geom/geometrymodifierscale.h \
    src/geom/geometrymodifiertesselate.h \
    src/geom/geometrymodifierchain.h \
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
    src/object/sprite.h \
    src/object/util/texturesetting.h \
    src/geom/geometrymodifiertexcoords.h \
    src/geom/objexporter.h \
    src/object/util/colorpostprocessingsetting.h \
    src/audio/tool/soundfile.h \
    src/audio/tool/soundfilemanager.h \
    src/object/param/parametertext.h \
    src/math/intersection.h \
    src/projection/domesettings.h \
    src/projection/projectorsettings.h \
    src/projection/projectormapper.h \
    src/projection/projectionsystemsettings.h \
    src/projection/camerasettings.h \
    src/io/povrayexporter.h \
    src/gl/scenedebugrenderer.h \
    src/gl/compatibility.h \
    src/object/microphonegroup.h \
    src/audio/audiomicrophone.h \
    src/io/equationpresets.h \
    src/io/equationpreset.h \
    src/audio/mididevices.h \
    src/audio/mididevice.h \
    src/audio/midievent.h \
    src/network/netlog.h \
    src/network/networkmanager.h \
    src/network/tcpserver.h \
    src/network/netevent.h \
    src/io/systeminfo.h \
    src/network/network_fwd.h \
    src/gl/opengl_undef.h \
    src/io/streamoperators_glbinding.h \
    src/io/filemanager.h \
    src/tool/deleter.h \
    src/network/eventcom.h \
    src/geom/tesselator.h \
    src/audio/tool/notefreq.h \
    src/object/synthesizer.h \
    src/audio/tool/floatgate.h \
    src/audio/tool/synth.h \
    src/audio/tool/envelopegenerator.h \
    src/audio/tool/filter24.h \
    src/math/denormals.h \
    src/audio/tool/butterworthfilter.h \
    src/audio/tool/fixedfilter.h \
    src/object/util/synthsetting.h \
    src/object/util/objectfilter.h \
    src/geom/geometrymodifierduplicate.h \
    src/math/polygon.h \
    src/audio/tool/bandlimitwavetablegenerator.h \
    src/math/fft.h \
    src/object/param/parametertimeline1d.h \
    src/audio/tool/fftwavetablegenerator.h \
    src/projection/testprojectionrenderer.h \
    src/io/commandlineparser.h \
    src/io/version.h \
    src/tool/selection.h \
    src/graph/directedgraph.h \
    src/object/util/objectmodulatorgraph.h \
    src/graph/tree.h \
    src/projection/projectorblender.h \
    src/video/videostreamreader.h \
    src/tool/commonresolutions.h \
    src/gl/scenerenderer.h \
    src/object/util/alphablendsetting.h \
    src/object/util/texturemorphsetting.h \
    src/io/currenttime.h \
    src/network/clientstate.h \
    src/io/qtextstreamoperators.h \
    src/io/helpsystem.h \
    src/io/helpexporterhtml.h \
    src/io/helpexporterlatex.h \
    src/io/isclient.h \
    src/engine/serverengine.h \
    src/object/util/objecteditor.h \
    src/types/time.h \
    src/audio/tool/audiobuffer.h \
    src/object/audioobject.h \
    src/object/param/parameters.h \
    src/object/audio/oscillatorao.h \
    src/object/util/audioobjectconnections.h \
    src/object/audio/audiooutao.h \
    src/engine/liveaudioengine.h \
    src/engine/audioengine.h \
    src/object/util/objectdsppath.h \
    src/object/audio/filterao.h \
    src/math/transformationbuffer.h \
    src/model/objectmimedata.h \
    src/object/audio/mverbao.h \
    src/audio/3rd/MVerb.h \
    src/object/audio/fftao.h \
    src/audio/tool/resamplebuffer.h \
    src/object/audio/shaperao.h \
    src/audio/spatial/spatialsoundsource.h \
    src/audio/spatial/spatialmicrophone.h \
    src/types/conversion.h \
    src/audio/tool/delay.h \
    src/object/audio/audioinao.h \
    src/object/param/modulatorevent.h \
    src/object/audio/filterbankao.h \
    src/object/param/modulatoroutput.h \
    src/object/audio/envelopefollowerao.h \
    src/gl/bufferobject.h \
    src/object/oscillograph.h \
    src/object/audio/delayao.h \
    src/object/audio/soundsourceao.h \
    src/object/audio/parameterao.h \
    src/object/audio/impulseao.h \
    src/object/audio/panao.h \
    src/object/audio/dustao.h \
    src/object/audio/bandfilterbankao.h \
    src/audio/tool/dumbfile.h \
    src/object/audio/modplayerao.h \
    src/geom/geometrymodifierangelscript.h \
    src/script/3rd/angelscript/scriptmath/scriptmathcomplex.h \
    src/script/3rd/angelscript/scriptarray/scriptarray.h \
    src/script/3rd/angelscript/scriptstdstring/scriptstdstring.h \
    src/script/angelscript_vector.h \
    src/script/angelscript.h \
    src/math/advanced.h \
    src/script/angelscript_math.h \
    src/script/angelscript_object.h \
    src/script/angelscript_geometry.h \
    src/geom/builtinlinefont.h \
    src/script/angelscript_timeline.h \
    src/object/ascriptobject.h \
    src/object/util/useruniformsetting.h \
    src/geom/pointcloud.h \
    src/object/util/objectglpath.h \
    src/engine/renderengine.h \
    src/geom/marchingcubes.h \
    src/network/udpconnection.h \
    src/script/angelscript_network.h \
    src/network/udpaudioconnection.h \
    src/types/refcounted.h \
    src/script/angelscript_image.h \
    src/io/time.h \
    src/tool/asciirect.h \
    src/types/properties.h \
    src/io/docbookexporter.h \
    src/maincommandline.h \
    src/gl/offscreencontext.h \
    src/io/diskrendersettings.h \
    src/engine/diskrenderer.h \
    src/object/shaderobject.h \
    src/tool/threadpool.h \
    src/object/interface/valuefloatinterface.h \
    src/object/control/sequencefloat.h \
    src/object/control/sequence.h \
    src/object/control/modulatorobject.h \
    src/object/control/modulatorobjectfloat.h \
    src/object/control/sequences.h \
    src/object/control/track.h \
    src/object/control/trackfloat.h \
    src/object/control/clip.h \
    src/object/control/clipcontroller.h \
    src/object/control/derivativeobjectfloat.h \
    src/audio/spatial/wavetracershader.h \
    src/audio/tool/irmap.h \
    src/audio/audioplayer.h \
    src/audio/audioplayerdata.h \
    src/math/convolution.h \
    src/object/audio/sampleholdao.h \
    src/object/audio/phasorao.h \
    src/object/audio/noiseao.h \
    src/object/audio/playbufferao.h \
    src/object/audio/convolveao.h \
    src/audio/tool/convolvebuffer.h \
    src/geom/geometrymodifierenum.h \
    src/io/ladspaloader.h \
    src/audio/tool/ladspaplugin.h \
    src/object/audio/pluginao.h \
    src/object/param/parametercallback.h \
    src/object/audio/waveplayerao.h \
    src/audio/3rd/ladspa.h \
    src/object/param/modulatortexture.h \
    src/object/interface/valuetextureinterface.h \
    src/object/param/parametertexture.h \
    src/object/texture/textureobjectbase.h \
    src/object/texture/colorto.h \
    src/object/texture/blurto.h \
    src/object/texture/imageto.h \
    src/object/textobject.h \
    src/object/interface/valuetextinterface.h \
    src/object/texture/keyto.h \
    src/object/texture/mixto.h \
    src/object/param/parametertransformation.h \
    src/object/interface/valuetransformationinterface.h \
    src/object/transform/transformationinput.h \
    src/object/param/modulatortransformation.h \
    src/object/transform/mirrortrans.h \
    src/object/transform/cleartrans.h \
    src/object/texture/randomto.h \
    src/object/texture/normalmapto.h \
    src/object/texture/lensdistto.h \
    src/gl/texturerenderer.h \
    src/object/audio/microphoneao.h \
    src/object/texture/generate3dto.h \
    $$PWD/io/sswproject.h \
    $$PWD/model/jsontreemodel.h \
    $$PWD/geom/shploader.h \
    $$PWD/audio/tool/soundfileistream.h \
    $$PWD/io/imagereader.h \
    $$PWD/object/texture/distancemapto.h


SOURCES += \
    src/math/timeline1d.cpp \
    src/io/console.cpp \
    src/audio/audiosource.cpp \
    src/io/xmlstream.cpp \
    src/io/datastream.cpp \
    src/model/qobjecttreemodel.cpp \
    src/object/object.cpp \
    src/tool/stringmanip.cpp \
    src/object/objectfactory.cpp \
    src/io/application.cpp \
    src/object/soundsource.cpp \
    src/object/microphone.cpp \
    src/object/camera.cpp \
    src/object/dummy.cpp \
    src/model/objecttreemimedata.cpp \
    src/gl/window.cpp \
    src/gl/context.cpp \
    src/object/scene.cpp \
    src/object/objectgl.cpp \
    src/gl/manager.cpp \
    src/io/init.cpp \
    src/object/model3d.cpp \
    src/types/float.cpp \
    src/io/applicationtime.cpp \
    src/math/noiseperlin.cpp \
    src/math/funcparser/parser.cpp \
    src/tool/syntaxhighlighter.cpp \
    src/object/transform/axisrotation.cpp \
    src/object/transform/transformation.cpp \
    src/object/transform/translation.cpp \
    src/object/transform/scale.cpp \
    src/object/param/parameter.cpp \
    src/object/param/parameterfloat.cpp \
    src/tool/actionlist.cpp \
    src/tool/enumnames.cpp \
    src/object/transform/look.cpp \
    src/object/transform/lookat.cpp \
    src/object/transform/shear.cpp \
    src/object/transform/mix.cpp \
    src/io/memory.cpp \
    src/object/param/parameterselect.cpp \
    src/object/group.cpp \
    src/io/settings.cpp \
    src/audio/audiodevices.cpp \
    src/audio/audiodevice.cpp \
    src/engine/renderer.cpp \
    src/gl/drawable.cpp \
    src/gl/shader.cpp \
    src/gl/shadersource.cpp \
    src/gl/opengl.cpp \
    src/gl/vertexarrayobject.cpp \
    src/geom/objloader.cpp \
    src/geom/geometry.cpp \
    src/geom/geometryfactory.cpp \
    src/gl/texture.cpp \
    src/gl/framebufferobject.cpp \
    src/gl/screenquad.cpp \
    src/math/cubemapmatrix.cpp \
    src/geom/geometrycreator.cpp \
    src/geom/freecamera.cpp \
    src/io/files.cpp \
    src/object/param/modulator.cpp \
    src/object/param/modulatorfloat.cpp \
    src/io/currentthread.cpp \
    src/object/lightsource.cpp \
    src/gl/lightsettings.cpp \
    src/audio/tool/envelopefollower.cpp \
    src/audio/tool/multifilter.cpp \
    src/audio/tool/waveform.cpp \
    src/audio/tool/wavetablegenerator.cpp \
    src/io/lockedoutput.cpp \
    src/object/param/parameterint.cpp \
    src/audio/tool/chebychevfilter.cpp \
    src/object/textureoverlay.cpp \
    src/gl/rendersettings.cpp \
    src/object/param/parameterfilename.cpp \
    src/io/filetypes.cpp \
    src/geom/geometrymodifier.cpp \
    src/geom/geometrymodifierscale.cpp \
    src/geom/geometrymodifiertesselate.cpp \
    src/geom/geometrymodifierchain.cpp \
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
    src/object/sprite.cpp \
    src/object/util/texturesetting.cpp \
    src/geom/geometrymodifiertexcoords.cpp \
    src/geom/objexporter.cpp \
    src/object/util/colorpostprocessingsetting.cpp \
    src/audio/tool/soundfile.cpp \
    src/audio/tool/soundfilemanager.cpp \
    src/object/param/parametertext.cpp \
    src/math/intersection.cpp \
    src/projection/domesettings.cpp \
    src/projection/projectorsettings.cpp \
    src/projection/projectormapper.cpp \
    src/projection/projectionsystemsettings.cpp \
    src/projection/camerasettings.cpp \
    src/io/povrayexporter.cpp \
    src/gl/scenedebugrenderer.cpp \
    src/gl/compatibility.cpp \
    src/object/microphonegroup.cpp \
    src/audio/audiomicrophone.cpp \
    src/io/equationpresets.cpp \
    src/io/equationpreset.cpp \
    src/audio/mididevices.cpp \
    src/audio/mididevice.cpp \
    src/audio/midievent.cpp \
    src/network/netlog.cpp \
    src/network/networkmanager.cpp \
    src/network/tcpserver.cpp \
    src/network/netevent.cpp \
    src/io/systeminfo.cpp \
    src/io/filemanager.cpp \
    src/network/eventcom.cpp \
    src/geom/tesselator.cpp \
    src/object/synthesizer.cpp \
    src/audio/tool/synth.cpp \
    src/audio/tool/filter24.cpp \
    src/math/denormals.cpp \
    src/audio/tool/butterworthfilter.cpp \
    src/audio/tool/fixedfilter.cpp \
    src/object/util/synthsetting.cpp \
    src/object/util/objectfilter.cpp \
    src/geom/geometrymodifierduplicate.cpp \
    src/math/polygon.cpp \
    src/projection/projectormapper_gl.cpp \
    src/audio/tool/bandlimitwavetablegenerator.cpp \
    src/math/fft.cpp \
    src/object/param/parametertimeline1d.cpp \
    src/projection/testprojectionrenderer.cpp \
    src/io/commandlineparser.cpp \
    src/io/version.cpp \
    src/object/util/objectmodulatorgraph.cpp \
    src/object/util/objecttree.cpp \
    src/projection/projectorblender.cpp \
    src/video/videostreamreader.cpp \
    src/tool/commonresolutions.cpp \
    src/gl/scenerenderer.cpp \
    src/object/util/alphablendsetting.cpp \
    src/object/util/texturemorphsetting.cpp \
    src/io/currenttime.cpp \
    src/network/clientstate.cpp \
    src/io/qtextstreamoperators.cpp \
    src/io/helpsystem.cpp \
    src/io/helpexporterhtml.cpp \
    src/io/helpexporterlatex.cpp \
    src/io/isclient.cpp \
    src/main.cpp \
    src/engine/serverengine.cpp \
    src/object/util/objecteditor.cpp \
    src/audio/tool/audiobuffer.cpp \
    src/object/audioobject.cpp \
    src/object/param/parameters.cpp \
    src/object/audio/oscillatorao.cpp \
    src/object/util/audioobjectconnections.cpp \
    src/object/audio/audiooutao.cpp \
    src/engine/liveaudioengine.cpp \
    src/engine/audioengine.cpp \
    src/object/util/objectdsppath.cpp \
    src/object/audio/filterao.cpp \
    src/model/objectmimedata.cpp \
    src/object/audio/mverbao.cpp \
    src/object/audio/fftao.cpp \
    src/object/audio/shaperao.cpp \
    src/audio/spatial/spatialsoundsource.cpp \
    src/audio/spatial/spatialmicrophone.cpp \
    src/object/audio/audioinao.cpp \
    src/object/param/modulatorevent.cpp \
    src/object/audio/filterbankao.cpp \
    src/object/param/modulatoroutput.cpp \
    src/object/audio/envelopefollowerao.cpp \
    src/gl/bufferobject.cpp \
    src/object/oscillograph.cpp \
    src/object/audio/delayao.cpp \
    src/object/audio/soundsourceao.cpp \
    src/object/audio/parameterao.cpp \
    src/object/audio/impulseao.cpp \
    src/object/audio/panao.cpp \
    src/object/audio/dustao.cpp \
    src/object/audio/bandfilterbankao.cpp \
    src/audio/tool/dumbfile.cpp \
    src/object/audio/modplayerao.cpp \
    src/geom/geometrymodifierangelscript.cpp \
    src/script/3rd/angelscript/scriptmath/scriptmathcomplex.cpp \
    src/script/3rd/angelscript/scriptarray/scriptarray.cpp \
    src/script/3rd/angelscript/scriptstdstring/scriptstdstring.cpp \
    src/script/3rd/angelscript/scriptstdstring/scriptstdstring_utils.cpp \
    src/script/angelscript_vector.cpp \
    src/script/angelscript.cpp \
    src/math/advanced.cpp \
    src/script/angelscript_math.cpp \
    src/script/angelscript_object.cpp \
    src/script/angelscript_geometry.cpp \
    src/geom/builtinlinefont.cpp \
    src/script/angelscript_timeline.cpp \
    src/object/ascriptobject.cpp \
    src/object/util/useruniformsetting.cpp \
    src/geom/pointcloud.cpp \
    src/object/util/objectglpath.cpp \
    src/engine/renderengine.cpp \
    src/geom/marchingcubes.cpp \
    src/network/udpconnection.cpp \
    src/script/angelscript_network.cpp \
    src/network/udpaudioconnection.cpp \
    src/script/angelscript_image.cpp \
    src/io/time.cpp \
    src/tool/asciirect.cpp \
    src/types/properties.cpp \
    src/io/docbookexporter.cpp \
    src/maincommandline.cpp \
    src/gl/offscreencontext.cpp \
    src/io/diskrendersettings.cpp \
    src/engine/diskrenderer.cpp \
    src/object/shaderobject.cpp \
    src/tool/threadpool.cpp \
    src/object/control/sequencefloat.cpp \
    src/object/control/sequence.cpp \
    src/object/control/modulatorobject.cpp \
    src/object/control/modulatorobjectfloat.cpp \
    src/object/control/sequences.cpp \
    src/object/control/track.cpp \
    src/object/control/trackfloat.cpp \
    src/object/control/clip.cpp \
    src/object/control/clipcontroller.cpp \
    src/object/control/derivativeobjectfloat.cpp \
    src/audio/spatial/wavetracershader.cpp \
    src/audio/tool/irmap.cpp \
    src/audio/audioplayer.cpp \
    src/audio/audioplayerdata.cpp \
    src/math/convolution.cpp \
    src/object/audio/sampleholdao.cpp \
    src/object/audio/phasorao.cpp \
    src/object/audio/noiseao.cpp \
    src/object/audio/playbufferao.cpp \
    src/object/audio/convolveao.cpp \
    src/audio/tool/convolvebuffer.cpp \
    src/geom/geometrymodifierenum.cpp \
    src/io/ladspaloader.cpp \
    src/audio/tool/ladspaplugin.cpp \
    src/object/audio/pluginao.cpp \
    src/object/param/parametercallback.cpp \
    src/object/audio/waveplayerao.cpp \
    src/object/param/modulatortexture.cpp \
    src/object/param/parametertexture.cpp \
    src/object/texture/textureobjectbase.cpp \
    src/object/texture/colorto.cpp \
    src/object/texture/blurto.cpp \
    src/object/texture/imageto.cpp \
    src/object/textobject.cpp \
    src/object/texture/keyto.cpp \
    src/object/texture/mixto.cpp \
    src/object/param/parametertransformation.cpp \
    src/object/transform/transformationinput.cpp \
    src/object/param/modulatortransformation.cpp \
    src/object/transform/mirrortrans.cpp \
    src/object/transform/cleartrans.cpp \
    src/object/texture/randomto.cpp \
    src/object/texture/normalmapto.cpp \
    src/object/texture/lensdistto.cpp \
    src/gl/texturerenderer.cpp \
    src/object/audio/microphoneao.cpp \
    src/object/texture/generate3dto.cpp \
    $$PWD/io/sswproject.cpp \
    $$PWD/model/jsontreemodel.cpp \
    $$PWD/geom/shploader.cpp \
    $$PWD/audio/tool/soundfileistream.cpp \
    $$PWD/io/imagereader.cpp \
    $$PWD/object/texture/distancemapto.cpp
