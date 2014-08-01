/** @file screenquad.h

    @brief Screen-sized texture quad painter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/1/2014</p>
*/

#ifndef MOSRC_GL_SCREENQUAD_H
#define MOSRC_GL_SCREENQUAD_H

#include "gl/opengl_fwd.h"
#include "types/int.h"

namespace MO {
namespace GL {


class ScreenQuad
{
public:
    ScreenQuad(ErrorReporting reporting);

    bool create();
    void release();

    bool draw(uint w, uint h);

private:
    ErrorReporting rep_;
    GL::Drawable * quad_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SCREENQUAD_H
