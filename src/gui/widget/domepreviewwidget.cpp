/** @file domepreviewwidget.cpp

    @brief Display of dome and projectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include <vector>

#include "domepreviewwidget.h"
#include "projection/domesettings.h"
#include "projection/projectorsettings.h"
#include "projection/projectormapper.h"
#include "geom/geometry.h"
#include "geom/geometryfactory.h"
#include "gl/drawable.h"

namespace MO {
namespace GUI {

DomePreviewWidget::DomePreviewWidget(QWidget *parent)
    : Basic3DWidget (Basic3DWidget::RM_DIRECT, parent),
      domeSettings_ (new DomeSettings()),
      projectorSettings_(new ProjectorSettings()),
      domeGeometry_ (0),
      projectorGeometry_(0),
      showGrid_     (true),
      showRays_     (true),
      showProjectedSurface_(true)

{
    setObjectName("_DomePreviewWidget");

    createDomeGeometry_();
    createProjectorGeometry_();
}

DomePreviewWidget::~DomePreviewWidget()
{
    delete domeSettings_;
    delete domeGeometry_;
    delete projectorSettings_;
    delete projectorGeometry_;
}

void DomePreviewWidget::setDomeSettings(const DomeSettings & s)
{
    *domeSettings_ = s;
    createDomeGeometry_();
    createProjectorGeometry_();
    update();
}

void DomePreviewWidget::createDomeGeometry_()
{
    domeGeometry_ = new GEOM::Geometry();
    domeGeometry_->setSharedVertices(false);
    domeGeometry_->setColor(0.5,0.5,0.5,1.0);

    GEOM::GeometryFactory::createDomeLines(domeGeometry_,
                                           domeSettings_->radius(),
                                           domeSettings_->coverage(),
                                           24, 12);
    Mat4 trans(1.0);

    if (domeSettings_->tiltX() != 0)
    {
        trans = glm::rotate(trans, domeSettings_->tiltX(), Vec3(1, 0, 0));
    }
    if (domeSettings_->tiltZ() != 0)
    {
        trans = glm::rotate(trans, domeSettings_->tiltZ(), Vec3(0, 0, 1));
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

void DomePreviewWidget::setProjectorSettings(const ProjectorSettings & s)
{
    *projectorSettings_ = s;
    createProjectorGeometry_();
    update();
}

void DomePreviewWidget::createProjectorGeometry_()
{
    ProjectorMapper mapper;
    mapper.setSettings(*domeSettings_, *projectorSettings_);
    if (!mapper.isValid())
        return;

    // build geometry
    projectorGeometry_ = new GEOM::Geometry();

    if (showRays_)
    {
        projectorGeometry_->setColor(0.7,0.7,0.5,1);

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
        projectorGeometry_->setColor(0.5,1.0,0.5,1);
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

    projectorGeometry_->applyMatrix(domeTransform_);
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
}

void DomePreviewWidget::drawGL(const Mat4 &projection,
                               const Mat4 &cubeViewTrans,
                               const Mat4 &viewTrans,
                               const Mat4 &trans)
{
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

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

    if (domeDrawable_->isReady())
        domeDrawable_->renderShader(projection, cubeViewTrans, viewTrans, trans);

    if (projectorDrawable_->isReady())
        projectorDrawable_->renderShader(projection, cubeViewTrans, viewTrans, trans);

    if (showGrid_)
        drawGrid(projection, cubeViewTrans, viewTrans, trans);
}

} // namespace GUI
} // namespace MO
