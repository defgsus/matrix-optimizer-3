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
    cameraMode_     (CM_FREE),
    isGlInitialized_(false),
    closeRequest_   (false),
    modeChangeRequest_(false),
    orthoScale_     (3.f),
    fboSize_        (512, 512),
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

    viewSet(VD_FRONT, 10);
}

Basic3DWidget::~Basic3DWidget()
{
    MO_DEBUG_GL("Basic3DWidget::~Basic3DWidget()");

    if (isGlInitialized_)
        MO_GL_WARNING("destructor of Basic3DWidget without releaseGL");

    delete camera_;
    delete fbo_;
    delete screenQuad_;
    delete gridObject_;
}

void Basic3DWidget::setFboSize(const QSize & s)
{
    // XXX fbo_ and fboSize_ might be different when modeChangeRequest_ is pending
    //     need to think about that ...
    if (fbo_ && (int)fbo_->width() == s.width() && (int)fbo_->height() == s.height())
        return;

    // get current render mode
    // (a change request could be pending)
    RenderMode rm = renderMode_;
    if (modeChangeRequest_)
        rm = nextRenderMode_;

    fboSize_ = s;

    if (rm == RM_DIRECT || rm == RM_DIRECT_ORTHO)
        return;

    nextRenderMode_ = rm;
    modeChangeRequest_ = true;
    update();
}

void Basic3DWidget::setRenderMode(RenderMode rm)
{
    if (rm == renderMode_)
        return;

    nextRenderMode_ = rm;
    modeChangeRequest_ = true;
    update();
}

void Basic3DWidget::setCameraMode(CameraMode cm)
{
    if (cm == cameraMode_)
        return;

    cameraMode_ = cm;
    update();
}

void Basic3DWidget::viewRotateX(Float d)
{
    if (!d) return;
    if (cameraMode_ == CM_CENTER)
    {
        rotationMatrix_ =
            glm::rotate(Mat4(1), d, Vec3(1,0,0))
            * rotationMatrix_;
    }
    else if (cameraMode_ == CM_SET)
    {
        fixViewMatrix_ = glm::rotate(fixViewMatrix_, d, Vec3(1,0,0));
    }
    else camera_->rotateX(d);

    update();
}

void Basic3DWidget::viewRotateY(Float d)
{
    if (!d) return;
    if (cameraMode_ == CM_CENTER)
    {
        rotationMatrix_ =
            glm::rotate(Mat4(1), d, Vec3(0,1,0))
            * rotationMatrix_;
    }
    else if (cameraMode_ == CM_SET)
    {
        fixViewMatrix_ = glm::rotate(fixViewMatrix_, d, Vec3(0,1,0));
    }
    else camera_->rotateY(d);

    update();
}

void Basic3DWidget::viewSet(ViewDirection dir, Float distance)
{
    Mat4 mat(1);
    switch (dir)
    {
        case VD_FRONT:
            mat = glm::translate(mat, Vec3(0,0,-distance));
        break;

        case VD_BACK:
            mat = glm::translate(mat, Vec3(0,0,-distance));
            mat = glm::rotate(mat, 180.f, Vec3(0,1,0));
        break;

        case VD_TOP:
            mat = glm::translate(mat, Vec3(0,0,-distance));
            mat = glm::rotate(mat, 90.f, Vec3(1,0,0));
        break;

        case VD_BOTTOM:
            mat = glm::translate(mat, Vec3(0,0,-distance));
            mat = glm::rotate(mat, -90.f, Vec3(1,0,0));
        break;

        case VD_LEFT:
            mat = glm::translate(mat, Vec3(0,0,-distance));
            mat = glm::rotate(mat, 90.f, Vec3(0,1,0));
        break;

        case VD_RIGHT:
            mat = glm::translate(mat, Vec3(0,0,-distance));
            mat = glm::rotate(mat, -90.f, Vec3(0,1,0));
        break;
    }

    if (cameraMode_ == CM_FREE)
        camera_->setMatrix(mat);
    else
    if (cameraMode_ == CM_SET)
        fixViewMatrix_ = mat;
    else
    {
        rotationMatrix_ = mat;
        rotationMatrix_[3][0] = 0;
        rotationMatrix_[3][1] = 0;
        rotationMatrix_[3][2] = 0;
        distanceZ_ = distance;
    }

    update();
}

void Basic3DWidget::setProjectionMatrix(const Mat4 & m)
{
    fixProjectionMatrix_ = m;
    update();
}

void Basic3DWidget::setViewMatrix(const Mat4 & m)
{
    fixViewMatrix_ = m;
    update();
}

const Mat4& Basic3DWidget::projectionMatrix() const
{
    if (cameraMode_ == CM_SET)
        return fixProjectionMatrix_;
    else
        return projectionMatrix_;
}

Mat4 Basic3DWidget::viewMatrix() const
{
    if (cameraMode_ == CM_FREE)
        return camera_->getMatrix();
    else
    if (cameraMode_ == CM_CENTER)
    {
        Mat4 m = glm::translate(Mat4(), Vec3(0,0,-distanceZ_));
        return m * rotationMatrix_;
    }
    else return fixViewMatrix_;
}

void Basic3DWidget::mousePressEvent(QMouseEvent * e)
{
    lastMousePos_ = e->pos();
}

