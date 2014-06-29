/** @file manager.h

    @brief OpenGL window & assets manager

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_GL_MANAGER_H
#define MOSRC_GL_MANAGER_H

#include <QObject>

namespace MO {
namespace GL {

class Window;
class Context;

class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject *parent = 0);

    // ---------------- opengl ---------------------

    /** Creates an OpenGL Window.
        When the window is shown, it's Context will be created
        and Manager::contextCreated() will be emitted. */
    Window * createGlWindow();


signals:

    /** This will signal the creation of a new Context */
    void contextCreated(Context *);

public slots:

};

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_MANAGER_H
