/** @file geometrywidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include "geometrywidget.h"
#include "gl/vertexarrayobject.h"
#include "gl/geometry.h"
#include "gl/drawable.h"

namespace MO {
namespace GUI {


GeometryWidget::GeometryWidget(QWidget *parent) :
    Basic3DWidget   (parent),
    drawable_       (new GL::Drawable())
{

}

GeometryWidget::~GeometryWidget()
{
    delete drawable_;
}

void GeometryWidget::initializeGL()
{
    drawable_->createOpenGl();
}

void GeometryWidget::setGeometry(GL::Geometry * g)
{
    drawable_->setGeometry(g);
}

void GeometryWidget::paintGL()
{
    glClearColor(0.1, 0.2, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!drawable_->isReady())
        return;

    drawable_->renderShader(projectionMatrix(), transformationMatrix());
}


} // namespace GUI
} // namespace MO
