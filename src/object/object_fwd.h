/** @file object_fwd.h

    @brief forward definitions of common object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/7/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT_FWD_H
#define MOSRC_OBJECT_OBJECT_FWD_H

namespace MO {

    namespace IO { class DataStream; }

    class Object;
    class Scene;
    class Camera;
    class Microphone;
    class SoundSource;
    class Parameter;
    class ParameterFloat;
    class ParameterSelect;
    class ObjectGl;
    class Model3d;
    class Transformation;
    class Rotation;
    class Scale;
    class Shear;
    class Look;
    class LookAt;
    class Mix;
    class Track;
    class TrackFloat;
    class Sequences;
    class Sequence;
    class SequenceFloat;

    class Setting;
    class SettingFloat;

    class ObjectTreeModel;
    class ObjectTreeMimeData;

} // namespace MO

#endif // OBJECT_FWD_H