void Basic3DWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (cameraMode_ == CM_SET)
        return;

    Float fac = e->modifiers() & Qt::SHIFT ?
                10.f : e->modifiers() & Qt::CTRL? 0.025f : 1.f;

    int dx = lastMousePos_.x() - e->x(),
        dy = lastMousePos_.y() - e->y();
    lastMousePos_ = e->pos();

    if (e->buttons() & Qt::LeftButton)
    {
        if (cameraMode_ == CM_CENTER)
        {
            viewRotateX(dy);
            viewRotateY(dx);
        }
        else
        {
            camera_->moveX(-0.03*fac*dx);
            camera_->moveY( 0.03*fac*dy);
            update();
        }
    }

    if (e->buttons() & Qt::RightButton)
    {
        if (cameraMode_ == CM_CENTER)
        {
            distanceZ_ += 0.04 * fac * dy;
        }
        else
        {
            camera_->rotateX(-dy * fac);
            camera_->rotateY(-dx * fac);
        }
        update();
    }
}

void Basic3DWidget::wheelEvent(QWheelEvent * e)
{
    if (cameraMode_ != CM_FREE)
        return;

    Float fac = e->modifiers() & Qt::SHIFT ?
                10.f : e->modifiers() & Qt::CTRL? 0.025f : 1.f;

    Float d = std::max(-1, std::min(1, e->delta() ));
    camera_->moveZ(-0.3 * d * fac);
    update();
}

void Basic3DWidget::shutDownGL()
{
    closeRequest_ = true;
    update();
}

void Basic3DWidget::closeEvent(QCloseEvent * e)
{
    if (isGlInitialized_)
    {
        e->ignore();
        // pass shut-down signal to paintGL
        closeRequest_ = true;
        update();
    }
    else QGLWidget::closeEvent(e);
}

void Basic3DWidget::initializeGL()
{
    MO_DEBUG_GL("Basic3DWidget::initializeGL()");

    makeCurrent();

    MO_EXTEND_EXCEPTION(
        createGLStuff_(),
        "in Basic3DWidget " << this
        );

    // call derived classes
    initGL();

    isGlInitialized_ = true;

    emit glInitialized();
}

void Basic3DWidget::releaseGL_()
{
    MO_DEBUG_GL("Basic3DWidget::releaseGL_()");

    makeCurrent();

    MO_EXTEND_EXCEPTION(
        releaseGLStuff_(),
        "in Basic3DWidget " << this
        );

    // call derived classes
    releaseGL();

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

        fbo_ = new GL::FrameBufferObject(fboSize_.width(), fboSize_.height(),
                                         GL_RGBA, GL_FLOAT,
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
                glm::perspective(angle, (float)fbo_->width()/fbo_->height(), 0.01f, 1000.0f);
    }
    else
    if (renderMode_ == RM_FULLDOME_CUBE)
    {
        projectionMatrix_ =
                glm::perspective(90.f, (float)fbo_->width()/fbo_->height(), 0.01f, 1000.0f);
    }
    else
    if (renderMode_ == RM_DIRECT_ORTHO)
    {
        const float aspect = (float)w/h;
        const float scale = orthoScale_;
        projectionMatrix_ = glm::ortho(-scale*aspect, scale*aspect, -scale, scale, 0.01f, 1000.f);
    }
    else
        projectionMatrix_ =
                glm::perspective(angle, (float)w/h, 0.01f, 1000.0f);

    MO_CHECK_GL( glViewport(0,0,w,h) );
}

void Basic3DWidget::paintGL()
{
    makeCurrent();

    if (closeRequest_)
    {
        releaseGL_();
        closeRequest_ = false;
        return;
    }

    if (modeChangeRequest_)
    {
        modeChangeRequest_ = false;
        releaseGLStuff_();
        renderMode_ = nextRenderMode_;
        createGLStuff_();
        resizeGL(width(), height());
    }

    Mat4 identity(1.0);

    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT) );

    if (renderMode_ == RM_DIRECT || renderMode_ == RM_DIRECT_ORTHO)
    {
        MO_EXTEND_EXCEPTION(
            drawGL(projectionMatrix(), viewMatrix(), viewMatrix(), identity),
            "in Basic3DWidget " << this
            );
        return;
    }

    if (renderMode_ == RM_FRAMEBUFFER)
    {
        fbo_->bind();
        fbo_->setViewport();

        MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );

        MO_EXTEND_EXCEPTION(
            drawGL(projectionMatrix(), viewMatrix(), viewMatrix(), identity),
            "in Basic3DWidget " << this
            );

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

        const Mat4 viewm = viewMatrix();

        MO_EXTEND_EXCEPTION(
            fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
            drawGL(projectionMatrix(), MATH::CubeMapMatrix::negativeZ * viewm, viewm, identity);
            fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
            drawGL(projectionMatrix(), MATH::CubeMapMatrix::positiveX * viewm, viewm, identity);
            fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
            drawGL(projectionMatrix(), MATH::CubeMapMatrix::negativeX * viewm, viewm, identity);
            fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
            drawGL(projectionMatrix(), MATH::CubeMapMatrix::positiveY * viewm, viewm, identity);
            fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
            drawGL(projectionMatrix(), MATH::CubeMapMatrix::negativeY * viewm, viewm, identity);
            // XXX only needed for angle-of-view >= 250
            fbo_->attachCubeTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
            drawGL(projectionMatrix(), MATH::CubeMapMatrix::positiveZ * viewm, viewm, identity);
            ,
            "in Basic3DWidget " << this
            );

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


void Basic3DWidget::drawGrid(const Mat4 &projection,
                             const Mat4 &cubeViewTrans,
                             const Mat4 &viewTrans,
                             const Mat4 &trans)
{
    if (!gridObject_)
    {
        gridObject_ = new GL::Drawable("grid3d");
        GEOM::GeometryFactory::createGridXZ(
                    gridObject_->geometry(), 10, 10, true);
        gridObject_->createOpenGl();
    }

    if (gridObject_->isReady())
        gridObject_->renderShader(projection, cubeViewTrans, viewTrans, trans);
}

} // namespace GUI
} // namespace MO
