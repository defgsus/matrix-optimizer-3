/** @file scenerenderer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.10.2014</p>
*/

#ifndef MOSRC_GL_SCENERENDERER_H
#define MOSRC_GL_SCENERENDERER_H

#include <functional>

#include <QSize>
#include <QSurfaceFormat>

#include "types/float.h"
#include "types/time.h"

class QSurface;

namespace MO {
class Scene;
namespace GL {

class Context;
class OffscreenContext;
class GlWindow;

class SceneRenderer
{
public:
    explicit SceneRenderer();
    ~SceneRenderer();

    static QSurfaceFormat defaultFormat();

    Scene * scene() const { return scene_; }
    Context * context() const { return context_; }
    QSurface * surface() const { return surface_; }

    /** Speed of last rendering in seconds */
    Double renderSpeed() const { return renderSpeed_; }

    void setScene(Scene * scene);

    void setTimeCallback(std::function<Double()> timeFunc) { timeFuncD_ = timeFunc; }
    void setTimeCallback(std::function<RenderTime()> timeFunc) { timeFuncT_ = timeFunc; }

    void setSize(const QSize& resolution);

    void createContext(QSurface *surface);
    Context* createContext(GlWindow *window);
    OffscreenContext * createOffscreenContext();

    /** @p renderToScreen is a current hack.
        Can be set to false, to just render the Scene::fboFinal() */
    void render(bool renderToScreen);

    //signal void contextCreated();

private:

    void updateSceneGlContext_();

    Scene * scene_;
    Context * context_;
    QSurface * surface_;

    QSize size_;

    std::function<Double()> timeFuncD_;
    std::function<RenderTime()> timeFuncT_;

    Double renderSpeed_, lastTime_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SCENERENDERER_H
