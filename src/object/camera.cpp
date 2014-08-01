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
#include "gl/cameraspace.h"
#include "gl/framebufferobject.h"
#include "io/log.h"

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
    fbo_.resize(num);
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

    fbo_[thread] = new GL::FrameBufferObject(
                1024,
                1024,//glContext(thread)->size().height(),
                GL_RGBA, GL_FLOAT, GL::ER_THROW);
    fbo_[thread]->create();
    fbo_[thread]->unbind();
}

void Camera::initCameraSpace(GL::CameraSpace &cam, uint thread, uint sample) const
{
    cam.setProjectionMatrix(projection_[thread][sample]);
}

void Camera::startGlFrame(uint thread, Double )
{
    //fbo_[thread]->bind();
    //MO_CHECK_GL( glViewport(0, 0, glContext(thread)->size().width(), glContext(thread)->size().height()) );
    MO_CHECK_GL( glViewport(0, 0, 1024, 1024) );

    MO_CHECK_GL( glClearColor(0,0.2,0.2,1) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

    MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );

    //MO_CHECK_GL( setProjectionMatrix(thread, 0) );
}

/*
void Camera::setProjectionMatrix(uint thread, uint sample)
{
    // XXX this is a hack
    MO_CHECK_GL( glMatrixMode(GL_PROJECTION) );
    MO_CHECK_GL( glLoadMatrixf(&projection_[thread][sample][0][0]) );
    MO_CHECK_GL( glMatrixMode(GL_MODELVIEW) );
    MO_CHECK_GL( glLoadIdentity() );
}
*/

} // namespace MO
