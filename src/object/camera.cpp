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
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "scene.h"

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
    screenQuad_.resize(num);
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

    // screen-quad
    screenQuad_[thread] = new GL::ScreenQuad(GL::ER_THROW);
    screenQuad_[thread]->create();

    // create framebuffer

    Scene * scene = sceneObject();
    MO_ASSERT(scene, "Camera::initGl() without scene object");

    fbo_[thread] = new GL::FrameBufferObject(
                scene->frameBufferWidth(),
                scene->frameBufferHeight(),
                scene->frameBufferFormat(),
                GL_FLOAT,
                GL::ER_THROW);
    fbo_[thread]->create();
    fbo_[thread]->unbind();

}

void Camera::initCameraSpace(GL::CameraSpace &cam, uint thread, uint sample) const
{
    cam.setProjectionMatrix(projection_[thread][sample]);
}

void Camera::startGlFrame(uint thread, Double )
{
    GL::FrameBufferObject * fbo = fbo_[thread];
    fbo->bind();

    MO_CHECK_GL( glViewport(0, 0, fbo->width(), fbo->height()) );

    MO_CHECK_GL( glClearColor(0,0.2,0.2,1) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

    MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );

}

void Camera::finishGlFrame(uint thread, Double)
{
    fbo_[thread]->unbind();
    //MO_CHECK_GL( glViewport(0, 0, glContext(thread)->size().width(), glContext(thread)->size().height()) );
}

void Camera::drawFramebuffer(uint thread, Double )
{
    GL::FrameBufferObject * fbo = fbo_[thread];

    fbo->colorTexture()->bind();
    screenQuad_[thread]->draw(fbo->width(), fbo->height());
    fbo->colorTexture()->unbind();
}


} // namespace MO
