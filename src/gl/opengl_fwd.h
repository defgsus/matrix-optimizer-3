/** @file opengl_fwd.h

    @brief Forward declarations for opengl related classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_GL_OPENGL_FWD_H
#define MOSRC_GL_OPENGL_FWD_H


namespace MO {

    class Image;

namespace GL {

    // some forwards
    class BufferObject;
    class Context;
    class Drawable;
    class FrameBufferObject;
    class Manager;
    class SceneDebugRenderer;
    class SceneRenderer;
    class ScreenQuad;
    class Shader;
    class ShaderSource;
    class Texture;
    class TextureRenderer;
    class Uniform;
    class VertexArrayObject;
    class Window;
    class Properties;

    class CameraSpace;
    class LightSettings;
    class RenderSettings;

} // namespace GL
    namespace GEOM {

        class Geometry;
        class GeometryCreator;
        class GeometryFactory;
        class GeometryFactorySettings;

        class GeometryModifier;
        class GeometryModifierChain;

        class ObjLoader;

        class FreeCamera;
        class FreeFloatCamera;

    } // namespace GEOM
} // namespace MO

#endif // MOSRC_GL_OPENGL_FWD_H
