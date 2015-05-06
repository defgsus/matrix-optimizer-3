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
        TT_GLSL
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

    class Parameter;
    class Parameters;
    class ParameterCallback;
    class ParameterInt;
    class ParameterFilename;
    class ParameterFloat;
    class ParameterSelect;
    class ParameterText;
    class ParameterTimeline1D;

    class TransformationBuffer;
    class Transformation;
    class Rotation;
    class Scale;
    class Shear;
    class Look;
    class LookAt;
    class Mix;

    class ModulatorOutput;
    class Modulator;
    class ModulatorFloat;

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
