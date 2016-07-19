HEADERS += \
    src/object/object.h \
    src/object/group.h \
    src/object/scene.h \
    src/object/dummy.h \
    src/object/audioobject.h \
    src/object/soundsource.h \
    src/object/microphone.h \
    src/object/textobject.h \
    src/object/visual/objectgl.h \
    src/object/visual/lightsource.h \
    src/object/visual/model3d.h \
    src/object/visual/camera.h \
    src/object/visual/shaderobject.h \
    src/object/texture/textureobjectbase.h \
    src/object/texture/imageto.h \
    src/object/control/modulatorobject.h \
    src/object/control/modulatorobjectfloat.h \
    src/object/control/sequencefloat.h \
    src/object/control/sequence.h \
    src/object/control/track.h \
    src/object/control/trackfloat.h \
    src/object/control/clip.h \
    src/object/control/clipcontroller.h \
    src/object/transform/transformation.h \
    src/object/transform/axisrotation.h \
    src/object/transform/translation.h \
    src/object/transform/scale.h \
    src/object/transform/look.h \
    src/object/transform/lookat.h \
    src/object/transform/shear.h \
    src/object/transform/mix.h \
    src/object/audio/audioinao.h \
    src/object/audio/audiooutao.h \
    src/object/util/texturesetting.h \
    src/object/util/colorpostprocessingsetting.h \
    src/object/util/synthsetting.h \
    src/object/util/objectfilter.h \
    src/object/util/objectmodulatorgraph.h \
    src/object/util/alphablendsetting.h \
    src/object/util/texturemorphsetting.h \
    src/object/util/objecteditor.h \
    src/object/util/audioobjectconnections.h \
    src/object/util/objectdsppath.h \
    src/object/util/useruniformsetting.h \
    src/object/util/objectglpath.h \
    src/object/util/objectconnectiongraph.h \
    src/object/util/parameterevolution.h \
    src/object/util/scenelock_p.h \
    src/object/util/objectfactory.h \
    src/object/param/parameter.h \
    src/object/param/parameterfloat.h \
    src/object/param/parameterselect.h \
    src/object/param/modulator.h \
    src/object/param/modulatorfloat.h \
    src/object/param/parameterint.h \
    src/object/param/parameterfilename.h \
    src/object/param/parametertext.h \
    src/object/param/parametertimeline1d.h \
    src/object/param/parameters.h \
    src/object/param/modulatorevent.h \
    src/object/param/parametercallback.h \
    src/object/param/modulatortexture.h \
    src/object/param/parametertexture.h \
    src/object/param/parametertransformation.h \
    src/object/param/modulatortransformation.h \
    src/object/param/parameterimagelist.h \
    src/object/param/parametergeometry.h \
    src/object/param/modulatorgeometry.h \
    src/object/param/parameterfont.h \
    src/object/interface/valuefloatinterface.h \
    src/object/interface/valuetextureinterface.h \
    src/object/interface/valuetextinterface.h \
    src/object/interface/valuetransformationinterface.h \
    src/object/interface/valuegeometryinterface.h \
    src/object/interface/masteroutinterface.h \
    src/object/interface/geometryeditinterface.h \
    src/object/interface/evolutioneditinterface.h \
    src/object/interface/valueshadersourceinterface.h \
    src/object/object_fwd.h \
    src/object/util/scenesignals.h \
    src/object/microphonegroup.h \
    src/object/synthesizer.h \
    src/object/audio/oscillatorao.h \
    src/object/audio/filterao.h \
    src/object/audio/mverbao.h \
    src/object/audio/fftao.h \
    src/object/audio/shaperao.h \
    src/object/audio/filterbankao.h \
    src/object/audio/envelopefollowerao.h \
    src/object/audio/delayao.h \
    src/object/audio/soundsourceao.h \
    src/object/audio/parameterao.h \
    src/object/audio/impulseao.h \
    src/object/audio/panao.h \
    src/object/audio/dustao.h \
    src/object/audio/bandfilterbankao.h \
    src/object/audio/modplayerao.h \
    src/object/ascriptobject.h \
    src/object/control/derivativeobjectfloat.h \
    src/object/audio/sampleholdao.h \
    src/object/audio/phasorao.h \
    src/object/audio/noiseao.h \
    src/object/audio/playbufferao.h \
    src/object/audio/convolveao.h \
    src/object/audio/pluginao.h \
    src/object/audio/waveplayerao.h \
    src/object/texture/colorto.h \
    src/object/texture/blurto.h \
    src/object/texture/keyto.h \
    src/object/texture/mixto.h \
    src/object/transform/transformationinput.h \
    src/object/transform/mirrortrans.h \
    src/object/transform/cleartrans.h \
    src/object/texture/randomto.h \
    src/object/texture/normalmapto.h \
    src/object/texture/lensdistto.h \
    src/object/audio/microphoneao.h \
    src/object/texture/generate3dto.h \
    src/object/texture/distancemapto.h \
    src/object/texture/thresholdto.h \
    src/object/texture/imagesto.h \
    src/object/visual/geometryobject.h \
    src/object/visual/sprite.h \
    src/object/visual/textureoverlay.h \
    src/object/visual/oscillograph.h \
    src/object/visual/imagegallery.h \
    src/object/texture/shaderto.h \
    src/object/texture/posterizeto.h \
    src/object/texture/kalisetto.h \
    src/object/control/oscinputobject.h \
    src/object/control/keyboardobject.h \
    src/object/transform/freefloattransform.h \
    src/object/texture/textto.h \
    src/object/control/mouseobject.h \
    src/object/texture/neuroto.h \
    src/object/texture/cropto.h \
    src/object/visual/skybox.h \
    $$PWD/util/objecttreesearch.h \
    $$PWD/texture/cubemapto.h \
    $$PWD/pythonobject.h \
    $$PWD/audio/synthao.h \
    $$PWD/audio/shaderao.h \
    $$PWD/control/counterco.h \
    $$PWD/audio/counterao.h \
    $$PWD/texture/videoto.h \
    $$PWD/texture/floattotexture.h \
    $$PWD/audio/fixeddelayao.h \
    $$PWD/param/floatmatrix.h \
    $$PWD/param/modulatorfloatmatrix.h \
    $$PWD/interface/valuefloatmatrixinterface.h \
    $$PWD/param/parameterfloatmatrix.h \
    $$PWD/texture/floatmatrixto.h \
    $$PWD/audio/floatmatrixao.h \
    $$PWD/audio/beatdetectorao.h \
    $$PWD/audio/fftfilterao.h

