/** @file testprojectionrenderer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.10.2014</p>
*/

#ifndef TESTPROJECTIONRENDERER_H
#define TESTPROJECTIONRENDERER_H

#include "projectionsystemsettings.h"
#include "gl/opengl_fwd.h"

namespace MO {

/** Calculator for ProjectorSettings */
class TestProjectionRenderer
{
public:

    TestProjectionRenderer();
    ~TestProjectionRenderer();

    // ---------- getter ----------

    const ProjectionSystemSettings & settings() const;

    // -------- setter ------------

    void setSettings(const ProjectionSystemSettings&);

    // -------- openGL ------------

    void releaseGl();

    void renderSlice(uint index);

    GL::Texture * renderSliceTexture(uint index);

    //______________ PRIVATE AREA _________________
private:

    class Private;
    Private * p_;

};

} // namespace MO

#endif // TESTPROJECTIONRENDERER_H
