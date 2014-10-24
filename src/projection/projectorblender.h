/** @file projectorblender.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.10.2014</p>
*/

#ifndef MOSRC_PROJECTION_PROJECTORBLENDER_H
#define MOSRC_PROJECTION_PROJECTORBLENDER_H

#include <gl/opengl_fwd.h>
#include "types/int.h"

namespace MO {

class ProjectionSystemSettings;

class ProjectorBlender
{
public:
    ProjectorBlender(const ProjectionSystemSettings * = 0);
    ~ProjectorBlender();

    void setSettings(const ProjectionSystemSettings&);

    /** Renders the blend texture for the current settings and projector @p index.
        If @p height == 0, the width and height of the projector's virtual camera will be used,
        otherwise the given height and calculated width (from projector's aspect ratio) is used. */
    GL::Texture * renderBlendTexture(uint index, uint height = 0);

private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_PROJECTORBLENDER_H
