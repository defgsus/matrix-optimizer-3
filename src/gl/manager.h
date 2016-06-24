/** @file manager.h

    @brief OpenGL window & assets manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_GL_MANAGER_H
#define MOSRC_GL_MANAGER_H

#include <functional>

#include <QObject>
#include <QKeyEvent>

#include "types/vector.h"
#include "opengl_fwd.h"

namespace MO {
class Scene;
namespace GL {

class SceneRenderer;

class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject *parent = 0);
    ~Manager();

    // ---------------- opengl ---------------------
#if 0
    /** Creates an OpenGL Window.
        The window is initally hidden.
        When the window is shown, it's Context will be created
        and Manager::contextCreated() will be emitted. */
    Window * createGlWindow(uint thread);
#endif

    bool isWindowVisible() const;

    void render();

    void setScene(Scene *);

    void setTimeCallback(std::function<Double()> timeFunc);

    SceneRenderer * renderer() const;

    bool isAnimating() const;

    Double messuredFps() const;
signals:

    /* This will signal the creation of a new Context */
    //void contextCreated(uint thread, MO::GL::Context *);

    void cameraMatrixChanged(const MO::Mat4&);

    void outputSizeChanged(const QSize&);

    void keyPressed(QKeyEvent*);

    void imageFinished(const GL::Texture* tex, const QString& id, const QImage& img);

public slots:

    void setWindowVisible(bool e);

    void startAnimate();
    void stopAnimate();

    /** Will render the texture to a QImage of dimension @p s.
        @p tex MUST be a texture of the Manager's thread/context.
        Emits imageFinished() when ready. */
    void renderImage(const GL::Texture* tex, const QSize& s, const QString& id);

private slots:

    void onCameraMatrixChanged_(const MO::Mat4&);

signals:
    void sendResize_(const QSize&);
    void sendKeyPressed_(QKeyEvent*);
    void sendImage(const GL::Texture* tex, const QString& id, const QImage& img);

private:

    struct Private;
    Private* p_;
};

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_MANAGER_H
