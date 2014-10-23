/** @file manager.h

    @brief OpenGL window & assets manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_GL_MANAGER_H
#define MOSRC_GL_MANAGER_H

#include <QObject>
#include "types/vector.h"

namespace MO {
class Scene;
namespace GL {

class Window;
class Context;
class SceneRenderer;

class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject *parent = 0);
    ~Manager();

    // ---------------- opengl ---------------------

    /** Creates an OpenGL Window.
        The window is initally hidden.
        When the window is shown, it's Context will be created
        and Manager::contextCreated() will be emitted. */
    Window * createGlWindow(uint thread);

    void setScene(Scene *);

signals:

    /* This will signal the creation of a new Context */
    //void contextCreated(uint thread, MO::GL::Context *);

    /** Context is current, please render. */
    void renderRequest(uint thread);

    void cameraMatrixChanged(const MO::Mat4&);

    void outputSizeChanged(const QSize&);

public slots:

private slots:

    void onCameraMatrixChanged_(const MO::Mat4&);

private:

    Scene * scene_;
    Window * window_;
    SceneRenderer * renderer_;

};

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_MANAGER_H
