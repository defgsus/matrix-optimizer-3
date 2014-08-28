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

    bool create(const QString& defines = QString())
        { return create(":/shader/framebufferdraw.vert", ":/shader/framebufferdraw.frag",
                        defines); }

    bool create(const QString& vertexFile,
                const QString& fragmentFile,
                const QString& defines = QString());

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
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SCREENQUAD_H
