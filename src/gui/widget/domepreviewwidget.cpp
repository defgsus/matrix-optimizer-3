/** @file domepreviewwidget.cpp

    @brief Display of dome and projectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include <vector>

#include <QPainter>
#include <QImage>

#include "domepreviewwidget.h"
#include "projection/projectionsystemsettings.h"
#include "projection/projectormapper.h"
#include "geom/geometry.h"
#include "geom/geometryfactory.h"
#include "gl/drawable.h"
#include "gl/compatibility.h"
#include "gl/shadersource.h"
#include "gl/texture.h"
#include "img/image.h"
#include "math/vector.h"
#include "io/log.h"
#include "io/settings.h"

namespace MO {
namespace GUI {

DomePreviewWidget::DomePreviewWidget(QWidget *parent)
    : Basic3DWidget (Basic3DWidget::RM_DIRECT, parent),
      settings_     (new ProjectionSystemSettings()),
      textureFunc_  (0),
      domeGeometry_ (0),
      projectorGeometry_(0),
      showGrid_     (true),
      showDome_     (true),
      showRays_     (true),
      showCurrentCamera_(false),
      showProjectedSurface_(true),
      showSliceTexture_(false)

{
    setObjectName("_DomePreviewWidget");

    showGrid_ = MO::settings->value(objectName()+"/showGrid", true).toBool();
    showDome_ = MO::settings->value(objectName()+"/showDome", true).toBool();
    showRays_ = MO::settings->value(objectName()+"/showRays", true).toBool();
    showSliceTexture_ = MO::settings->value(objectName()+"/showTex", true).toBool();

    createDomeGeometry_();
    createProjectorGeometry_();
}

DomePreviewWidget::~DomePreviewWidget()
{
    MO::settings->setValue(objectName()+"/showGrid", showGrid_);
    MO::settings->setValue(objectName()+"/showDome", showDome_);
    MO::settings->setValue(objectName()+"/showRays", showRays_);
    MO::settings->setValue(objectName()+"/showTex", showSliceTexture_);

    for (auto i : ptextureGeom_)
        delete i;
    for (auto i : ptextureDrawable_)
        delete i;
    delete domeGeometry_;
    delete projectorGeometry_;
    delete settings_;
}

void DomePreviewWidget::setViewMatrix(const Mat4 & m)
{
    Basic3DWidget::setViewMatrix(m * glm::inverse(domeTransform_));
}

void DomePreviewWidget::setShowRays(bool enable)
{
    if (enable == showRays_)
        return;

    showRays_ = enable;
    createProjectorGeometry_();
    update();
}

void DomePreviewWidget::setShowTexture(bool enable)
{
    if (enable == showSliceTexture_)
        return;

    showSliceTexture_ = enable;
    createProjectorGeometry_();
    update();
}

void DomePreviewWidget::setShowCurrentCamera(bool enable)
{
    if (enable == showCurrentCamera_)
        return;

    showCurrentCamera_ = enable;

    if (showCurrentCamera_)
    {
        lastRenderMode_ = renderMode();
        lastCameraMode_ = cameraMode();
        lastFboSize_ = fboSize();

        setRenderMode(RM_FRAMEBUFFER);
        setCameraMode(CM_SET);

        setCurrentCameraMatrix_();
    }
    else
    {
        // switch back
        setFboSize(lastFboSize_);
        setRenderMode(lastRenderMode_);
        setCameraMode(lastCameraMode_);
    }

    createProjectorGeometry_();
}


void DomePreviewWidget::setCurrentCameraMatrix_()
{
    if (projIndex_ < 0 || projIndex_ >= (int)settings_->numProjectors())
    {
        // set some default matrix if projector index is out of range
        setProjectionMatrix(MATH::perspective((Float)62,
                                             (Float)fboSize().width() / fboSize().height(),
                                             (Float)0.01,
                                             (Float)10000.));
        setViewMatrix(glm::translate(Mat4(1), Vec3(0,0,-10)));
    }
    else
    {
        const CameraSettings& c = settings_->cameraSettings(projIndex_);
        setFboSize(c.width(), c.height());
        setProjectionMatrix(c.getProjectionMatrix());
        setViewMatrix(c.getViewMatrix());
    }
}

void DomePreviewWidget::setProjectionSettings(
        const ProjectionSystemSettings & s, int currentIndex)
{
    *settings_ = s;
    projIndex_ = currentIndex;
    createDomeGeometry_();
    createProjectorGeometry_();
    if (showCurrentCamera_)
        setCurrentCameraMatrix_();
    update();
}

void DomePreviewWidget::createDomeGeometry_()
{
    delete domeGeometry_;
    domeGeometry_ = new GEOM::Geometry();
    domeGeometry_->setSharedVertices(false);
    domeGeometry_->setColor(0.5,0.5,0.5,1.0);

    GEOM::GeometryFactory::createDomeLines(domeGeometry_,
                                           settings_->domeSettings().radius(),
                                           settings_->domeSettings().coverage(),
                                           24, 12);
    Mat4 trans(1.0);

    if (settings_->domeSettings().tiltX() != 0)
    {
        trans = MATH::rotate(trans, -settings_->domeSettings().tiltX(), Vec3(1, 0, 0));
    }
    if (settings_->domeSettings().tiltZ() != 0)
    {
        trans = MATH::rotate(trans, settings_->domeSettings().tiltZ(), Vec3(0, 0, 1));
    }

    // transform
    domeGeometry_->applyMatrix(trans);

    // make sit on floor
    Vec3 mi, ma;
    domeGeometry_->getExtent(&mi, &ma);
    domeGeometry_->translate(0,-mi[1], 0);

    // store this transformation
    domeTransform_ = glm::translate(Mat4(1.0), Vec3(0,-mi[1],0)) * trans;

    if (domeGeometry_->numTriangles())
        domeGeometry_->calculateTriangleNormals();
}

void DomePreviewWidget::createProjectorGeometry_()
{
    ProjectorMapper mapper;

    for (auto i : ptextureGeom_)
        delete i;
    ptextureGeom_.clear();

    // build geometry
    delete projectorGeometry_;
    projectorGeometry_ = new GEOM::Geometry();

    for (uint i=0; i<settings_->numProjectors(); ++i)
    {
        const bool highlight = (projIndex_ < 0 || projIndex_ == (int)i);
        if (showCurrentCamera_ && !highlight)
            continue;

        mapper.setSettings(settings_->domeSettings(),
                           settings_->projectorSettings(i));
        if (!mapper.isValid())
            continue;

        // projector rays
        if (showRays_)
        {
            if (highlight)
                projectorGeometry_->setColor(0.7,0.7,0.5,1);
            else
                projectorGeometry_->setColor(0.3,0.3,0.2,1);

            for (int i=0; i<3; ++i)
            {
                const Float x = (Float)i/2;

                Vec3 pos = mapper.getRayOrigin(x,0);
                int v0 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                pos = mapper.mapToDome(x,0);
                int v1 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                projectorGeometry_->addLine(v0, v1);

                pos = mapper.getRayOrigin(x,1);
                v0 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                pos = mapper.mapToDome(x,1);
                v1 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                projectorGeometry_->addLine(v0, v1);

                pos = mapper.getRayOrigin(0,x);
                v0 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                pos = mapper.mapToDome(0,x);
                v1 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                projectorGeometry_->addLine(v0, v1);

                pos = mapper.getRayOrigin(1,x);
                v0 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                pos = mapper.mapToDome(1,x);
                v1 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                projectorGeometry_->addLine(v0, v1);
            }
        }

        if (showProjectedSurface_)
        {
            if (highlight)
                projectorGeometry_->setColor(0.5,1.0,0.5,1);
            else
                projectorGeometry_->setColor(0.2,0.3,0.2,1);

            std::vector<GEOM::Geometry::IndexType> idx;
            const int num = 11;
            // create grid
            for (uint y = 0; y<num; ++y)
            for (uint x = 0; x<num; ++x)
            {
                const Vec3 pos = mapper.mapToDome(
                            (Float)x/(num-1), (Float)y/(num-1));
                idx.push_back( projectorGeometry_->addVertex(pos[0], pos[1], pos[2]));
            }

            // connect grid
            for (uint y = 0; y<num; ++y)
            for (uint x = 0; x<num; ++x)
            {
                if (x>0)
                    projectorGeometry_->addLine(idx[y*num+x-1], idx[y*num+x]);
                if (y>0)
                    projectorGeometry_->addLine(idx[(y-1)*num+x], idx[y*num+x]);
            }
        }


        // textured slice

        if (showSliceTexture_)
        {
            GEOM::Geometry * g = new GEOM::Geometry();

            if (highlight)
                g->setColor(1,1,1,1);
            else
                g->setColor(0.5,0.5,0.5,1);

            std::vector<GEOM::Geometry::IndexType> idx;
            const int num = 11;
            // create grid
            for (uint y = 0; y<num; ++y)
            for (uint x = 0; x<num; ++x)
            {
                const Vec3 pos = mapper.mapToDome(
                            (Float)x/(num-1), (Float)y/(num-1));
                g->setTexCoord(Float(x)/(num-1), Float(y)/(num-1));
                idx.push_back( g->addVertex(pos[0], pos[1], pos[2]));
            }

            // connect grid
            for (uint y = 1; y<num; ++y)
            for (uint x = 1; x<num; ++x)
            {
                g->addTriangle(idx[(y-1)*num+x-1], idx[(y-1)*num+x], idx[y*num+x]);
                g->addTriangle(idx[(y-1)*num+x-1], idx[y*num+x], idx[y*num+x-1]);
            }

            ptextureGeom_.push_back(g);
        }

    }

    projectorGeometry_->applyMatrix(domeTransform_);
    for (auto p : ptextureGeom_)
        p->applyMatrix(domeTransform_);
}

void DomePreviewWidget::initGL()
{
    domeDrawable_ = new GL::Drawable(objectName() + "_dome");
    projectorDrawable_ = new GL::Drawable(objectName() + "_proj");
}

void DomePreviewWidget::releaseGL()
{
    if (domeDrawable_->isReady())
        domeDrawable_->releaseOpenGl();
    delete domeDrawable_;

    if (projectorDrawable_->isReady())
        projectorDrawable_->releaseOpenGl();
    delete projectorDrawable_;

    for (auto i : ptextureDrawable_)
    {
        if (i->isReady())
            i->releaseOpenGl();
        delete i;
    }
    ptextureDrawable_.clear();

    for (auto i : ptexture_)
    {
        if (i->isCreated())
            i->release();
        delete i;
    }
    ptexture_.clear();
}

void DomePreviewWidget::drawGL(const Mat4 &projection,
                               const Mat4 &cubeViewTrans,
                               const Mat4 &viewTrans,
                               const Mat4 &trans)
{
    if (domeGeometry_)
    {
        domeDrawable_->setGeometry(domeGeometry_);
        domeGeometry_ = 0;

        domeDrawable_->createOpenGl();
    }

    if (projectorGeometry_)
    {
        projectorDrawable_->setGeometry(projectorGeometry_);
        projectorGeometry_ = 0;

        projectorDrawable_->createOpenGl();
    }

    if (ptextureGeom_.size())
    {
        // create drawables if necessary
        if (ptextureDrawable_.size() < ptextureGeom_.size())
        {
            int s = ptextureDrawable_.size();
            ptextureDrawable_.resize(ptextureGeom_.size());
            for (uint i=s; i<ptextureDrawable_.size(); ++i)
            {
                ptextureDrawable_[i] = new GL::Drawable(QString("_tex%1").arg(i));
                GL::ShaderSource * src = new GL::ShaderSource();
                src->loadDefaultSource();
                src->addDefine("#define MO_ENABLE_TEXTURE");
                ptextureDrawable_[i]->setShaderSource(src);
            }
            // also create textures
            ptexture_.resize(ptextureGeom_.size());
            for (uint i=s; i<ptextureDrawable_.size(); ++i)
            {
                createTexture_(&ptexture_[i], i);
            }
        }
        // or remove them
        else if (ptextureDrawable_.size() > ptextureGeom_.size())
        {
            int s = ptextureDrawable_.size();
            for (uint i=s; i<ptextureDrawable_.size(); ++i)
            {
                if (ptextureDrawable_[i]->isReady())
                    ptextureDrawable_[i]->releaseOpenGl();
                delete ptextureDrawable_[i];
            }
            ptextureDrawable_.resize(ptextureGeom_.size());
            // remove textures
            for (uint i=s; i<ptexture_.size(); ++i)
            {
                if (ptexture_[i]->isCreated())
                    ptexture_[i]->release();
                delete ptexture_[i];
            }
            ptexture_.resize(ptextureGeom_.size());
        }
        // create vaos
        for (uint i = 0; i < ptextureGeom_.size(); ++i)
        {
            ptextureDrawable_[i]->setGeometry(ptextureGeom_[i]);
            ptextureDrawable_[i]->createOpenGl();
        }

        ptextureGeom_.clear();

    }

    // --- draw ---

    MO_CHECK_GL( gl::glClearColor(0, 0, 0, 1) );
    MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );

    MO_CHECK_GL( gl::glDisable(gl::GL_DEPTH_TEST) );
    MO_CHECK_GL( gl::glEnable(gl::GL_BLEND) );
    MO_CHECK_GL( gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE) );
    GL::setLineSmooth(true);
    GL::setLineWidth((GLfloat)fboSize().height() / 512);

    if (domeDrawable_->isReady() && showDome_)
        domeDrawable_->renderShader(projection, cubeViewTrans, viewTrans, trans);

    if (projectorDrawable_->isReady())
        projectorDrawable_->renderShader(projection, cubeViewTrans, viewTrans, trans);

    if (showSliceTexture_)
    for (uint i=0; i<ptextureDrawable_.size(); ++i)
    {
        ptexture_[i]->bind();
        ptextureDrawable_[i]->renderShader(projection, cubeViewTrans, viewTrans, trans);
    }

    if (showGrid_)
        drawGrid(projection, cubeViewTrans, viewTrans, trans);

    MO_CHECK_GL( gl::glDisable(gl::GL_BLEND) );
}


void DomePreviewWidget::createTexture_(GL::Texture **tex, int index)
{
    MO_DEBUG_GL("DomePreviewWidget::createTexture_(" << *tex << ", " << index << ")");

    // use callback
    if (textureFunc_)
    {
        *tex = textureFunc_(index);
        // restore viewport
        MO_CHECK_GL( gl::glViewport(0,0,width(), height()) );
        return;
    }

    // render a number per projector

    QImage qimg(320,200, QImage::Format_ARGB32);
    QPainter p(&qimg);

    qimg.fill(QColor(100,100,100));
    p.setPen(QPen(Qt::white));
    p.setBrush(QBrush(Qt::white));
    QFont font(p.font());
    font.setPixelSize(qimg.height()/2);
    p.setFont(font);
    p.drawText(qimg.rect(), Qt::AlignCenter | Qt::AlignVCenter,
               QString("%1").arg(index+1));

    Image img;
    img.createFrom(qimg);

    *tex = GL::Texture::createFromImage(img, gl::GL_RGB);

    //tex->create(320,200,gl::GL_RGB,gl::GL_FLOAT,0);
}


} // namespace GUI
} // namespace MO
