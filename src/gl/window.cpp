/** @file window.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include "opengl.h"

#include <QDebug>
#include <QApplication>
#include <QShowEvent>
#include <QOpenGLContext>
#include <QKeyEvent>
#include <QTime>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>

#include "gl/opengl_undef.h"

#include "window.h"
#include "context.h"
#include "io/error.h"
#include "io/log.h"
#include "geom/freecamera.h"
#include "gl/scenerenderer.h"
#include "io/application.h"

namespace MO {
namespace GL {

Window::Window()
    : QWindow       (),
      renderer_     (0),
      thread_       (0),
      updatePending_(false),
      animating_    (false),
      messure_      (new QTime()),
      fps_          (0.0),
      isFreeCamera_ (true),
      cameraControl_(new GEOM::FreeCamera())
{
    MO_DEBUG_GL("Window::Window()");

    setObjectName("_GlWindow");
    setTitle(tr("OpenGL"));

    setWidth(512);
    setHeight(512);

    setSurfaceType(QSurface::OpenGLSurface);
    setFormat(SceneRenderer::defaultFormat());
/*
    updateTimer_ = new QTimer(this);
    updateTimer_->setSingleShot(true);
    updateTimer_->setInterval(1);
    connect(updateTimer_, &QTimer::timeout, [=]()
    {
        //MO_DEBUG("helo");
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest), Qt::HighEventPriority);
    });
    */
}

Window::~Window()
{
    MO_DEBUG_GL("Window::~Window()");

    delete messure_;
    delete cameraControl_;

    delete renderer_;

//    if (context_)
//        context_->doneCurrent();
}

QSize Window::frameSize() const
{
    return size() * devicePixelRatio();
}

void Window::setScreen(uint screenIndex)
{
    // XXX workaround because setScreen() is not very reliable right now
    // ( https://bugreports.qt-project.org/browse/QTBUG-33138 )
    setGeometry(application()->screenGeometry(screenIndex));
}

void Window::setRenderer(SceneRenderer *renderer)
{
    renderer_ = renderer;
}

void Window::exposeEvent(QExposeEvent *)
{
    if (isExposed())
        renderNow();
}

bool Window::event(QEvent * e)
{
    if (e->type() == QEvent::UpdateRequest)
    {
        updatePending_ = false;
        renderNow();
        return true;
    }

    return QWindow::event(e);
}

void Window::resizeEvent(QResizeEvent *)
{
    if (renderer_)
        renderer_->setSize(size() * devicePixelRatio());

    emit sizeChanged(size() * devicePixelRatio());
}

void Window::keyPressEvent(QKeyEvent * e)
{
    e->ignore();

    if ((e->modifiers() == Qt::ALT && e->key() == Qt::Key_F)
    || (e->key() == Qt::Key_F11))
    {
        setWindowState( (Qt::WindowState)(
            windowState() ^ Qt::WindowFullScreen));
        e->accept();
    }

    if (e->key() == Qt::Key_Escape)
    {
        if (windowState() & Qt::WindowFullScreen)
            setWindowState( (Qt::WindowState)(
                windowState() ^ Qt::WindowFullScreen));
        e->accept();
    }

    if (!e->isAccepted())
        emit keyPressed(e);
}

void Window::renderLater()
{
    //MO_DEBUG_GL("Window::renderLater() pending==" << updatePending_);

    if (!updatePending_)
    {
        updatePending_ = true;
        //updateTimer_->start();
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest), Qt::HighEventPriority);
    }
}

/*
void Window::createRenderer_()
{
    MO_DEBUG_GL("creating renderer in window");
    renderer_ = new GL::SceneRenderer();
    renderer_->setSize(size() * devicePixelRatio());

    renderer_->createContext(this);

    //emit contextCreated(thread_, context_);
}
*/

void Window::renderNow()
{
    //MO_DEBUG("Window::renderNow()");

    if (!isExposed() || !renderer_)
        return;

    if (!renderer_->context())
        renderer_->createContext(this);

    // constantly update size XXX
    renderer_->setSize(size() * devicePixelRatio());

    renderer_->render();

    // messure time
    if (animating_)
    {
        int e = messure_->elapsed();
        messure_->start();

        double fps = 1000.0 / std::max(1, e);
        fps_ += 0.1 * (fps - fps_);

        // XXX takes too much resources
        // update should come less frequently
        //setTitle(tr("OpenGl %1 fps").arg((int)(fps_+0.5)));
    }
        //else setTitle(tr("OpenGL"));

    // call again :)
    if (animating_)
        renderLater();
}





void Window::mousePressEvent(QMouseEvent * e)
{
    lastMousePos_ = e->pos();
}

void Window::mouseMoveEvent(QMouseEvent * e)
{
    if (!isFreeCamera_)
        return;

    Float fac = e->modifiers() & Qt::SHIFT ?
                10.f : e->modifiers() & Qt::CTRL? 0.025f : 1.f;

    int dx = lastMousePos_.x() - e->x(),
        dy = lastMousePos_.y() - e->y();
    lastMousePos_ = e->pos();

    bool send = false;

    if (e->buttons() & Qt::LeftButton)
    {
        cameraControl_->moveX(-0.03*fac*dx);
        cameraControl_->moveY( 0.03*fac*dy);

        send = true;
    }

    if (e->buttons() & Qt::RightButton)
    {
        cameraControl_->rotateX(-dy * fac);
        cameraControl_->rotateY(-dx * fac);

        send = true;
    }

    if (send)
    {
        emit cameraMatrixChanged(cameraControl_->getMatrix());
        e->accept();
    }
}

void Window::mouseReleaseEvent(QMouseEvent *)
{

}


void Window::wheelEvent(QWheelEvent * e)
{
    Float fac = e->modifiers() & Qt::SHIFT ?
                10.f : e->modifiers() & Qt::CTRL? 0.025f : 1.f;

    Float d = std::max(-1, std::min(1, e->delta() ));
    cameraControl_->moveZ(-0.3 * d * fac);

    emit cameraMatrixChanged(cameraControl_->getMatrix());

    e->accept();
}

} // namespace GL
} // namespace MO
