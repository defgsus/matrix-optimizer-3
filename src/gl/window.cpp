/** @file window.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include <QDebug>
#include <QApplication>
#include <QShowEvent>
#include <QOpenGLFramebufferObject>
#include <QKeyEvent>

#include "window.h"
#include "context.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GL {

Window::Window(QScreen * targetScreen)
    :   QWindow       (targetScreen),
        context_      (0),
        updatePending_(false),
        animating_    (false)
{
    MO_DEBUG_GL("Window::Window()");

    setObjectName("_GlWindow");
    setTitle("OpenGL");

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
//    if (context_)
//        context_->doneCurrent();
}

/*
void Window::setFramebuffer(QOpenGLFramebufferObject *frameBuffer)
{
    frameBuffer_ = frameBuffer;
}
*/

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
    if (!updatePending_)
    {
        updatePending_ = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void Window::renderNow()
{
    MO_DEBUG_GL("Window::renderNow()");

    if (!isExposed())
        return;

    //bool needsInit = false;

    if (!context_)
    {
        MO_DEBUG_GL("creating context in window");

        context_ = new MO::GL::Context(this);
        context_->setFormat(requestedFormat());
        if (!context_->create())
            MO_GL_ERROR("could not create context");

        emit contextCreated(context_);

        //needsInit = true;
    }

    if (!context_->makeCurrent(this))
        MO_GL_ERROR("could not make context current");

    context_->setSize(size());

    emit renderRequest();


#if (0)
    if (!gl_)
    {
        MO_DEBUG_GL("requesting openGL functions");
        gl_ = context_->versionFunctions<MO_OPENGL_FUNCTION_CLASS>();
        if (!gl_)
            MO_GL_ERROR("could not receive QOpenGLFunctions object");

        MO_DEBUG_GL("initializing openGL functions");

        if (!gl_->initializeOpenGLFunctions())
            MO_GL_ERROR("could not initialize openGL functions");
    }

    if (needsInit)
    {
        GLint vmaj, vmin;
        gl_->glGetIntegerv(GL_MAJOR_VERSION, &vmaj);
        gl_->glGetIntegerv(GL_MINOR_VERSION, &vmin);
        qDebug() << "vendor:  " << QString((const char*)gl_->glGetString(GL_VENDOR))
                 << "\nversion: " << vmaj << "." << vmin;
    }

    const qreal retinaScale = devicePixelRatio();
    gl_->glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    gl_->glClearColor(0,0.5,0.5,1);
    gl_->glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT);
    qDebug() << gl_->glGetError();
    //gl->glBegin();
    //frameBuffer_->texture();
#endif

    context_->swapBuffers(this);

    // call again :)
    if (animating_)
        renderLater();
}


} // namespace GL
} // namespace MO
