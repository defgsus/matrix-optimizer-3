/** @file opengl_fwd.h

    @brief Forward declarations for opengl related classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_GL_OPENGL_FWD_H
#define MOSRC_GL_OPENGL_FWD_H


namespace MO {
namespace GL {

    enum ErrorReporting
    {
        ER_IGNORE,
        ER_THROW
    };

    // some forwards
    class CameraSpace;
    class Context;
    class Drawable;
    class Geometry;
    class GeometryFactory;
    class GeometryFactorySettings;
    class Manager;
    class Shader;
    class ShaderSource;
    class Uniform;
    class VertexArrayObject;
    class Window;

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_OPENGL_FWD_H
