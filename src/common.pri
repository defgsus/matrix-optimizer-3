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
    src/tool/stringmanip.h \
    src/io/application.h \
    src/model/objecttreemimedata.h \
    src/gl/window.h \
    src/gl/context.h \
    src/gl/manager.h \
    src/io/init.h \
    src/io/applicationtime.h \
    src/math/random.h \
    src/math/noiseperlin.h \
    src/math/funcparser/functions.h \
    src/math/funcparser/parser.h \
    src/math/funcparser/parser_defines.h \
    src/math/funcparser/parser_program.h \
    src/tool/syntaxhighlighter.h \
    src/tool/actionlist.h \
    src/tool/enumnames.h \
    src/io/memory.h \
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
    src/io/currentthread.h \
    src/gl/lightsettings.h \
    src/audio/audio_fwd.h \
    src/audio/tool/envelopefollower.h \
    src/audio/tool/multifilter.h \
    src/audio/tool/waveform.h \
    src/audio/tool/wavetable.h \
    src/audio/tool/wavetablegenerator.h \
    src/tool/locklessqueue.h \
    src/io/lockedoutput.h \
    src/audio/tool/chebychevfilter.h \
    src/gl/rendersettings.h \
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
    src/geom/geometrymodifiertexcoords.h \
    src/geom/objexporter.h \
    src/audio/tool/soundfile.h \
    src/audio/tool/soundfilemanager.h \
    src/math/intersection.h \
    src/projection/domesettings.h \
    src/projection/projectorsettings.h \
    src/projection/projectormapper.h \
    src/projection/projectionsystemsettings.h \
    src/projection/camerasettings.h \
    src/io/povrayexporter.h \
    src/gl/scenedebugrenderer.h \
    src/gl/compatibility.h \
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
    src/audio/tool/floatgate.h \
    src/audio/tool/synth.h \
    src/audio/tool/envelopegenerator.h \
    src/audio/tool/filter24.h \
    src/math/denormals.h \
    src/audio/tool/butterworthfilter.h \
    src/audio/tool/fixedfilter.h \
    src/geom/geometrymodifierduplicate.h \
    src/math/polygon.h \
    src/audio/tool/bandlimitwavetablegenerator.h \
    src/math/fft.h \
    src/audio/tool/fftwavetablegenerator.h \
    src/projection/testprojectionrenderer.h \
    src/io/commandlineparser.h \
    src/io/version.h \
    src/tool/selection.h \
    src/graph/directedgraph.h \
    src/graph/tree.h \
    src/projection/projectorblender.h \
    src/video/videostreamreader.h \
    src/tool/commonresolutions.h \
    src/gl/scenerenderer.h \
    src/io/currenttime.h \
    src/network/clientstate.h \
    src/io/qtextstreamoperators.h \
    src/io/helpsystem.h \
    src/io/helpexporterhtml.h \
    src/io/helpexporterlatex.h \
    src/io/isclient.h \
    src/engine/serverengine.h \
    src/types/time.h \
    src/audio/tool/audiobuffer.h \
    src/engine/liveaudioengine.h \
    src/engine/audioengine.h \
    src/math/transformationbuffer.h \
    src/model/objectmimedata.h \
    src/audio/3rd/MVerb.h \
    src/audio/tool/resamplebuffer.h \
    src/audio/spatial/spatialsoundsource.h \
    src/audio/spatial/spatialmicrophone.h \
    src/types/conversion.h \
    src/audio/tool/delay.h \
    src/gl/bufferobject.h \
    src/audio/tool/dumbfile.h \
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
    src/geom/pointcloud.h \
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
    src/tool/threadpool.h \
    src/audio/spatial/wavetracershader.h \
    src/audio/tool/irmap.h \
    src/audio/audioplayer.h \
    src/audio/audioplayerdata.h \
    src/math/convolution.h \
    src/audio/tool/convolvebuffer.h \
    src/geom/geometrymodifierenum.h \
    src/io/ladspaloader.h \
    src/audio/tool/ladspaplugin.h \
    src/audio/3rd/ladspa.h \
    src/gl/texturerenderer.h \
    $$PWD/io/sswproject.h \
    $$PWD/model/jsontreemodel.h \
    $$PWD/geom/shploader.h \
    $$PWD/audio/tool/soundfileistream.h \
    $$PWD/io/imagereader.h \
    $$PWD/tool/dfdownsampler.h \
    $$PWD/geom/geometrymodifiertext.h \
    $$PWD/geom/textmesh.h \
    $$PWD/tool/linearizerfloat.h \
    $$PWD/network/oscinput.h \
    $$PWD/network/oscinputs.h \
    $$PWD/tool/valuesmoother.h \
    $$PWD/math/interpolationtype.h \
    $$PWD/io/keyboardstate.h \
    $$PWD/io/mousestate.h \
    $$PWD/math/csgbase.h \
    $$PWD/math/csgprimitives.h \
    $$PWD/math/csgcombine.h \
    $$PWD/model/csgtreemodel.h \
    $$PWD/gl/csgshader.h \
    $$PWD/math/csgdeform.h \
    $$PWD/math/csgfractals.h \
    $$PWD/gl/neurogl.h \
    $$PWD/tool/evolutionbase.h \
    $$PWD/tool/evolutionpool.h \
    $$PWD/model/evolutionmimedata.h \
    $$PWD/math/kalisetevolution.h \
    $$PWD/tool/brainf.h \
    $$PWD/tool/brainfevolution.h \
    $$PWD/tool/generalimage.h \
    $$PWD/python/34/python.h \
    $$PWD/python/34/python_geometry.h \
    $$PWD/python/34/python_funcs.h \
    $$PWD/geom/geometrymodifierpython34.h \
    $$PWD/python/34/python_output.h \
    $$PWD/python/34/python_object.h \
    $$PWD/python/34/python_vector.h \
    $$PWD/python/34/py_utils.h \
    $$PWD/python/34/python_timeline.h \
    $$PWD/math/arithmeticarray.h \
    $$PWD/math/timelinend.h \
    $$PWD/model/python34model.h \
    $$PWD/python/34/py_tree.h \
    $$PWD/math/vec_math.h \
    $$PWD/io/log_texture.h \
    $$PWD/io/log_param.h \
    $$PWD/io/log_gui.h \
    $$PWD/io/log_gl.h \
    $$PWD/io/log_io.h \
    $$PWD/io/log_audio.h \
    $$PWD/io/log_mod.h \
    $$PWD/io/log_tree.h \
    $$PWD/io/log_midi.h \
    $$PWD/io/log_geom.h \
    $$PWD/io/log_snd.h


