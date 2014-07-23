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


void Camera::setNumberThreads(uint num)
{
    ObjectGl::setNumberThreads(num);

    projection_.resize(num);
}

void Camera::setBufferSize(uint bufferSize, uint thread)
{
    ObjectGl::setBufferSize(bufferSize, thread);

    projection_[thread].resize(bufferSize);
}

void Camera::initGl(uint thread)
{
    projection_[thread][0]
        = glm::perspective(63.f,
                (float)glContext(thread)->size().width()/glContext(thread)->size().height(),
                0.1f, 1000.0f);
}


void Camera::renderGl(uint , Double )
{
}

void Camera::startGlFrame(uint thread, Double )
{
    glViewport(0, 0, glContext(thread)->size().width(), glContext(thread)->size().height());

    glClearColor(0,0.2,0.2,1);
    glClear(GL_COLOR_BUFFER_BIT);

    setProjectionMatrix(thread, 0);
}

void Camera::setProjectionMatrix(uint thread, uint sample)
{
    // XXX this is a hack
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&projection_[thread][sample][0][0]);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

} // namespace MO
