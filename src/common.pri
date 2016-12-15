RESOURCES += \
    images.qrc \
    shaders.qrc \
    models.qrc \
    $$PWD/../templates.qrc

HEADERS += \
    src/types/vector.h \
    src/math/vector.h \
    src/math/constants.h \
    src/math/interpol.h \
    src/types/float.h \
    src/io/console.h \
    src/math/functions.h \
    src/io/log.h \
    src/io/error.h \
    src/io/streamoperators_qt.h \
    src/doc.h \
    src/tool/stringmanip.h \
    src/io/init.h \
    src/math/random.h \
    src/math/funcparser/functions.h \
    src/math/funcparser/parser.h \
    src/math/funcparser/parser_defines.h \
    src/math/funcparser/parser_program.h \
    src/tool/enumnames.h \
    src/io/memory.h \
    src/types/int.h \
    src/gl/opengl.h \
    src/gl/opengl_fwd.h \
    src/math/hash.h \
    src/audio/audio_fwd.h \
    src/io/lockedoutput.h \
    src/io/filetypes.h \
    src/math/intersection.h \
    src/gl/compatibility.h \
    src/network/netlog.h \
    src/io/systeminfo.h \
    src/network/network_fwd.h \
    src/gl/opengl_undef.h \
    src/io/streamoperators_glbinding.h \
    src/tool/deleter.h \
    src/math/denormals.h \
    src/io/version.h \
    src/io/isclient.h \
    src/types/time.h \
    src/audio/3rd/MVerb.h \
    src/types/conversion.h \
    src/script/3rd/angelscript/scriptmath/scriptmathcomplex.h \
    src/script/3rd/angelscript/scriptarray/scriptarray.h \
    src/script/3rd/angelscript/scriptstdstring/scriptstdstring.h \
    src/script/angelscript_vector.h \
    src/script/angelscript.h \
    src/math/advanced.h \
    src/script/angelscript_math.h \
    src/script/angelscript_object.h \
    src/script/angelscript_geometry.h \
    src/script/angelscript_timeline.h \
    src/script/angelscript_network.h \
    src/script/angelscript_image.h \
    src/io/time.h \
    src/maincommandline.h \
    src/audio/3rd/ladspa.h \
    $$PWD/python/34/python.h \
    $$PWD/python/34/python_geometry.h \
    $$PWD/python/34/python_funcs.h \
    $$PWD/python/34/python_output.h \
    $$PWD/python/34/python_object.h \
    $$PWD/python/34/python_vector.h \
    $$PWD/python/34/py_utils.h \
    $$PWD/python/34/python_timeline.h \
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
    $$PWD/io/log_snd.h \
    $$PWD/gl/gl_state.h \
    $$PWD/video/ffm/ffmpeg.h \
    $$PWD/video/ffm/videostream.h \
    $$PWD/gl/win32/winerror.h \
    $$PWD/gl/win32/wm_codes.def \
    $$PWD/gl/win32/wglext.h \
    $$PWD/gl/win32/wm_codes.inl \
    $$PWD/gl/x11/include_x11.h \
    $$PWD/io/architecture.h \
    $$PWD/audio/3rd/KlangFalter/FFTConvolver.h \
    $$PWD/audio/3rd/KlangFalter/Utilities.h \
    $$PWD/audio/spatial/SpatialMicrophone.h \
    $$PWD/audio/spatial/SpatialSoundSource.h \
    $$PWD/audio/spatial/WaveTracerShader.h \
    $$PWD/audio/tool/AudioBuffer.h \
    $$PWD/audio/tool/BandlimitWavetableGenerator.h \
    $$PWD/audio/tool/BeatDetector.h \
    $$PWD/audio/tool/ButterworthFilter.h \
    $$PWD/audio/tool/ChebychevFilter.h \
    $$PWD/audio/tool/ConvolveBuffer.h \
    $$PWD/audio/tool/Delay.h \
    $$PWD/audio/tool/DumbFile.h \
    $$PWD/audio/tool/EnvelopeFollower.h \
    $$PWD/audio/tool/EnvelopeGenerator.h \
    $$PWD/audio/tool/FftWavetableGenerator.h \
    $$PWD/audio/tool/Filter24.h \
    $$PWD/audio/tool/FixedBlockDelay.h \
    $$PWD/audio/tool/FixedFilter.h \
    $$PWD/audio/tool/FloatGate.h \
    $$PWD/audio/tool/IrMap.h \
    $$PWD/audio/tool/LadspaPlugin.h \
    $$PWD/audio/tool/MultiFilter.h \
    $$PWD/audio/tool/NoteFreq.h \
    $$PWD/audio/tool/ResampleBuffer.h \
    $$PWD/audio/tool/SoundFile.h \
    $$PWD/audio/tool/SoundFileIStream.h \
    $$PWD/audio/tool/SoundFileManager.h \
    $$PWD/audio/tool/Synth.h \
    $$PWD/audio/tool/Waveform.h \
    $$PWD/audio/tool/Wavetable.h \
    $$PWD/audio/tool/WavetableGenerator.h \
    $$PWD/audio/AudioDevice.h \
    $$PWD/audio/AudioDevices.h \
    $$PWD/audio/AudioPlayer.h \
    $$PWD/audio/AudioPlayerData.h \
    $$PWD/audio/AudioSource.h \
    $$PWD/audio/Configuration.h \
    $$PWD/audio/MidiDevice.h \
    $$PWD/audio/MidiDevices.h \
    $$PWD/audio/MidiEvent.h \
    $$PWD/engine/AudioEngine.h \
    $$PWD/engine/DiskRenderer.h \
    $$PWD/engine/LiveAudioEngine.h \
    $$PWD/engine/ServerEngine.h \
    $$PWD/geom/BuiltinLineFont.h \
    $$PWD/geom/FreeCamera.h \
    $$PWD/geom/Geometry.h \
    $$PWD/geom/GeometryCreator.h \
    $$PWD/geom/GeometryFactory.h \
    $$PWD/geom/GeometryFactorySettings.h \
    $$PWD/geom/GeometryModifier.h \
    $$PWD/geom/GeometryModifierAngelscript.h \
    $$PWD/geom/GeometryModifierChain.h \
    $$PWD/geom/GeometryModifierConvertlines.h \
    $$PWD/geom/GeometryModifierCreate.h \
    $$PWD/geom/GeometryModifierDuplicate.h \
    $$PWD/geom/GeometryModifierEnum.h \
    $$PWD/geom/GeometryModifierExtrude.h \
    $$PWD/geom/GeometryModifierNormalize.h \
    $$PWD/geom/GeometryModifierNormals.h \
    $$PWD/geom/GeometryModifierPrimitiveEquation.h \
    $$PWD/geom/GeometryModifierPython34.h \
    $$PWD/geom/GeometryModifierRemove.h \
    $$PWD/geom/GeometryModifierRotate.h \
    $$PWD/geom/GeometryModifierScale.h \
    $$PWD/geom/GeometryModifierTesselate.h \
    $$PWD/geom/GeometryModifierTexCoords.h \
    $$PWD/geom/GeometryModifierText.h \
    $$PWD/geom/GeometryModifierTranslate.h \
    $$PWD/geom/GeometryModifierVertexEquation.h \
    $$PWD/geom/GeometryModifierVertexGroup.h \
    $$PWD/geom/MarchingCubes.h \
    $$PWD/geom/ObjExporter.h \
    $$PWD/geom/ObjLoader.h \
    $$PWD/geom/PointCloud.h \
    $$PWD/geom/ShpLoader.h \
    $$PWD/geom/Tesselator.h \
    $$PWD/geom/TextMesh.h \
    $$PWD/gl/win32/GlContext_win32.h \
    $$PWD/gl/win32/GlWindow_win32.h \
    $$PWD/gl/x11/GlContext_x11.h \
    $$PWD/gl/x11/GlWindow_x11.h \
    $$PWD/gl/BufferObject.h \
    $$PWD/gl/CameraSpace.h \
    $$PWD/gl/Context.h \
    $$PWD/gl/CsgShader.h \
    $$PWD/gl/Drawable.h \
    $$PWD/gl/GlContext.h \
    $$PWD/gl/GlContext_private.h \
    $$PWD/gl/GlWindow.h \
    $$PWD/gl/GlWindow_private.h \
    $$PWD/gl/LightSettings.h \
    $$PWD/gl/Manager.h \
    $$PWD/gl/NeuroGl.h \
    $$PWD/gl/OffscreenContext.h \
    $$PWD/gl/RenderSettings.h \
    $$PWD/gl/SceneDebugRenderer.h \
    $$PWD/gl/SceneRenderer.h \
    $$PWD/gl/ScreenQuad.h \
    $$PWD/gl/Shader.h \
    $$PWD/gl/ShaderSource.h \
    $$PWD/gl/Texture.h \
    $$PWD/gl/TextureRenderer.h \
    $$PWD/gl/VertexArrayObject.h \
    $$PWD/gl/Window.h \
    $$PWD/graph/DirectedGraph.h \
    $$PWD/graph/Tree.h \
    $$PWD/io/Application.h \
    $$PWD/io/ApplicationTime.h \
    $$PWD/io/CurrentThread.h \
    $$PWD/io/CurrentTime.h \
    $$PWD/io/DataStream.h \
    $$PWD/io/DiskRenderSettings.h \
    $$PWD/io/DocbookExporter.h \
    $$PWD/io/EquationPreset.h \
    $$PWD/io/EquationPresets.h \
    $$PWD/io/FileManager.h \
    $$PWD/io/Files.h \
    $$PWD/io/HelpExporterHtml.h \
    $$PWD/io/HelpExporterLatex.h \
    $$PWD/io/HelpSystem.h \
    $$PWD/io/ImageReader.h \
    $$PWD/io/JsonInterface.h \
    $$PWD/io/KeyboardState.h \
    $$PWD/io/LadspaLoader.h \
    $$PWD/io/MouseState.h \
    $$PWD/io/PovrayExporter.h \
    $$PWD/io/QTextStreamOperators.h \
    $$PWD/io/Settings.h \
    $$PWD/io/SswProject.h \
    $$PWD/io/XmlStream.h \
    $$PWD/math/ArithmeticArray.h \
    $$PWD/math/Convolution.h \
    $$PWD/math/CsgBase.h \
    $$PWD/math/CsgCombine.h \
    $$PWD/math/CsgDeform.h \
    $$PWD/math/CsgFractals.h \
    $$PWD/math/CsgPrimitives.h \
    $$PWD/math/CubemapMatrix.h \
    $$PWD/math/Fft.h \
    $$PWD/math/OouraFft.h \
    $$PWD/math/FftWindow.h \
    $$PWD/math/InterpolationType.h \
    $$PWD/math/KalisetEvolution.h \
    $$PWD/math/NoisePerlin.h \
    $$PWD/math/Polygon.h \
    $$PWD/math/Timeline1d.h \
    $$PWD/math/TimelinePoint.h \
    $$PWD/math/TimelineNd.h \
    $$PWD/math/TransformationBuffer.h \
    $$PWD/model/CsgTreeModel.h \
    $$PWD/model/EvolutionMimeData.h \
    $$PWD/model/JsonTreeModel.h \
    $$PWD/model/ObjectMimeData.h \
    $$PWD/model/ObjectTreeMimeData.h \
    $$PWD/model/ObjectTreeModel.h \
    $$PWD/model/Python34Model.h \
    $$PWD/model/QObjectTreeModel.h \
    $$PWD/network/ClientState.h \
    $$PWD/network/EventCom.h \
    $$PWD/network/NetEvent.h \
    $$PWD/network/NetworkManager.h \
    $$PWD/network/OscInput.h \
    $$PWD/network/OscInputs.h \
    $$PWD/network/TcpServer.h \
    $$PWD/network/UdpAudioConnection.h \
    $$PWD/network/UdpConnection.h \
    $$PWD/projection/CameraSettings.h \
    $$PWD/projection/DomeSettings.h \
    $$PWD/projection/ProjectionSystemSettings.h \
    $$PWD/projection/ProjectorBlender.h \
    $$PWD/projection/ProjectorMapper.h \
    $$PWD/projection/ProjectorSettings.h \
    $$PWD/projection/TestProjectionRenderer.h \
    $$PWD/tool/ActionList.h \
    $$PWD/tool/AsciiRect.h \
    $$PWD/tool/Brainf.h \
    $$PWD/tool/BrainfEvolution.h \
    $$PWD/tool/CommonResolutions.h \
    $$PWD/tool/DfDownsampler.h \
    $$PWD/tool/EvolutionBase.h \
    $$PWD/tool/EvolutionPool.h \
    $$PWD/tool/GeneralImage.h \
    $$PWD/tool/LinearizerFloat.h \
    $$PWD/tool/LocklessQueue.h \
    $$PWD/tool/Selection.h \
    $$PWD/tool/SyntaxHighlighter.h \
    $$PWD/tool/ThreadPool.h \
    $$PWD/tool/ValueSmoother.h \
    $$PWD/types/Properties.h \
    $$PWD/types/Refcounted.h \
    $$PWD/types/Refcounted_info.h \
    $$PWD/video/DecoderFrame.h \
    $$PWD/video/VideoStreamReader.h \
    $$PWD/gl/FrameBufferObject.h \
    $$PWD/io/CommandLineParser.h \
    $$PWD/math/Fraction.h \
    $$PWD/io/log_FloatMatrix.h \
    $$PWD/tool/ProgressInfo.h \
    $$PWD/math/FloatMatrix.h \
    $$PWD/io/error_index.h \
    $$PWD/io/NeurofoxLoader.h \
    $$PWD/python/34/python_matrix4.h \
    $$PWD/python/34/vector_helper.h \
    $$PWD/python/py_helper.h \
    $$PWD/python/mo3.h \
    $$PWD/python/py_mo_helper.h


