/** @file rendersettings.h

    @brief RenderSettings including all infos, passed to objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/12/2014</p>
*/

#ifndef MOSRC_GL_RENDERSETTINGS_H
#define MOSRC_GL_RENDERSETTINGS_H

#include "gl/opengl_fwd.h"
#include "types/int.h"

namespace MO {
namespace GL {

class RenderSettings
{
public:
    RenderSettings();

    // ------------ getter --------------

    const CameraSpace & cameraSpace() const { return *camSpace_; }
    const LightSettings & lightSettings() const { return *lightSettings_; }


    // ----------- setter ---------------

    void setCameraSpace(const CameraSpace * camSpace) { camSpace_ = camSpace; }
    void setLightSettings(const LightSettings * light) { lightSettings_ = light; }

private:

    const CameraSpace * camSpace_;
    const LightSettings * lightSettings_;
};


} // namespace GL
} // namespace MO


#endif // MOSRC_GL_RENDERSETTINGS_H
