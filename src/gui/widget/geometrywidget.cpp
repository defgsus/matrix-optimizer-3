/** @file geometrywidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include "geometrywidget.h"
#include "gl/vertexarrayobject.h"
#include "gl/drawable.h"
#include "geom/geometry.h"

namespace MO {
namespace GUI {


GeometryWidget::GeometryWidget(RenderMode mode, QWidget *parent) :
    Basic3DWidget   (mode, parent),
    drawable_       (new GL::Drawable("geomwidget"))
{
    setMinimumSize(128, 128);

}

GeometryWidget::~GeometryWidget()
{
    if (drawable_->isReady())
        drawable_->releaseOpenGl();
    delete drawable_;
}
/*
void GeometryWidget::initializeGL()
{

}
*/
void GeometryWidget::setGeometry(GEOM::Geometry * g)
{
    drawable_->setGeometry(g);
    drawable_->createOpenGl();
    updateGL();
}

void GeometryWidget::drawGL(const Mat4 &projection, const Mat4 &transformation)
{
    MO_CHECK_GL( glClearColor(0.1, 0.2, 0.3, 1.0) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
    // XXX not working in RM_DIRECT
    MO_CHECK_GL( glEnable(GL_BLEND) );
    MO_CHECK_GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );

    if (drawable_->isReady())
        drawable_->renderShader(projection, transformation);
}


} // namespace GUI
} // namespace MO
