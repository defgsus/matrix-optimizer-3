/** @file domepreviewwidget.cpp

    @brief Display of dome and projectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

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
      showGrid_     (true)

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
    mapper.setSettings(*projectorSettings_);
    if (!mapper.isValid())
        return;

    // build geometry
    projectorGeometry_ = new GEOM::Geometry();
    projectorGeometry_->setColor(1,0.5,0.5,1);

    Vec3 pos = mapper.pos();
    const int v0 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
    pos = mapper.mapToDome(0,0, *domeSettings_);
    const int v1 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
    pos = mapper.mapToDome(1,0, *domeSettings_);
    const int v2 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
    pos = mapper.mapToDome(1,1, *domeSettings_);
    const int v3 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);
    pos = mapper.mapToDome(0,1, *domeSettings_);
    const int v4 = projectorGeometry_->addVertex(pos[0], pos[1], pos[2]);

    projectorGeometry_->addLine(v0, v1);
    projectorGeometry_->addLine(v0, v2);
    projectorGeometry_->addLine(v0, v3);
    projectorGeometry_->addLine(v0, v4);
    projectorGeometry_->addLine(v1, v2);
    projectorGeometry_->addLine(v2, v3);
    projectorGeometry_->addLine(v3, v4);
    projectorGeometry_->addLine(v4, v1);

    //    Mat4 trans(mapper.getTransformationMatrix());
    //projectorGeometry_->applyMatrix(trans);

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
