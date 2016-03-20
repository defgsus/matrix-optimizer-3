/** @file offscreencontext.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.03.2015</p>
*/

#ifndef MOSRC_GL_OFFSCREENCONTEXT_H
#define MOSRC_GL_OFFSCREENCONTEXT_H

#include "context.h"

class QOffscreenSurface;

namespace MO {
namespace GL {

class OffscreenContext : public Context
{
    Q_OBJECT
public:
    explicit OffscreenContext(QObject *parent = 0);
    ~OffscreenContext();

    // --------- getter --------------

    QOffscreenSurface * qsurface() const { return p_os_surface_; }

    // ---------- opengl -------------

public slots:

    /** Creates an offscreen surface and an QOpenGLContext associated to it.
        @throws GlException on any error. */
    void createGl();

    /** Destroys the context, or does nothing */
    void destroyGl();

    void swapBuffers();

signals:


protected:

    QOffscreenSurface * p_os_surface_;
};

} // namespace GL
} // namespace MO

#endif // OFFSCREENCONTEXT_H