SOURCES += \
    src/math/timeline1d.cpp \
    src/io/console.cpp \
    src/audio/audiosource.cpp \
    src/io/xmlstream.cpp \
    src/io/datastream.cpp \
    src/model/qobjecttreemodel.cpp \
    src/tool/stringmanip.cpp \
    src/io/application.cpp \
    src/model/objecttreemimedata.cpp \
    src/gl/window.cpp \
    src/gl/context.cpp \
    src/io/init.cpp \
    src/types/float.cpp \
    src/io/applicationtime.cpp \
    src/math/noiseperlin.cpp \
    src/math/funcparser/parser.cpp \
    src/tool/syntaxhighlighter.cpp \
    src/gl/manager.cpp \
    src/tool/actionlist.cpp \
    src/tool/enumnames.cpp \
    src/io/memory.cpp \
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
    src/io/currentthread.cpp \
    src/gl/lightsettings.cpp \
    src/audio/tool/envelopefollower.cpp \
    src/audio/tool/multifilter.cpp \
    src/audio/tool/waveform.cpp \
    src/audio/tool/wavetablegenerator.cpp \
    src/io/lockedoutput.cpp \
    src/audio/tool/chebychevfilter.cpp \
    src/gl/rendersettings.cpp \
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
    src/geom/geometrymodifiertexcoords.cpp \
    src/geom/objexporter.cpp \
    src/audio/tool/soundfile.cpp \
    src/audio/tool/soundfilemanager.cpp \
    src/math/intersection.cpp \
    src/projection/domesettings.cpp \
    src/projection/projectorsettings.cpp \
    src/projection/projectormapper.cpp \
    src/projection/projectionsystemsettings.cpp \
    src/projection/camerasettings.cpp \
    src/io/povrayexporter.cpp \
    src/gl/scenedebugrenderer.cpp \
    src/gl/compatibility.cpp \
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
    src/audio/tool/synth.cpp \
    src/audio/tool/filter24.cpp \
    src/math/denormals.cpp \
    src/audio/tool/butterworthfilter.cpp \
    src/audio/tool/fixedfilter.cpp \
    src/geom/geometrymodifierduplicate.cpp \
    src/math/polygon.cpp \
    src/projection/projectormapper_gl.cpp \
    src/audio/tool/bandlimitwavetablegenerator.cpp \
    src/math/fft.cpp \
    src/projection/testprojectionrenderer.cpp \
    src/io/commandlineparser.cpp \
    src/io/version.cpp \
    src/projection/projectorblender.cpp \
    src/video/videostreamreader.cpp \
    src/tool/commonresolutions.cpp \
    src/gl/scenerenderer.cpp \
    src/io/currenttime.cpp \
    src/network/clientstate.cpp \
    src/io/qtextstreamoperators.cpp \
    src/io/helpsystem.cpp \
    src/io/helpexporterhtml.cpp \
    src/io/helpexporterlatex.cpp \
    src/io/isclient.cpp \
    src/main.cpp \
    src/engine/serverengine.cpp \
    src/audio/tool/audiobuffer.cpp \
    src/engine/liveaudioengine.cpp \
    src/engine/audioengine.cpp \
    src/audio/spatial/spatialsoundsource.cpp \
    src/audio/spatial/spatialmicrophone.cpp \
    src/model/objectmimedata.cpp \
    src/audio/tool/dumbfile.cpp \
    src/gl/bufferobject.cpp \
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
    src/geom/pointcloud.cpp \
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
    src/tool/threadpool.cpp \
    src/audio/spatial/wavetracershader.cpp \
    src/audio/tool/irmap.cpp \
    src/audio/audioplayer.cpp \
    src/audio/audioplayerdata.cpp \
    src/math/convolution.cpp \
    src/audio/tool/convolvebuffer.cpp \
    src/geom/geometrymodifierenum.cpp \
    src/io/ladspaloader.cpp \
    src/audio/tool/ladspaplugin.cpp \
    src/gl/texturerenderer.cpp \
    $$PWD/io/sswproject.cpp \
    $$PWD/model/jsontreemodel.cpp \
    $$PWD/geom/shploader.cpp \
    $$PWD/audio/tool/soundfileistream.cpp \
    $$PWD/io/imagereader.cpp \
    $$PWD/tool/dfdownsampler.cpp \
    $$PWD/geom/tesselator_glu.cpp \
    $$PWD/geom/tesselator.cpp \
    $$PWD/geom/geometrymodifiertext.cpp \
    $$PWD/geom/textmesh.cpp \
    $$PWD/tool/linearizerfloat.cpp \
    $$PWD/network/oscinput.cpp \
    $$PWD/network/oscinputs.cpp \
    $$PWD/io/mousestate.cpp \
    $$PWD/io/keyboardstate.cpp \
    $$PWD/math/csgbase.cpp \
    $$PWD/math/csgprimitives.cpp \
    $$PWD/math/csgcombine.cpp \
    $$PWD/model/csgtreemodel.cpp \
    $$PWD/gl/csgshader.cpp \
    $$PWD/math/csgdeform.cpp \
    $$PWD/math/csgfractals.cpp \
    $$PWD/gl/neurogl.cpp \
    $$PWD/tool/evolutionbase.cpp \
    $$PWD/tool/evolutionpool.cpp \
    $$PWD/model/evolutionmimedata.cpp \
    $$PWD/math/kalisetevolution.cpp \
    $$PWD/tool/brainfevolution.cpp \
    $$PWD/tool/generalimage.cpp \
    $$PWD/python/34/python.cpp \
    $$PWD/python/34/python_geometry.cpp \
    $$PWD/python/34/python_funcs.cpp \
    $$PWD/geom/geometrymodifierpython34.cpp \
    $$PWD/python/34/python_output.cpp \
    $$PWD/python/34/python_object.cpp \
    $$PWD/python/34/python_vector.cpp \
    $$PWD/python/34/py_utils.cpp \
    $$PWD/python/34/python_timeline.cpp \
    $$PWD/math/timelinend.cpp \
    $$PWD/model/python34model.cpp \
    $$PWD/python/34/py_tree.cpp \
    $$PWD/types/refcounted.cpp

