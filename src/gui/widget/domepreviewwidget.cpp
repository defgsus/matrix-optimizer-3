/** @file domepreviewwidget.cpp

    @brief Display of dome and projectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include "domepreviewwidget.h"

namespace MO {
namespace GUI {

DomePreviewWidget::DomePreviewWidget(QWidget *parent)
    : Basic3DWidget (Basic3DWidget::RM_DIRECT, parent),
      showGrid_     (true)
{
}

void DomePreviewWidget::initGL()
{

}

void DomePreviewWidget::releaseGL()
{

}

void DomePreviewWidget::drawGL(const Mat4 &projection,
                               const Mat4 &cubeViewTrans,
                               const Mat4 &viewTrans,
                               const Mat4 &trans)
{
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );


    if (showGrid_)
        drawGrid(projection, cubeViewTrans, viewTrans, trans);
}

} // namespace GUI
} // namespace MO
