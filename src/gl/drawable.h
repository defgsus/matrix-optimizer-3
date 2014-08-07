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

class LightSettings;

class Drawable
{
public:
    /** @p name is for debugging purposes mainly */
    Drawable(const QString& name);
    ~Drawable();

    // ------------- getter ------------------

    /** Returns access to the geometry class of the Drawable. */
    GEOM::Geometry * geometry();

    /** Returns access to the shader source of the Drawable. */
    ShaderSource * shaderSource();

    /** Returns access to the shader of the Drawable. */
    Shader * shader();

    /** Returns true when the Drawable is ready to render */
    bool isReady() const;

    // ------------- setter ------------------

    /** Sets the geometry to draw.
        The ownership of the Geometry class is taken and
        the previous class is deleted. */
    void setGeometry(GEOM::Geometry * g);

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

    /** Sets the ambient color (through uniform) */
    void setAmbientColor(Float r, Float g, Float b, Float a = 1.0);

    void render();
    void renderShader();

    void renderShader(const Mat4& proj, const Mat4& viewtrans,
                      const Mat4& trans, const LightSettings& lights)
    { renderShader(proj, viewtrans, &trans, &lights); }

    void renderShader(const Mat4& proj, const Mat4& view, const Mat4 * viewtrans = 0, const LightSettings* = 0);
    void renderImmediate();

private:

    void checkGeometryChanged_();
    void compileShader_();
    void createVAO_();

    QString name_;

    GEOM::Geometry * geometry_;
    ShaderSource * shaderSource_;
    Shader * shader_;

    bool doRecompile_, geometryChanged_;

    VertexArrayObject * vao_;

    GLuint
        uniformProj_,
        uniformView_,
        uniformTransform_,
        uniformLightPos_,
        uniformLightColor_;

    Uniform * uniColor_;

};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_DRAWABLE_H
