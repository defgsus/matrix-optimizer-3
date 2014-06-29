/** @file window.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include <QDebug>
#include <QCoreApplication>
#include <QShowEvent>
#include <QOpenGLFramebufferObject>

#include "window.h"
#include "context.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GL {

Window::Window(QScreen * targetScreen)
    :   QWindow       (targetScreen),
        context_      (0),
        updatePending_(0)
{
    MO_DEBUG_GL("Window::Window()");

    setTitle("OpenGL");

    setSurfaceType(QSurface::OpenGLSurface);

    QSurfaceFormat format;
    format.setSamples(16);
    setFormat(format);
}

Window::~Window()
{
    MO_DEBUG_GL("Window::~Window()");
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
    if (e->type() == QEvent::UpdateRequest)
    {
        updatePending_ = false;
        renderNow();
        return true;
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
}


#if 0
void OpenGLWindow::render()
{
    if (!m_device)
        m_device = new QOpenGLPaintDevice;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_device->setSize(size());

    QPainter painter(m_device);
    render(&painter);
}
//! [2]

//! [3]
void OpenGLWindow::renderLater()
{
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool OpenGLWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        m_update_pending = false;
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}
//! [3]

//! [4]
void OpenGLWindow::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}




void OpenGLWindow2::renderLater()
{
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool OpenGLWindow2::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        m_update_pending = false;
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void OpenGLWindow2::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}
//! [3]

//! [4]
void OpenGLWindow2::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize)
    {
        //initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}
#endif


} // namespace GL
} // namespace MO
