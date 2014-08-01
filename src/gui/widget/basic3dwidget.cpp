/** @file basic3dwidget.cpp

    @brief prototype for a view into an opengl scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include <QPainter>
#include <QMouseEvent>

#include "basic3dwidget.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"

namespace MO {
namespace GUI {


Basic3DWidget::Basic3DWidget(RenderMode mode, QWidget *parent) :
    QGLWidget       (parent),
    renderMode_     (mode),
    fbo_            (0),
    screenQuad_     (0)
{
    if (mode == RM_DIRECT)
    {
        QGLFormat f(format());
        f.setDepth(true);
        setFormat(f);
    }

    viewInit();
}

void Basic3DWidget::viewInit(Float distanceZ)
{
    distanceZ_ = distanceZ;
    rotationMatrix_ = Mat4();
    updateGL();
}

void Basic3DWidget::viewRotateX(Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(Mat4(), d, Vec3(1,0,0))
            * rotationMatrix_;
    updateGL();
}

void Basic3DWidget::viewRotateY(Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(Mat4(), d, Vec3(0,1,0))
            * rotationMatrix_;
    updateGL();
}

Mat4 Basic3DWidget::transformationMatrix() const
{
    Mat4 m = glm::translate(Mat4(), Vec3(0,0,-distanceZ_));
    return m * rotationMatrix_;
}

void Basic3DWidget::mousePressEvent(QMouseEvent * e)
{
    lastMousePos_ = e->pos();
}

void Basic3DWidget::mouseMoveEvent(QMouseEvent * e)
{
    int dx = lastMousePos_.x() - e->x(),
        dy = lastMousePos_.y() - e->y();
    lastMousePos_ = e->pos();

    if (e->buttons() & Qt::LeftButton)
    {
        viewRotateX(dy);
        viewRotateY(dx);
    }

    if (e->buttons() & Qt::RightButton)
    {
        distanceZ_ += 0.04 * dy;
        updateGL();
    }
}


void Basic3DWidget::initializeGL()
{
    if (renderMode_ == RM_FULLDOME_CUBE || renderMode_ == RM_FRAMEBUFFER)
    {
        screenQuad_ = new GL::ScreenQuad("basic3dwidget", GL::ER_THROW);
        screenQuad_->create(
                    renderMode_ == RM_FULLDOME_CUBE ?
                        "#define MO_FULLDOME_CUBE" : "");

        fbo_ = new GL::FrameBufferObject(512, 512, GL_RGBA, GL_FLOAT,
                                         (renderMode_ == RM_FULLDOME_CUBE),
                                         GL::ER_THROW);
        fbo_->create();
        fbo_->unbind();
    }

    MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );

    emit glInitialized();
}

void Basic3DWidget::resizeGL(int w, int h)
{
    float angle = 63;

    if (renderMode_ == RM_FRAMEBUFFER)
    {
        projectionMatrix_ =
                glm::perspective(angle, (float)fbo_->width()/fbo_->height(), 0.1f, 1000.0f);
    }
    else
    if (renderMode_ == RM_FULLDOME_CUBE)
    {
        projectionMatrix_ =
                glm::perspective(90.f, (float)fbo_->width()/fbo_->height(), 0.1f, 1000.0f);
    }
    else
    {
        projectionMatrix_ = glm::perspective(angle, (float)w/h, 0.1f, 1000.0f);
        MO_CHECK_GL( glViewport(0,0,w,h) );
    }
}

void Basic3DWidget::paintGL()
{
    if (renderMode_ == RM_DIRECT)
    {
        drawGL(projectionMatrix(), transformationMatrix());
        return;
    }

    if (renderMode_ == RM_FRAMEBUFFER)
    {
        fbo_->bind();
        fbo_->setViewport();

        MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );

        drawGL(projectionMatrix(), transformationMatrix());

        fbo_->unbind();

        // draw to screen
        MO_CHECK_GL( glViewport(0,0,width(), height()) );
        MO_CHECK_GL( glClearColor(0.1, 0.1, 0.1, 1.0) );
        MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT) );
        MO_CHECK_GL( glDisable(GL_DEPTH_TEST) );
        fbo_->colorTexture()->bind();
        screenQuad_->draw(width(), height());
        fbo_->colorTexture()->unbind();

    }

    if (renderMode_ == RM_FULLDOME_CUBE)
    {
        // ARB_seamless_cube_map
        MO_CHECK_GL( glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS) );

        fbo_->bind();
        fbo_->setViewport();

        MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );

        Mat4 rpx = glm::rotate(Mat4(1.0), -90.f, Vec3(0,1,0));
        Mat4 rnx = glm::rotate(Mat4(1.0), 90.f, Vec3(0,1,0));
        Mat4 rpy = glm::rotate(glm::rotate(Mat4(1.0), -90.f, Vec3(1,0,0)), 180.f, Vec3(0,0,1));
        Mat4 rny = glm::rotate(glm::rotate(Mat4(1.0), 90.f, Vec3(1,0,0)), 180.f, Vec3(0,0,1));;

        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
        drawGL(projectionMatrix(), transformationMatrix());
        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
        drawGL(projectionMatrix(), rpx * transformationMatrix());
        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
        drawGL(projectionMatrix(), rnx * transformationMatrix());
        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
        drawGL(projectionMatrix(), rpy * transformationMatrix());
        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
        drawGL(projectionMatrix(), rny * transformationMatrix());


        fbo_->unbind();

        // draw to screen
        MO_CHECK_GL( glViewport(0,0,width(), height()) );
        MO_CHECK_GL( glClearColor(0, 0, 0, 1.0) );
        MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT) );
        MO_CHECK_GL( glDisable(GL_DEPTH_TEST) );
        fbo_->colorTexture()->bind();
        screenQuad_->draw(width(), height());
        fbo_->colorTexture()->unbind();
    }
}

} // namespace GUI
} // namespace MO
