/** @file domepreviewwidget.cpp

    @brief Display of dome and projectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include <vector>

#include <QPainter>
#include <QImage>

#include "DomePreviewWidget.h"
#include "projection/ProjectionSystemSettings.h"
#include "projection/ProjectorMapper.h"
#include "geom/Geometry.h"
#include "geom/GeometryFactory.h"
#include "gl/Drawable.h"
#include "gl/compatibility.h"
#include "gl/ShaderSource.h"
#include "gl/Texture.h"
#include "math/vector.h"
#include "io/log_gl.h"
#include "io/Settings.h"

namespace MO {
namespace GUI {

DomePreviewWidget::DomePreviewWidget(QWidget *parent)
    : Basic3DWidget (Basic3DWidget::RM_DIRECT, parent),
      glProps_      (new GL::Properties),
      settings_     (new ProjectionSystemSettings()),
      textureFunc_  (0),
      domeGeometry_ (0),
      projectorGeometry_(0),
      showGrid_     (true),
      showDome_     (true),
      showRays_     (true),
      showCurrentCamera_(false),
      showProjectedSurface_(true),
      showSliceTexture_(false),
      showHighlight_(true)

{
    setObjectName("_DomePreviewWidget");

    showGrid_ = MO::settings()->value(objectName()+"/showGrid", true).toBool();
    showDome_ = MO::settings()->value(objectName()+"/showDome", true).toBool();
    showRays_ = MO::settings()->value(objectName()+"/showRays", true).toBool();
    showSliceTexture_ = MO::settings()->value(objectName()+"/showTex", true).toBool();
    showHighlight_ = MO::settings()->value(objectName()+"/showHighlight", true).toBool();

    createDomeGeometry_();
    createProjectorGeometry_();
}

DomePreviewWidget::~DomePreviewWidget()
{
    MO::settings()->setValue(objectName()+"/showGrid", showGrid_);
    MO::settings()->setValue(objectName()+"/showDome", showDome_);
    MO::settings()->setValue(objectName()+"/showRays", showRays_);
    MO::settings()->setValue(objectName()+"/showTex", showSliceTexture_);
    MO::settings()->setValue(objectName()+"/showHighlight", showHighlight_);

    delete glProps_;

    for (auto i : ptextureGeom_)
        i->releaseRef("DomePreviewWidget destroy");
    for (auto i : ptextureDrawable_)
        delete i;
    if (domeGeometry_)
        domeGeometry_->releaseRef("DomePreviewWidget destroy");
    if (projectorGeometry_)
        projectorGeometry_->releaseRef("DomePreviewWidget destroy");
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

void DomePreviewWidget::setShowHighlight(bool enable)
{
    if (enable == showHighlight_)
        return;

    showHighlight_ = enable;
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
    update();
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

void DomePreviewWidget::updateTextures()
{
    for (uint i=0; i<ptexture_.size(); ++i)
    {
        GL::Texture * tex = ptexture_[i];
        if (tex)
        {
            // put to release stack
            ptextureRelease_.push_back(tex);
            // flag for recreation
            ptexture_[i] = 0;
        }
    }

    update();
}

void DomePreviewWidget::updateTexture(uint index)
{
    if (index >= ptexture_.size())
        return;

    GL::Texture * tex = ptexture_[index];
    if (tex)
    {
        // put to release stack
        ptextureRelease_.push_back(tex);
        // flag for recreation
        ptexture_[index] = 0;
    }

    update();
}

void DomePreviewWidget::createDomeGeometry_()
{
    if (domeGeometry_)
        domeGeometry_->releaseRef("DomePreviewWidget create rel prev");
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
        i->releaseRef("DomePreviewWidget create rel prev");
    ptextureGeom_.clear();

    // move all textures to release-stack
    if (1)
    {
        for (auto t : ptexture_)
            ptextureRelease_.push_back(t);
        ptexture_.clear();
    }

    // build geometry
    if (projectorGeometry_)
        projectorGeometry_->releaseRef("DomePreviewWidget create del prev");
    projectorGeometry_ = new GEOM::Geometry();

    for (uint i=0; i<settings_->numProjectors(); ++i)
    {
        const bool highlight =
                showHighlight_ && (projIndex_ < 0 || projIndex_ == (int)i);
        //if (showCurrentCamera_ && !highlight)
        //    continue;

        mapper.setSettings(settings_->domeSettings(),
                           settings_->projectorSettings(i));
//        if (!mapper.isValid())
//            continue;

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

                Vec3 pos = mapper.getRayOrigin(x,-1);
                int v0 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                pos = mapper.mapToDome(x,-1);
                int v1 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                projectorGeometry_->addLine(v0, v1);

                pos = mapper.getRayOrigin(x,1);
                v0 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                pos = mapper.mapToDome(x,1);
                v1 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                projectorGeometry_->addLine(v0, v1);

                pos = mapper.getRayOrigin(-1,x);
                v0 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                pos = mapper.mapToDome(-1,x);
                v1 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                projectorGeometry_->addLine(v0, v1);

                pos = mapper.getRayOrigin(1,x);
                v0 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                pos = mapper.mapToDome(1,x);
                v1 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
                projectorGeometry_->addLine(v0, v1);
            }
        }

        // projected grid

        if (showProjectedSurface_)
        {
            if (highlight)
                projectorGeometry_->setColor(0.5,1.0,0.5,1);
            else
                if (showCurrentCamera_)
                    projectorGeometry_->setColor(0.1,0.13,0.1,1);
                else
                    projectorGeometry_->setColor(0.2,0.3,0.2,1);

            std::vector<GEOM::Geometry::IndexType> idx;
            const int num = 11;
            // create grid
            for (uint y = 0; y<num; ++y)
            for (uint x = 0; x<num; ++x)
            {
                const Vec3 pos = mapper.mapToDome(
                            (Float)x/(num-1)*2-1,
                            (Float)y/(num-1)*2-1);
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
                if (showCurrentCamera_)
                    g->setColor(0.15,0.15,0.15,1);
                else
                    g->setColor(0.5,0.5,0.5,1);

            std::vector<GEOM::Geometry::IndexType> idx;
            const int num = 11;
            // create grid
            for (uint y = 0; y<num; ++y)
            for (uint x = 0; x<num; ++x)
            {
                const Vec3 pos = mapper.mapToDome(
                            (Float)x/(num-1)*2-1,
                            (Float)y/(num-1)*2-1);

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
    glProps_->getProperties();
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

    if (projectorGeometry_)
        projectorGeometry_->releaseRef("DomePreviewWidget release");
    projectorGeometry_ = 0;

    // release textures
    for (auto i : ptexture_)
    {
        if (i->isHandle())
            i->release();
        delete i;
    }
    ptexture_.clear();
}

void DomePreviewWidget::prepareDrawGL()
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
            for (uint i=s; i<ptextureGeom_.size(); ++i)
            {
                ptextureDrawable_[i] = new GL::Drawable(QString("_tex%1").arg(i));
                GL::ShaderSource * src = new GL::ShaderSource();
                src->loadDefaultSource();
                src->addDefine("#define MO_ENABLE_TEXTURE");
                ptextureDrawable_[i]->setShaderSource(src);
            }
        }
        // or remove them
        else if (ptextureDrawable_.size() > ptextureGeom_.size())
        {
            int s = ptextureGeom_.size();
            for (uint i=s; i<ptextureDrawable_.size(); ++i)
            {
                if (ptextureDrawable_[i]->isReady())
                    ptextureDrawable_[i]->releaseOpenGl();
                delete ptextureDrawable_[i];
            }
            ptextureDrawable_.resize(s);
        }

        // create vaos
        for (uint i = 0; i < ptextureGeom_.size(); ++i)
        {
            ptextureDrawable_[i]->setGeometry(ptextureGeom_[i]);
            ptextureDrawable_[i]->createOpenGl();
        }

        ptextureGeom_.clear();
    }

    // create content textures if necessary
    if (showSliceTexture_)
    {
        if (ptexture_.size() < ptextureDrawable_.size())
        {
            int s = ptexture_.size();
            ptexture_.resize(ptextureDrawable_.size());
            for (uint i=s; i<ptextureDrawable_.size(); ++i)
                ptexture_[i] = 0;
        }
        // or remove them
        else if (ptexture_.size() > ptextureDrawable_.size())
        {
            int s = ptextureDrawable_.size();
            for (uint i=s; i<ptexture_.size(); ++i)
            {
                if (ptexture_[i]->isHandle())
                    ptexture_[i]->release();
                delete ptexture_[i];
            }
            ptexture_.resize(ptextureDrawable_.size());
        }
    }

    // release unused textures
    while (!ptextureRelease_.empty())
    {
        GL::Texture * tex = ptextureRelease_.back();
        ptextureRelease_.pop_back();
        if (tex->isAllocated())
            tex->release();
        delete tex;
    }

    // update content textures
    if (showSliceTexture_)
    for (uint i=0; i<ptexture_.size(); ++i)
    {
        if (!ptexture_[i])
            createTexture_(&ptexture_[i], i);
    }
}

void DomePreviewWidget::drawGL(const Mat4 &projection,
                               const Mat4 &cubeViewTrans,
                               const Mat4 &viewTrans,
                               const Mat4 &trans)
{
    // --- draw ---

    makeCurrent();
    // Mac OS X has a problem here with the implementation of glClear
    // http://stackoverflow.com/questions/9986826/glclear-fails-with-gl-framebuffer-undefined
    MO_CHECK_GL( gl::glClearColor(0, 0, 0, 1) );
#ifdef __APPLE__
    gl::glGetError();
#endif
    MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );
#ifdef __APPLE__
    gl::glGetError();
#endif
    MO_CHECK_GL( gl::glDisable(gl::GL_DEPTH_TEST) );
    MO_CHECK_GL( gl::glEnable(gl::GL_BLEND) );
    MO_CHECK_GL( gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE) );
    glProps_->setLineSmooth(true);
    glProps_->setLineWidth((GLfloat)fboSize().height() / 512);

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

    *tex = GL::Texture::createFromImage(qimg, gl::GL_RGB);

    //tex->create(320,200,gl::GL_RGB,gl::GL_FLOAT,0);
}


} // namespace GUI
} // namespace MO
