/** @file window.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/28/2014</p>
*/

#include <QShowEvent>

#include "window.h"
#include "context.h"


namespace MO {
namespace GL {

Window::Window(QScreen * targetScreen)
    :   QWindow       (targetScreen),
        context_      (0),
        isFullScreen_ (false)
{

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


void Window::showEvent(QShowEvent * e)
{
    QWindow::showEvent(e);

    if (isFullScreen_)
        setVisibility(FullScreen);
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
    if (!isExposed() || !context_)
        return;

    context_->makeCurrent(this);

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
