/** @file screenquad.h

    @brief Screen-sized texture quad painter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/1/2014</p>
*/

#ifndef MOSRC_GL_SCREENQUAD_H
#define MOSRC_GL_SCREENQUAD_H

#include <QString>

#include "gl/opengl_fwd.h"
#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace GL {


class ScreenQuad
{
public:
    ScreenQuad(const QString& name, ErrorReporting reporting = ER_THROW);

    /** 0 or 1 to switch off, 2-n for number of samples in x and y direction.
        @note Must be called before creation! */
    void setAntialiasing(uint samples);

    bool create(const QString& defines = QString())
        { return create(":/shader/framebufferdraw.vert", ":/shader/framebufferdraw.frag",
                        defines); }

    /** Creates the opengl resources.
        If @p geom != 0, it will be used as the quad geometry and is
        expected to be in the range of [-1,1], lying on the z-plane.
        Ownership is taken. */
    bool create(const QString& vertexFile,
                const QString& fragmentFile,
                const QString& defines = QString(),
                GEOM::Geometry * geom = 0);

    void release();

    /** Draws a quadratic quad into the view area given by @p w and @p h.
        The quad will be centered correctly, if @p w != @p h */
    bool draw(uint w, uint h);

    /** Draws a quad scaled to the size 1.0 x @p aspect into the
        view area given by @p w and @p h.
        The quad will be centered correctly, if @p w != @p h */
    bool draw(uint screen_w, uint screen_h, Float aspect);

    // ----------- getter -----------

    /** Returns the Drawable, or NULL if not created */
    Drawable * drawable() const { return quad_; }
    /** Returns the Shader for the Drawable, or NULL if not created. */
    Shader * shader() const;

private:
    QString name_;
    ErrorReporting rep_;
    Drawable * quad_;
    uint antialias_;
    Uniform * u_resolution_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SCREENQUAD_H
