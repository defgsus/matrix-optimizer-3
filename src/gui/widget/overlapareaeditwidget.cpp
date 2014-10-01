/** @file overlapareaeditwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.09.2014</p>
*/

#include "overlapareaeditwidget.h"
#include "projection/projectorsettings.h"
#include "projection/projectormapper.h"
#include "geom/geometry.h"
#include "geom/tesselator.h"
#include "gl/drawable.h"

namespace MO {
namespace GUI {


OverlapAreaEditWidget::OverlapAreaEditWidget(QWidget *parent)
    : Basic3DWidget (RM_DIRECT, parent),
      projector_    (new ProjectorSettings()),
      dome_         (new DomeSettings()),
      triangleGeom_ (0),
      lineGeom_     (0),
      blendGeom_    (0),
      triangles_    (0),
      lines_        (0),
      blends_       (0),
      showTesselation_(true),
      showBlend_    (true)
{
    setObjectName("_OverlapAreaEditWidget");

    createGeometry_();

    viewSet(VD_FRONT, 1);
    viewSetOrthoScale(1);
}

OverlapAreaEditWidget::~OverlapAreaEditWidget()
{
    delete blends_;
    delete lines_;
    delete triangles_;
    delete blendGeom_;
    delete lineGeom_;
    delete triangleGeom_;
    delete dome_;
    delete projector_;
}

void OverlapAreaEditWidget::setSettings(const DomeSettings& d, const ProjectorSettings& p)
{
    *projector_ = p;
    *dome_ = d;

    updateFboSize_();

    createGeometry_();

    update();
}

void OverlapAreaEditWidget::resizeGL(int w, int h)
{
    Basic3DWidget::resizeGL(w, h);
    updateFboSize_();
}

void OverlapAreaEditWidget::updateFboSize_()
{
    Float aspect = (Float)projector_->width() / projector_->height();
    setFboSize(height() * aspect, height());
}

void OverlapAreaEditWidget::createGeometry_()
{
    GEOM::Tesselator tess;

    ProjectorMapper mapper;
    mapper.setSettings(*dome_, *projector_);

    lineGeom_ = new GEOM::Geometry();

    // -- outline --
    lineGeom_->setColor(0.3,0.3,0.3,1);
    const GEOM::Geometry::IndexType
            p1 = lineGeom_->addVertex(0,0,0),
            p2 = lineGeom_->addVertex(1,0,0),
            p3 = lineGeom_->addVertex(1,1,0),
            p4 = lineGeom_->addVertex(0,1,0);
    lineGeom_->addLine(p1,p2);
    lineGeom_->addLine(p2,p3);
    lineGeom_->addLine(p3,p4);
    lineGeom_->addLine(p4,p1);

    // -- overlap area --

    for (uint idx = 0; idx < projector_->numOverlapAreas(); ++idx)
    {
        lineGeom_->setColor(0.5,1,0.5,1);

        const QVector<Vec2> & area = projector_->overlapArea(idx);

        if (!showTesselation_)
        {
            // draw outline
            GEOM::Geometry::IndexType p1=0, p2;
            for (int i=0; i<=area.size(); ++i)
            {
                p2 = lineGeom_->addVertex(area[i%area.size()][0], area[i%area.size()][1], 0);
                if (i>0)
                    lineGeom_->addLine(p1, p2);
                p1 = p2;
            }
        }
        else
        {
            tess.tesselate(area);
            tess.getGeometry(*lineGeom_, false);
        }

        if (showBlend_)
        {
            blendGeom_ = new GEOM::Geometry();
            blendGeom_->clear();
            mapper.getBlendGeometry(area, blendGeom_);
        }
    }
}

void OverlapAreaEditWidget::initGL()
{
    triangles_ = new GL::Drawable(objectName() + "_triangles");
    lines_ = new GL::Drawable(objectName() + "_lines");
    blends_ = new GL::Drawable(objectName() + "_blends");
}

void OverlapAreaEditWidget::releaseGL()
{
    if (triangles_ && triangles_->isReady())
        triangles_->releaseOpenGl();
    if (lines_ && lines_->isReady())
        lines_->releaseOpenGl();
    if (blends_ && blends_->isReady())
        blends_->releaseOpenGl();
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

    if (blendGeom_)
    {
        blends_->setGeometry(blendGeom_);
        blendGeom_ = 0;

        blends_->createOpenGl();
    }

    using namespace gl;

    MO_CHECK_GL( gl::glClearColor(0,0,0,1) );
    MO_CHECK_GL( gl::glClear(GL_COLOR_BUFFER_BIT) );
    MO_CHECK_GL( gl::glDisable(GL_DEPTH_TEST) );
    MO_CHECK_GL( gl::glEnable(GL_BLEND) );

    if (blends_->isReady())
    {
        MO_CHECK_GL( gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE) );

        blends_->renderShader(projection, cubeViewTrans, viewTrans, trans);

        MO_CHECK_GL( gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
    }

    if (triangles_->isReady())
        triangles_->renderShader(projection, cubeViewTrans, viewTrans, trans);

    if (lines_->isReady())
        lines_->renderShader(projection, cubeViewTrans, viewTrans, trans);
}

} // namespace GUI
} // namespace MO
