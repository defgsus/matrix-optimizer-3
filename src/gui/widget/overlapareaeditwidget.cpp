/** @file overlapareaeditwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.09.2014</p>
*/

#include "overlapareaeditwidget.h"
#include "projection/projectorsettings.h"
#include "geom/geometry.h"
#include "geom/tesselator.h"
#include "gl/drawable.h"

namespace MO {
namespace GUI {


OverlapAreaEditWidget::OverlapAreaEditWidget(QWidget *parent)
    : Basic3DWidget (RM_DIRECT_ORTHO, parent),
      projector_    (new ProjectorSettings()),
      triangleGeom_ (0),
      lineGeom_     (0),
      triangles_    (0),
      lines_        (0)
{
    setObjectName("_OverlapAreaEditWidget");

    viewSet(VD_FRONT, 1);
}

OverlapAreaEditWidget::~OverlapAreaEditWidget()
{
    delete lines_;
    delete triangles_;
    delete lineGeom_;
    delete triangleGeom_;
    delete projector_;
}

void OverlapAreaEditWidget::setProjector(const ProjectorSettings& p)
{
    *projector_ = p;

    createGeometry_();

    update();
}

void OverlapAreaEditWidget::createGeometry_()
{
    lineGeom_ = new GEOM::Geometry();

    const GEOM::Geometry::IndexType
            p1 = lineGeom_->addVertex(0,0,0),
            p2 = lineGeom_->addVertex(1,0,0),
            p3 = lineGeom_->addVertex(1,1,0),
            p4 = lineGeom_->addVertex(0,1,0);
    lineGeom_->addLine(p1,p2);
    lineGeom_->addLine(p2,p3);
    lineGeom_->addLine(p3,p4);
    lineGeom_->addLine(p4,p1);
}

void OverlapAreaEditWidget::initGL()
{
    triangles_ = new GL::Drawable(objectName() + "_triangles");
    lines_ = new GL::Drawable(objectName() + "_lines");
}

void OverlapAreaEditWidget::releaseGL()
{
    if (triangles_ && triangles_->isReady())
        triangles_->releaseOpenGl();
    if (lines_ && lines_->isReady())
        lines_->releaseOpenGl();
}

void OverlapAreaEditWidget::drawGL(const Mat4& projection,
                                   const Mat4& cubeViewTrans,
                                   const Mat4& viewTrans,
                                   const Mat4& trans)
{
    if (lineGeom_)
    {
        lines_->setGeometry(lineGeom_);
        lineGeom_ = 0;

        lines_->createOpenGl();
    }

    if (triangleGeom_)
    {
        triangles_->setGeometry(triangleGeom_);
        triangleGeom_ = 0;

        triangles_->createOpenGl();
    }

    using namespace gl;

    MO_CHECK_GL( gl::glClear(GL_COLOR_BUFFER_BIT) );
    MO_CHECK_GL( gl::glDisable(GL_DEPTH_TEST) );

    if (triangles_->isReady())
        triangles_->renderShader(projection, cubeViewTrans, viewTrans, trans);

    if (lines_->isReady())
        lines_->renderShader(projection, cubeViewTrans, viewTrans, trans);
}

} // namespace GUI
} // namespace MO
