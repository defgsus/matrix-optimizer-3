/** @file scenerenderer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.10.2014</p>
*/

#ifndef MOSRC_GL_SCENERENDERER_H
#define MOSRC_GL_SCENERENDERER_H

#include <QObject>
#include <QSize>
#include <QSurfaceFormat>

#include "types/float.h"

class QSurface;

namespace MO {
class Scene;
namespace GL {

class Context;

class SceneRenderer : public QObject
{
    Q_OBJECT
public:
    explicit SceneRenderer(QObject *parent = 0);
    ~SceneRenderer();

    static QSurfaceFormat defaultFormat();

    Scene * scene() const { return scene_; }
    Context * context() const { return context_; }
    QSurface * surface() const { return surface_; }

    void setScene(Scene * scene);

    void setSize(const QSize& resolution);

    void createContext(QSurface *surface);

    void render(Double time);

signals:

    void contextCreated();

public slots:

private:

    void updateSceneGlContext_();

    Scene * scene_;
    Context * context_;
    QSurface * surface_;

    QSize size_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SCENERENDERER_H
