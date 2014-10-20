/** @file projectorblender.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.10.2014</p>
*/

#ifndef MOSRC_PROJECTION_PROJECTORBLENDER_H
#define MOSRC_PROJECTION_PROJECTORBLENDER_H

#include <gl/opengl_fwd.h>

namespace MO {

class ProjectionSystemSettings;

class ProjectorBlender
{
public:
    ProjectorBlender(const ProjectionSystemSettings * = 0);
    ~ProjectorBlender();

    void setSettings(const ProjectionSystemSettings&);

    GL::Texture * renderBlendTexture(uint index);

private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_PROJECTORBLENDER_H
