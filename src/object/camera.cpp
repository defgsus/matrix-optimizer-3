/** @file camera.cpp

    @brief Camera Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include "camera.h"
#include "gl/context.h"

namespace MO {

MO_REGISTER_OBJECT(Camera)

Camera::Camera(QObject *parent) :
    ObjectGl(parent)
{
    setName("Camera");
    projection_ = glm::perspective(63.f, (float)1.0, 0.1f, 1000.0f);
}


void Camera::initGl()
{
    projection_ = glm::perspective(63.f, (float)
        glContext()->size().width()/glContext()->size().height(),
        0.1f, 1000.0f);
}


void Camera::renderGl()
{
}

} // namespace MO
