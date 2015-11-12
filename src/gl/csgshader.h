/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#ifndef MOSRC_GL_CSGSHADER_H
#define MOSRC_GL_CSGSHADER_H

#include <QSize>

#include "types/vector.h"
#include "gl/shadersource.h"

namespace MO {
class Properties;
class CsgRoot;
namespace GL {


/** Shader generator from a CsgRoot object */
class CsgShader
{
public:
    CsgShader();
    ~CsgShader();

    // -------- getter --------

    const MO::Properties& properties() const;

    GL::ShaderSource getShaderSource() const;

    // -------- setter --------

    void setProperties(const MO::Properties&);
    void setRootObject(const CsgRoot*);

    // ------- opengl ---------

    void createGl();
    void releaseGl();

    void render(const QSize& resolution, const Mat4& projection, const Mat4& transform);

private:
    struct Private;
    Private * p_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_CSGSHADER_H
