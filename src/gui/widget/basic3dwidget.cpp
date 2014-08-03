/** @file basic3dwidget.cpp

    @brief prototype for a view into an opengl scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include <QPainter>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QWheelEvent>

#include "basic3dwidget.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/drawable.h"
#include "io/log.h"
#include "math/cubemapmatrix.h"
#include "geom/geometryfactory.h"
#include "geom/freecamera.h"


namespace MO {
namespace GUI {


Basic3DWidget::Basic3DWidget(RenderMode mode, QWidget *parent) :
    QGLWidget       (parent),
    renderMode_     (mode),
    isGlInitialized_(false),
    closeRequest_   (false),
    modeChangeRequest_(false),
    useFreeCamera_  (true),
    camera_         (new GEOM::FreeCamera()),
    fbo_            (0),
    screenQuad_     (0),
    gridObject_     (0)
{
    MO_DEBUG_GL("Basic3DWidget::Basic3DWidget()");

    //if (mode == RM_DIRECT)
    {
        QGLFormat f(format());
        f.setDepth(true);
        setFormat(f);
    }

    viewInit();
}

Basic3DWidget::~Basic3DWidget()
{
    MO_DEBUG_GL("Basic3DWidget::~Basic3DWidget()");

    // XXX This should always work when GUI has a single thread
    if (isGlInitialized_)
        releaseGL();

    delete camera_;
    delete fbo_;
    delete screenQuad_;
    delete gridObject_;
}

void Basic3DWidget::setRenderMode(RenderMode rm)
{
    if (rm == renderMode_)
        return;

    nextRenderMode_ = rm;
    modeChangeRequest_ = true;
    update();
}

void Basic3DWidget::viewInit(Float distanceZ)
{
    distanceZ_ = distanceZ;
    rotationMatrix_ = Mat4();
    camera_->moveTo(Vec3(0,0,-distanceZ));
    update();
}

void Basic3DWidget::viewRotateX(Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(Mat4(), d, Vec3(1,0,0))
            * rotationMatrix_;
    update();
}

void Basic3DWidget::viewRotateY(Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(Mat4(), d, Vec3(0,1,0))
            * rotationMatrix_;
    update();
}

Mat4 Basic3DWidget::transformationMatrix() const
{
    if (useFreeCamera_)
        return camera_->getMatrix();
    else
    {
        Mat4 m = glm::translate(Mat4(), Vec3(0,0,-distanceZ_));
        return m * rotationMatrix_;
    }
}

void Basic3DWidget::mousePressEvent(QMouseEvent * e)
{
    lastMousePos_ = e->pos();
}

void Basic3DWidget::mouseMoveEvent(QMouseEvent * e)
{
    Float fac = e->modifiers() & Qt::SHIFT ?
                10.f : 1.f;

    int dx = lastMousePos_.x() - e->x(),
        dy = lastMousePos_.y() - e->y();
    lastMousePos_ = e->pos();

    if (e->buttons() & Qt::LeftButton)
    {
        if (!useFreeCamera_)
        {
            viewRotateX(dy);
            viewRotateY(dx);
        }
        else
        {
            //camera_->rotateX(dy);
            //camera_->rotateY(dx);
            camera_->moveX(-0.03*fac*dx);
            camera_->moveY( 0.03*fac*dy);
            update();
        }
    }

    if (e->buttons() & Qt::RightButton)
    {
        if (!useFreeCamera_)
        {
            distanceZ_ += 0.04 * fac * dy;
        }
        else
        {
            camera_->rotateX(-dy * fac);
            camera_->rotateY(-dx * fac);
            //camera_->moveZ(-0.03 * dy);
        }
        update();
    }
}

void Basic3DWidget::wheelEvent(QWheelEvent * e)
{
    Float fac = e->modifiers() & Qt::SHIFT ?
                10.f : 1.f;

    Float d = std::max(-1, std::min(1, e->delta() ));
    camera_->moveZ(-0.3 * d * fac);
    update();
}

void Basic3DWidget::closeEvent(QCloseEvent * e)
{
    if (isGlInitialized_)
    {
        e->ignore();
        // pass shut-down function to paintGL
        closeRequest_ = true;
        update();
    }
    else QGLWidget::closeEvent(e);
}

void Basic3DWidget::initializeGL()
{
    MO_DEBUG_GL("Basic3DWidget::initializeGL()");

    createGLStuff_();

    isGlInitialized_ = true;

    emit glInitialized();
}

void Basic3DWidget::releaseGL()
{
    MO_DEBUG_GL("Basic3DWidget::releaseGL()");

    releaseGLStuff_();

    isGlInitialized_ = false;

    emit glReleased();
}

void Basic3DWidget::createGLStuff_()
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
}

void Basic3DWidget::releaseGLStuff_()
{
    if (fbo_)
    {
        fbo_->release();
        delete fbo_;
        fbo_ = 0;
    }

    if (screenQuad_)
    {
        screenQuad_->release();
        delete screenQuad_;
        screenQuad_ = 0;
    }

    if (gridObject_)
    {
        gridObject_->releaseOpenGl();
        delete gridObject_;
        gridObject_ = 0;
    }
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
                glm::perspective(90.f, (float)fbo_->width()/fbo_->height(), 0.01f, 1000.0f);
    }
    else
    {
        projectionMatrix_ = glm::perspective(angle, (float)w/h, 0.1f, 1000.0f);
        MO_CHECK_GL( glViewport(0,0,w,h) );
    }
}

void Basic3DWidget::paintGL()
{
    if (modeChangeRequest_)
    {
        modeChangeRequest_ = false;
        releaseGLStuff_();
        renderMode_ = nextRenderMode_;
        createGLStuff_();
        resizeGL(width(), height());
    }

    if (closeRequest_)
    {
        releaseGL();
        closeRequest_ = false;
        return;
    }

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

        const Mat4 viewm = transformationMatrix();

        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
        drawGL(projectionMatrix(), MATH::CubeMapMatrix::negativeZ * viewm);
        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
        drawGL(projectionMatrix(), MATH::CubeMapMatrix::positiveX * viewm);
        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
        drawGL(projectionMatrix(), MATH::CubeMapMatrix::negativeX * viewm);
        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
        drawGL(projectionMatrix(), MATH::CubeMapMatrix::positiveY * viewm);
        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
        drawGL(projectionMatrix(), MATH::CubeMapMatrix::negativeY * viewm);
        // XXX only needed for angle-of-view >= 250
        fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
        drawGL(projectionMatrix(), MATH::CubeMapMatrix::positiveZ * viewm);

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


void Basic3DWidget::drawGrid(const Mat4 &projection, const Mat4 &transformation)
{
    if (!gridObject_)
    {
        gridObject_ = new GL::Drawable("grid3d");
        GEOM::GeometryFactory::createGridXZ(
                    gridObject_->geometry(), 10, 10, true);
        gridObject_->createOpenGl();
    }

    if (gridObject_->isReady())
        gridObject_->renderShader(projection, transformation);
}

} // namespace GUI
} // namespace MO