SOURCES += \
    src/object/object.cpp \
    src/object/scene.cpp \
    src/object/group.cpp \
    src/object/dummy.cpp \
    src/object/audioobject.cpp \
    src/object/soundsource.cpp \
    src/object/microphone.cpp \
    src/object/textobject.cpp \
    src/object/visual/objectgl.cpp \
    src/object/visual/lightsource.cpp \
    src/object/visual/camera.cpp \
    src/object/visual/model3d.cpp \
    src/object/visual/shaderobject.cpp \
    src/object/texture/textureobjectbase.cpp \
    src/object/texture/imageto.cpp \
    src/object/control/modulatorobject.cpp \
    src/object/control/modulatorobjectfloat.cpp \
    src/object/control/sequencefloat.cpp \
    src/object/control/sequence.cpp \
    src/object/control/track.cpp \
    src/object/control/trackfloat.cpp \
    src/object/control/clip.cpp \
    src/object/control/clipcontroller.cpp \
    src/object/transform/transformation.cpp \
    src/object/transform/axisrotation.cpp \
    src/object/transform/translation.cpp \
    src/object/transform/scale.cpp \
    src/object/transform/look.cpp \
    src/object/transform/lookat.cpp \
    src/object/transform/shear.cpp \
    src/object/transform/mix.cpp \
    src/object/audio/audioinao.cpp \
    src/object/audio/audiooutao.cpp \
    src/object/util/texturesetting.cpp \
    src/object/util/colorpostprocessingsetting.cpp \
    src/object/util/synthsetting.cpp \
    src/object/util/objectfilter.cpp \
    src/object/util/objectmodulatorgraph.cpp \
    src/object/util/objecttree.cpp \
    src/object/util/alphablendsetting.cpp \
    src/object/util/texturemorphsetting.cpp \
    src/object/util/objecteditor.cpp \
    src/object/util/audioobjectconnections.cpp \
    src/object/util/objectdsppath.cpp \
    src/object/util/useruniformsetting.cpp \
    src/object/util/objectglpath.cpp \
    src/object/util/objectconnectiongraph.cpp \
    src/object/util/parameterevolution.cpp \
    src/object/util/objectfactory.cpp \
    src/object/param/parameter.cpp \
    src/object/param/parameterfloat.cpp \
    src/object/param/parameterselect.cpp \
    src/object/param/modulator.cpp \
    src/object/param/modulatorfloat.cpp \
    src/object/param/parameterint.cpp \
    src/object/param/parameterfilename.cpp \
    src/object/param/parametertext.cpp \
    src/object/param/parametertimeline1d.cpp \
    src/object/param/parameters.cpp \
    src/object/param/modulatorevent.cpp \
    src/object/param/parametercallback.cpp \
    src/object/param/modulatortexture.cpp \
    src/object/param/parametertexture.cpp \
    src/object/param/parametertransformation.cpp \
    src/object/param/modulatortransformation.cpp \
    src/object/param/parameterimagelist.cpp \
    src/object/param/parametergeometry.cpp \
    src/object/param/modulatorgeometry.cpp \
    src/object/param/parameterfont.cpp \
    src/object/interface/geometryeditinterface.cpp \
    src/object/interface/evolutioneditinterface.cpp \
    src/object/interface/valuefloatinterface.cpp \
    src/object/microphonegroup.cpp \
    src/object/synthesizer.cpp \
    src/object/audio/oscillatorao.cpp \
    src/object/audio/filterao.cpp \
    src/object/audio/mverbao.cpp \
    src/object/audio/fftao.cpp \
    src/object/audio/shaperao.cpp \
    src/object/audio/filterbankao.cpp \
    src/object/audio/envelopefollowerao.cpp \
    src/object/audio/delayao.cpp \
    src/object/audio/soundsourceao.cpp \
    src/object/audio/parameterao.cpp \
    src/object/audio/impulseao.cpp \
    src/object/audio/panao.cpp \
    src/object/audio/dustao.cpp \
    src/object/audio/bandfilterbankao.cpp \
    src/object/audio/modplayerao.cpp \
    src/object/ascriptobject.cpp \
    src/object/control/derivativeobjectfloat.cpp \
    src/object/audio/sampleholdao.cpp \
    src/object/audio/phasorao.cpp \
    src/object/audio/noiseao.cpp \
    src/object/audio/playbufferao.cpp \
    src/object/audio/convolveao.cpp \
    src/object/audio/pluginao.cpp \
    src/object/audio/waveplayerao.cpp \
    src/object/texture/colorto.cpp \
    src/object/texture/blurto.cpp \
    src/object/texture/keyto.cpp \
    src/object/texture/mixto.cpp \
    src/object/transform/transformationinput.cpp \
    src/object/transform/mirrortrans.cpp \
    src/object/transform/cleartrans.cpp \
    src/object/texture/randomto.cpp \
    src/object/texture/normalmapto.cpp \
    src/object/texture/lensdistto.cpp \
    src/object/audio/microphoneao.cpp \
    src/object/texture/generate3dto.cpp \
    src/object/texture/distancemapto.cpp \
    src/object/texture/thresholdto.cpp \
    src/object/texture/imagesto.cpp \
    src/object/visual/geometryobject.cpp \
    src/object/visual/oscillograph.cpp \
    src/object/visual/sprite.cpp \
    src/object/visual/textureoverlay.cpp \
    src/object/visual/imagegallery.cpp \
    src/object/texture/shaderto.cpp \
    src/object/texture/posterizeto.cpp \
    src/object/texture/kalisetto.cpp \
    src/object/control/oscinputobject.cpp \
    src/object/control/keyboardobject.cpp \
    src/object/transform/freefloattransform.cpp \
    src/object/texture/textto.cpp \
    src/object/control/mouseobject.cpp \
    src/object/texture/neuroto.cpp \
    src/object/texture/cropto.cpp \
    src/object/visual/skybox.cpp \
    $$PWD/texture/cubemapto.cpp \
    $$PWD/pythonobject.cpp \
    $$PWD/audio/synthao.cpp \
    $$PWD/audio/shaderao.cpp \
    $$PWD/control/counterco.cpp \
    $$PWD/audio/counterao.cpp \
    $$PWD/texture/videoto.cpp \
    $$PWD/texture/floattotexture.cpp \
    $$PWD/audio/fixeddelayao.cpp \
    $$PWD/param/modulatorfloatmatrix.cpp \
    $$PWD/param/parameterfloatmatrix.cpp \
    $$PWD/texture/floatmatrixto.cpp \
    $$PWD/audio/floatmatrixao.cpp \
    $$PWD/audio/beatdetectorao.cpp \
    $$PWD/audio/fftfilterao.cpp \
    $$PWD/param/floatmatrix.cpp
