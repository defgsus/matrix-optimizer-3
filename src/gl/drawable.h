/** @file drawable.h

    @brief An object that can be painted in OpenGL

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

#ifndef MOSRC_GL_DRAWABLE_H
#define MOSRC_GL_DRAWABLE_H

#include "opengl.h"
#include "types/vector.h"

namespace MO {
namespace GL {

class Geometry;
class Context;
class Shader;
class ShaderSource;
class Uniform;

class Drawable
{
public:
    Drawable();
    ~Drawable();

    // ------------- getter ------------------

    /** Returns access to the geometry class of the Drawable. */
    Geometry * geometry();

    /** Returns access to the shader source of the Drawable. */
    ShaderSource * shaderSource();

    /** Returns access to the shader of the Drawable. */
    Shader * shader();

    // ------------- setter ------------------

    /** Sets the geometry to draw.
        The ownership of the Geometry class is taken and
        the previous class is deleted. */
    void setGeometry(Geometry * g);

    /** Sets the shader source.
        The ownership is taken and the previous class is deleted. */
    void setShaderSource(ShaderSource * s);

    /** Sets the shader object.
        The ownership is taken and the previous class is deleted. */
    void setShader(Shader * s);

    // ------------ opengl -------------------

    /** Creates the buffer objects.
        @note This must be called for current opengl context! */
    void createOpenGl();

    /** Releases all opengl resources.
        @note Must be called for the same current opengl context as createOpenGl() */
    void releaseOpenGl();

    void render();
    void renderShader(const Mat4& proj, const Mat4& view);
    void renderArrays();
    void renderAttribArrays();
    void renderImmediate();
    void renderImmediateShader(const Mat4& proj, const Mat4& view);

private:

    void compileShader_();
    void createVAO_();

    Geometry * geometry_;
    ShaderSource * shaderSource_;
    Shader * shader_;

    GLuint
        vao_,
        vertexBuffer_,
        triIndexBuffer_;

    GLuint
        uniformProj_,
        uniformView_,
        attribPos_;

};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_DRAWABLE_H
