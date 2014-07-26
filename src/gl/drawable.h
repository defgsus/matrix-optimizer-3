/** @file drawable.h

    @brief An object that can be painted in OpenGL

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

#ifndef MOSRC_GL_DRAWABLE_H
#define MOSRC_GL_DRAWABLE_H

#include "openglfunctions.h"

class QOpenGLBuffer;

namespace MO {
namespace GL {

class Geometry;
class Context;

class Drawable
{
public:
    Drawable(MO_QOPENGL_FUNCTIONS_CLASS * functions = 0);
    ~Drawable();

    // ------------- getter ------------------

    /** Returns access to the geometry class of the Drawable. */
    Geometry * geometry();

    // ------------- setter ------------------

    /** Sets the geometry to draw.
        The ownership of the Geometry class is taken and
        the previous class is deleted. */
    void setGeometry(Geometry * g);

    // ------------ opengl -------------------

    /** Creates the buffer objects.
        @note This must be called for current opengl context! */
    void createOpenGl();

    /** Releases all opengl resources.
        @note Must be called for the same current opengl context as createOpenGl() */
    void releaseOpenGl();

    void render();
    void renderImmidiate();
private:

    MO_QOPENGL_FUNCTIONS_CLASS * gl_;
    Geometry * geometry_;

    QOpenGLBuffer
        * vertexBuffer_,
        * triIndexBuffer_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_DRAWABLE_H
