/** @file scenelock_p.h

    @brief Scoped locks for Scene (private to Scene)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_SCENELOCK_P_H
#define MOSRC_OBJECT_UTIL_SCENELOCK_P_H

#include "object/scene.h"

namespace MO {


class ScopedSceneLockRead
{
    Scene * scene_;
public:
    ScopedSceneLockRead(Scene * scene) : scene_(scene)
    {
        scene_->lockRead_();
    }

    ~ScopedSceneLockRead() { scene_->unlock_(); }
};

class ScopedSceneLockWrite
{
    Scene * scene_;
public:
    ScopedSceneLockWrite(Scene * scene) : scene_(scene)
    {
        scene_->lockWrite_();
    }

    ~ScopedSceneLockWrite() { scene_->unlock_(); }
};


} // namespace MO

#endif // MOSRC_OBJECT_UTIL_SCENELOCK_P_H
