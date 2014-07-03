/** @file camera.cpp

    @brief Camera Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

//#include <QDebug>

#include "camera.h"
#include "gl/context.h"
#include "io/datastream.h"


namespace MO {

MO_REGISTER_OBJECT(Camera)

Camera::Camera(QObject *parent) :
    ObjectGl(parent)
{
    setName("Camera");
}

void Camera::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);
    io.writeHeader("cam", 1);
}

void Camera::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);
    io.readHeader("cam", 1);
}


void Camera::setNumberThreads(int num)
{
    ObjectGl::setNumberThreads(num);

    projection_.resize(num);
}

void Camera::initGl(int thread)
{
    projection_[thread] = glm::perspective(63.f, (float)
        glContext(thread)->size().width()/glContext(thread)->size().height(),
        0.1f, 1000.0f);
}


void Camera::renderGl(int , Double )
{
}

void Camera::startGlFrame(int thread, Double )
{
    glViewport(0, 0, glContext(thread)->size().width(), glContext(thread)->size().height());

    glClearColor(0,0.2,0.2,1);
    glClear(GL_COLOR_BUFFER_BIT);

    setProjectionMatrix(thread);
}

void Camera::setProjectionMatrix(int thread)
{
    // XXX this is a hack
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&projection_[thread][0][0]);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

} // namespace MO