SOURCES += \
    src/io/console.cpp \
    src/tool/stringmanip.cpp \
    src/io/init.cpp \
    src/types/float.cpp \
    src/math/funcparser/parser.cpp \
    src/io/memory.cpp \
    src/gl/opengl.cpp \
    src/io/lockedoutput.cpp \
    src/io/filetypes.cpp \
    src/math/intersection.cpp \
    src/gl/compatibility.cpp \
    src/network/netlog.cpp \
    src/io/systeminfo.cpp \
    src/math/denormals.cpp \
    src/io/version.cpp \
    src/io/isclient.cpp \
    src/main.cpp \
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
    src/script/angelscript_timeline.cpp \
    src/script/angelscript_network.cpp \
    src/script/angelscript_image.cpp \
    src/io/time.cpp \
    $$PWD/python/34/python.cpp \
    $$PWD/python/34/python_geometry.cpp \
    $$PWD/python/34/python_funcs.cpp \
    $$PWD/python/34/python_output.cpp \
    $$PWD/python/34/python_object.cpp \
    $$PWD/python/34/python_vector.cpp \
    $$PWD/python/34/py_utils.cpp \
    $$PWD/python/34/python_timeline.cpp \
    $$PWD/python/34/py_tree.cpp \
    $$PWD/gl/gl_state.cpp \
    $$PWD/video/ffm/ffmpeg.cpp \
    src/gl/win32/winerror.cpp \
    $$PWD/audio/3rd/KlangFalter/FFTConvolver.cpp \
    $$PWD/audio/3rd/KlangFalter/Utilities.cpp \
    $$PWD/tool/ActionList.cpp \
    $$PWD/audio/spatial/SpatialMicrophone.cpp \
    $$PWD/audio/spatial/SpatialSoundSource.cpp \
    $$PWD/audio/spatial/WaveTracerShader.cpp \
    $$PWD/audio/tool/AudioBuffer.cpp \
    $$PWD/audio/tool/BandlimitWavetableGenerator.cpp \
    $$PWD/audio/tool/BeatDetector.cpp \
    $$PWD/audio/tool/ButterworthFilter.cpp \
    $$PWD/audio/tool/ChebychevFilter.cpp \
    $$PWD/audio/tool/ConvolveBuffer.cpp \
    $$PWD/audio/tool/DumbFile.cpp \
    $$PWD/audio/tool/EnvelopeFollower.cpp \
    $$PWD/audio/tool/Filter24.cpp \
    $$PWD/audio/tool/FixedFilter.cpp \
    $$PWD/audio/tool/IrMap.cpp \
    $$PWD/audio/tool/LadspaPlugin.cpp \
    $$PWD/audio/tool/MultiFilter.cpp \
    $$PWD/audio/tool/SoundFile.cpp \
    $$PWD/audio/tool/SoundFileIStream.cpp \
    $$PWD/audio/tool/SoundFileManager.cpp \
    $$PWD/audio/tool/Synth.cpp \
    $$PWD/audio/tool/Waveform.cpp \
    $$PWD/audio/tool/WavetableGenerator.cpp \
    $$PWD/audio/AudioDevice.cpp \
    $$PWD/audio/AudioDevices.cpp \
    $$PWD/audio/AudioMicrophone.cpp \
    $$PWD/audio/AudioPlayer.cpp \
    $$PWD/audio/AudioPlayerData.cpp \
    $$PWD/audio/AudioSource.cpp \
    $$PWD/audio/MidiDevice.cpp \
    $$PWD/audio/MidiDevices.cpp \
    $$PWD/audio/MidiEvent.cpp \
    $$PWD/engine/AudioEngine.cpp \
    $$PWD/engine/DiskRenderer.cpp \
    $$PWD/engine/LiveAudioEngine.cpp \
    $$PWD/engine/ServerEngine.cpp \
    $$PWD/geom/BuiltinLineFont.cpp \
    $$PWD/geom/FreeCamera.cpp \
    $$PWD/geom/Geometry.cpp \
    $$PWD/geom/GeometryCreator.cpp \
    $$PWD/geom/GeometryFactory.cpp \
    $$PWD/geom/GeometryFactorySettings.cpp \
    $$PWD/geom/GeometryModifier.cpp \
    $$PWD/geom/GeometryModifierAngelscript.cpp \
    $$PWD/geom/GeometryModifierChain.cpp \
    $$PWD/geom/GeometryModifierConvertLines.cpp \
    $$PWD/geom/GeometryModifierCreate.cpp \
    $$PWD/geom/GeometryModifierDuplicate.cpp \
    $$PWD/geom/GeometryModifierEnum.cpp \
    $$PWD/geom/GeometryModifierExtrude.cpp \
    $$PWD/geom/GeometryModifierNormalize.cpp \
    $$PWD/geom/GeometryModifierNormals.cpp \
    $$PWD/geom/GeometryModifierPrimitiveEquation.cpp \
    $$PWD/geom/GeometryModifierPython34.cpp \
    $$PWD/geom/GeometryModifierRemove.cpp \
    $$PWD/geom/GeometryModifierRotate.cpp \
    $$PWD/geom/GeometryModifierScale.cpp \
    $$PWD/geom/GeometryModifierTesselate.cpp \
    $$PWD/geom/GeometryModifierTexCoords.cpp \
    $$PWD/geom/GeometryModifierText.cpp \
    $$PWD/geom/GeometryModifierTranslate.cpp \
    $$PWD/geom/GeometryModifierVertexEquation.cpp \
    $$PWD/geom/GeometryModifierVertexGroup.cpp \
    $$PWD/geom/MarchingCubes.cpp \
    $$PWD/geom/ObjExporter.cpp \
    $$PWD/geom/ObjLoader.cpp \
    $$PWD/geom/PointCloud.cpp \
    $$PWD/geom/ShpLoader.cpp \
    $$PWD/geom/Tesselator.cpp \
    $$PWD/geom/Tesselator_glu.cpp \
    $$PWD/geom/TextMesh.cpp \
    $$PWD/gl/win32/GlContext_win32.cpp \
    $$PWD/gl/win32/GlWindow_win32.cpp \
    $$PWD/gl/x11/GlContext_x11.cpp \
    $$PWD/gl/x11/GlWindow_x11.cpp \
    $$PWD/gl/BufferObject.cpp \
    $$PWD/gl/Context.cpp \
    $$PWD/gl/CsgShader.cpp \
    $$PWD/gl/Drawable.cpp \
    $$PWD/gl/FrameBufferObject.cpp \
    $$PWD/gl/GlContext_common.cpp \
    $$PWD/gl/GlWindow_common.cpp \
    $$PWD/gl/LightSettings.cpp \
    $$PWD/gl/Manager.cpp \
    $$PWD/gl/NeuroGl.cpp \
    $$PWD/gl/OffscreenContext.cpp \
    $$PWD/gl/RenderSettings.cpp \
    $$PWD/gl/SceneDebugRenderer.cpp \
    $$PWD/gl/SceneRenderer.cpp \
    $$PWD/gl/ScreenQuad.cpp \
    $$PWD/gl/Shader.cpp \
    $$PWD/gl/ShaderSource.cpp \
    $$PWD/gl/Texture.cpp \
    $$PWD/gl/TextureRenderer.cpp \
    $$PWD/gl/VertexArrayObject.cpp \
    $$PWD/gl/Window.cpp \
    $$PWD/io/Application.cpp \
    $$PWD/io/ApplicationTime.cpp \
    $$PWD/io/CommandLineParser.cpp \
    $$PWD/io/CurrentThread.cpp \
    $$PWD/io/CurrentTime.cpp \
    $$PWD/io/DataStream.cpp \
    $$PWD/io/DiskRenderSettings.cpp \
    $$PWD/io/DocbookExporter.cpp \
    $$PWD/io/EquationPreset.cpp \
    $$PWD/io/EquationPresets.cpp \
    $$PWD/io/FileManager.cpp \
    $$PWD/io/Files.cpp \
    $$PWD/io/HelpExporterHtml.cpp \
    $$PWD/io/HelpExporterLatex.cpp \
    $$PWD/io/HelpSystem.cpp \
    $$PWD/io/ImageReader.cpp \
    $$PWD/io/JsonInterface.cpp \
    $$PWD/io/KeyboardState.cpp \
    $$PWD/io/LadspaLoader.cpp \
    $$PWD/io/MouseState.cpp \
    $$PWD/io/PovrayExporter.cpp \
    $$PWD/io/QTextStreamOperators.cpp \
    $$PWD/io/Settings.cpp \
    $$PWD/io/SswProject.cpp \
    $$PWD/io/XmlStream.cpp \
    $$PWD/math/CsgBase.cpp \
    $$PWD/math/CsgCombine.cpp \
    $$PWD/math/CsgDeform.cpp \
    $$PWD/math/CsgFractals.cpp \
    $$PWD/math/CsgPrimitives.cpp \
    $$PWD/math/CubemapMatrix.cpp \
    $$PWD/math/Fft.cpp \
    $$PWD/math/OouraFft.cpp \
    $$PWD/math/KalisetEvolution.cpp \
    $$PWD/math/NoisePerlin.cpp \
    $$PWD/math/Polygon.cpp \
    $$PWD/math/Timeline1d.cpp \
    $$PWD/math/TimelinePoint.cpp \
    $$PWD/math/TimelineNd.cpp \
    $$PWD/model/CsgTreeModel.cpp \
    $$PWD/model/EvolutionMimeData.cpp \
    $$PWD/model/jsonTreeModel.cpp \
    $$PWD/model/ObjectMimeData.cpp \
    $$PWD/model/ObjectTreeMimeData.cpp \
    $$PWD/model/ObjectTreeModel.cpp \
    $$PWD/model/Python34Model.cpp \
    $$PWD/model/QObjectTreeModel.cpp \
    $$PWD/network/ClientState.cpp \
    $$PWD/network/EventCom.cpp \
    $$PWD/network/NetEvent.cpp \
    $$PWD/network/NetworkManager.cpp \
    $$PWD/network/OscInput.cpp \
    $$PWD/network/OscInputs.cpp \
    $$PWD/network/TcpServer.cpp \
    $$PWD/network/UdpAudioConnection.cpp \
    $$PWD/network/UdpConnection.cpp \
    $$PWD/projection/CameraSettings.cpp \
    $$PWD/projection/DomeSettings.cpp \
    $$PWD/projection/ProjectionSystemSettings.cpp \
    $$PWD/projection/ProjectorBlender.cpp \
    $$PWD/projection/ProjectorMapper.cpp \
    $$PWD/projection/ProjectorMapper_gl.cpp \
    $$PWD/projection/ProjectorSettings.cpp \
    $$PWD/projection/TestProjectionRenderer.cpp \
    $$PWD/tool/AsciiRect.cpp \
    $$PWD/tool/BrainfEvolution.cpp \
    $$PWD/tool/CommonResolutions.cpp \
    $$PWD/tool/DfDownsampler.cpp \
    $$PWD/tool/EnumNames.cpp \
    $$PWD/tool/EvolutionBase.cpp \
    $$PWD/tool/EvolutionPool.cpp \
    $$PWD/tool/GeneralImage.cpp \
    $$PWD/tool/LinearizerFloat.cpp \
    $$PWD/tool/SyntaxHighlighter.cpp \
    $$PWD/tool/ThreadPool.cpp \
    $$PWD/types/Properties.cpp \
    $$PWD/types/Refcounted.cpp \
    $$PWD/video/VideoStreamReader.cpp \
    $$PWD/video/ffm/VideoStream.cpp \
    $$PWD/mainCommandLine.cpp \
    $$PWD/math/Fraction.cpp \
    $$PWD/tool/ProgressInfo.cpp \
    $$PWD/math/FloatMatrix.cpp \
    $$PWD/io/NeurofoxLoader.cpp \
    $$PWD/python/34/python_matrix4.cpp \
    $$PWD/python/34/vector_helper.cpp \
    $$PWD/python/py_helper.cpp \
    $$PWD/python/mo3.cpp \
    $$PWD/python/py_mo_helper.cpp

DISTFILES +=

