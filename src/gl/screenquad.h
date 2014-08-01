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

namespace MO {
namespace GL {


class ScreenQuad
{
public:
    ScreenQuad(const QString& name, ErrorReporting reporting);

    bool create(const QString& vertexFile = ":/shader/framebufferdraw.vert",
                const QString& fragmentFile = ":/shader/framebufferdraw.frag");
    void release();

    bool draw(uint w, uint h);

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
