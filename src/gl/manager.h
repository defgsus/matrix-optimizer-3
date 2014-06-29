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


    Window * createGlWindow();


signals:

public slots:

};

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_MANAGER_H
