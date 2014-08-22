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

#include "window.h"
#include "context.h"
#include "io/error.h"
#include "io/log.h"
#include "geom/freecamera.h"

namespace MO {
namespace GL {

Window::Window(QScreen * targetScreen)
    : QWindow       (targetScreen),
      context_      (0),
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

    /*QSurfaceFormat format;
    format.setSamples(16);
    setFormat(format);*/
}

Window::~Window()
{
    MO_DEBUG_GL("Window::~Window()");

    delete messure_;
    delete cameraControl_;

//    if (context_)
//        context_->doneCurrent();
}


void Window::exposeEvent(QExposeEvent *)
{
    if (isExposed())
        renderNow();
}

bool Window::event(QEvent * e)
{
    switch (e->type())
    {
    case QEvent::UpdateRequest:
        updatePending_ = false;
        renderNow();
        return true;
    break;

    case QEvent::KeyPress:
        if (QKeyEvent * k = dynamic_cast<QKeyEvent*>(e))
        {
            if ((k->modifiers() == Qt::ALT && k->key() == Qt::Key_F)
            || (k->key() == Qt::Key_F11))
            {
                setWindowState( (Qt::WindowState)(
                    windowState() ^ Qt::WindowFullScreen));
            }
        }
    break;
    default: break;
    }
    return QWindow::event(e);
}

void Window::renderLater()
{
    //MO_DEBUG_GL("Window::renderLater()");

    if (!updatePending_)
    {
        updatePending_ = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void Window::renderNow()
{
    //MO_DEBUG_GL("Window::renderNow()");

    if (!isExposed())
        return;

    //bool needsInit = false;

    if (!context_)
    {
        MO_DEBUG_GL("creating context in window");

        context_ = new MO::GL::Context(this);
        context_->qcontext()->setFormat(requestedFormat());
        if (!context_->qcontext()->create())
            MO_GL_ERROR("could not create context");

        emit contextCreated(thread_, context_);

        //needsInit = true;
    }

    if (!context_->qcontext()->makeCurrent(this))
        MO_GL_ERROR("could not make context current");

    context_->setSize(size());

    moInitGl();

    MO_CHECK_GL( glViewport(0,0, width(), height()) );

    emit renderRequest(thread_);


    context_->qcontext()->swapBuffers(this);

    // messure time
    if (animating_)
    {
        int e = messure_->elapsed();
        messure_->start();

        double fps = 1000.0 / std::max(1, e);
        fps_ += 0.1 * (fps - fps_);
        setTitle(tr("OpenGl %1 fps").arg((int)(fps_+0.5)));
    }
        else setTitle(tr("OpenGL"));

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
                10.f : 1.f;

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
                10.f : 1.f;

    Float d = std::max(-1, std::min(1, e->delta() ));
    cameraControl_->moveZ(-0.3 * d * fac);

    emit cameraMatrixChanged(cameraControl_->getMatrix());

    e->accept();
}

} // namespace GL
} // namespace MO
