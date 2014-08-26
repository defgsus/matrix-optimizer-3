/** @file domepreviewwidget.cpp

    @brief Display of dome and projectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include "domepreviewwidget.h"
#include "projection/domesettings.h"
#include "geom/geometry.h"
#include "geom/geometryfactory.h"
#include "gl/drawable.h"

namespace MO {
namespace GUI {

DomePreviewWidget::DomePreviewWidget(QWidget *parent)
    : Basic3DWidget (Basic3DWidget::RM_DIRECT, parent),
      domeSettings_ (new DomeSettings()),
      domeGeometry_ (0),
      showGrid_     (true)

{
    setObjectName("_DomePreviewWidget");

    createDomeGeometry_();
}

DomePreviewWidget::~DomePreviewWidget()
{
    delete domeSettings_;
    delete domeGeometry_;
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

    GEOM::GeometryFactory::createDomeLines(domeGeometry_,
                                           domeSettings_->radius(),
                                           domeSettings_->coverage(),
                                           24, 12);

    Mat4 trans(1.0);

    if (domeSettings_->tiltX() < 0)
    {
        trans = glm::translate(trans, Vec3(0,0,-domeSettings_->radius()));
        trans = glm::rotate(trans, domeSettings_->tiltX(), Vec3(1, 0, 0));
    }
    else if (domeSettings_->tiltX() > 0)
    {
        trans = glm::translate(trans, Vec3(0,0,domeSettings_->radius()));
        trans = glm::rotate(trans, domeSettings_->tiltX(), Vec3(1, 0, 0));
    }

    domeGeometry_->applyMatrix(trans);

    if (domeGeometry_->numTriangles())
        domeGeometry_->calculateTriangleNormals();
}

void DomePreviewWidget::initGL()
{
    domeDrawable_ = new GL::Drawable(objectName());

}

void DomePreviewWidget::releaseGL()
{
    if (domeDrawable_->isReady())
        domeDrawable_->releaseOpenGl();
    delete domeDrawable_;
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

    if (domeDrawable_->isReady())
        domeDrawable_->renderShader(projection, cubeViewTrans, viewTrans, trans);

    if (showGrid_)
        drawGrid(projection, cubeViewTrans, viewTrans, trans);
}

} // namespace GUI
} // namespace MO
