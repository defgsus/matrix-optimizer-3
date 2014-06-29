/** @file window.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include <QDebug>
#include <QShowEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions_1_2>

#include "window.h"
#include "context.h"
#include "io/error.h"

namespace MO {
namespace GL {

Window::Window(QScreen * targetScreen)
    :   QWindow       (targetScreen),
        context_      (0),
        frameBuffer_  (0),
        isFullScreen_ (false)
{
    setTitle("OpenGL");

    setSurfaceType(QSurface::OpenGLSurface);
    QSurfaceFormat format;
    format.setMajorVersion(1);
    format.setMinorVersion(2);
    setFormat(format);
}


void Window::setFullScreen(bool fs)
{
    isFullScreen_ = fs;

    if (fs && isVisible())
        setVisibility(FullScreen);
}

void Window::setContext(Context * c)
{
    context_ = c;
}

void Window::setFramebuffer(QOpenGLFramebufferObject *frameBuffer)
{
    frameBuffer_ = frameBuffer;
}

void Window::showEvent(QShowEvent * e)
{
    QWindow::showEvent(e);

    if (isFullScreen_)
        setVisibility(FullScreen);
}

void Window::exposeEvent(QExposeEvent *)
{
    if (isExposed())
        render_();
}

bool Window::event(QEvent * e)
{
    if (e->type() == QEvent::UpdateRequest)
    {
        render_();
        return true;
    }
    return QWindow::event(e);
}

void Window::render_()
{
    qDebug() << "render";
    if (!isExposed() || !context_)
        return;

    bool needsInit = false;

    if (!context_->isValid())
    {
        context_->setFormat(requestedFormat());
        if (!context_->create())
            MO_GL_ERROR("could not create context");

        needsInit = true;
    }

    if (!context_->makeCurrent(this))
        MO_GL_ERROR("could not make context current")

    auto gl = context_->versionFunctions<QOpenGLFunctions_1_2>();
    if (!gl)
        MO_GL_ERROR("could not receive QOpenGLFunctions object");

    if (needsInit)
    {
        gl->initializeOpenGLFunctions();
        GLint vmaj, vmin;
        gl->glGetIntegerv(GL_MAJOR_VERSION, &vmaj);
        gl->glGetIntegerv(GL_MINOR_VERSION, &vmin);
        qDebug() << "vendor:  " << QString((const char*)gl->glGetString(GL_VENDOR))
                 << "\nversion: " << vmaj << "." << vmin;
    }

    gl->glViewport(0, 0, width(), height());

    gl->glClearColor(0,0,0,1);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    qDebug() << gl->glGetError();
    //gl->glBegin();
    //frameBuffer_->texture();

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
#endif


} // namespace GL
} // namespace MO
