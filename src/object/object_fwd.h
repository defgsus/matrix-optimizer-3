/** @file object_fwd.h

    @brief forward definitions of common object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/7/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT_FWD_H
#define MOSRC_OBJECT_OBJECT_FWD_H

namespace MO {

    /** Type of text in ParameterText */
    enum TextType
    {
        TT_PLAIN_TEXT,
        TT_EQUATION,
        TT_APP_STYLESHEET,
        TT_ANGELSCRIPT,
        TT_GLSL,
        TT_OBJECT_WILDCARD
    };

    /** General groups of object outputs and/or parameters */
    enum SignalType
    {
        ST_FLOAT,
        ST_INT,
        ST_SELECT,
        ST_TEXT,
        ST_FILENAME,
        ST_TRANSFORMATION,
        ST_TEXTURE,
        ST_TIMELINE1D,
        ST_AUDIO,
        ST_CALLBACK
    };

    namespace IO { class DataStream; }
    namespace GL { class LightSettings; class CameraSpace; }
    namespace MATH { class Timeline1D; }
    namespace AUDIO
    {
        class SpatialSoundSource;
        class SpatialMicrophone;
        class AudioBuffer;
        class Configuration;
    }

    class Object;
    class Scene;
    class Camera;
    class LightSource;
    class ObjectGl;
    class Model3d;
    class Sequences;
    class Sequence;
    class SequenceFloat;
    class ShaderObject;
    class Clip;
    class ClipController;
    class Track;
    class TrackFloat;

    class ValueFloatInterface;
    class ValueTextInterface;
    class ValueTextureInterface;
    class ValueTransformationInterface;

    class Parameter;
    class Parameters;
    class ParameterCallback;
    class ParameterInt;
    class ParameterFilename;
    class ParameterFloat;
    class ParameterSelect;
    class ParameterTimeline1D;
    class ParameterText;
    class ParameterTexture;
    class ParameterTransformation;

    class TransformationBuffer;
    class Transformation;
    class Rotation;
    class Scale;
    class Shear;
    class Look;
    class LookAt;
    class Mix;

    class Modulator;
    class ModulatorFloat;
    class ModulatorOutput;
    class ModulatorTexture;
    class ModulatorTransformation;

    class ModulatorObject;
    class ModulatorObjectFloat;

    class AudioObject;
    class AudioObjectConnection;
    class AudioObjectConnections;

    class ColorPostProcessingSetting;
    class SynthSetting;
    class TextureSetting;

    class ObjectFilter;

    class ObjectEditor;
    class ObjectDspPath;
    class ObjectTreeMimeData;

} // namespace MO

#endif // OBJECT_FWD_H
