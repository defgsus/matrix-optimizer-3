/** @file camera.cpp

    @brief Camera Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/
#include <QDebug>
#include "camera.h"

namespace MO {



Camera::Camera(QObject *parent) :
    ObjectGl(parent)
{
}



void Camera::render()
{
    qDebug() << "cam render";
    glClearColor(0,0.5,0.5,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1,0,0);
    glBegin(GL_QUADS);
        glVertex2f(0,0);
        glVertex2f(10,0);
        glVertex2f(10,10);
        glVertex2f(0,10);
    glEnd();
}

} // namespace MO
