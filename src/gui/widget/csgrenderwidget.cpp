/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#include "csgrenderwidget.h"
#include "gl/csgshader.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {

CsgRenderWidget::CsgRenderWidget(QWidget *parent)
    : Basic3DWidget (RM_FRAMEBUFFER, parent)
    , p_shader_     (new GL::CsgShader)
{
    setCameraMode(CM_FREE_INVERSE);

    setMinimumSize(320, 320);
}

CsgRenderWidget::~CsgRenderWidget()
{
    delete p_shader_;
}

const Properties& CsgRenderWidget::shaderProperties() const
{
    return p_shader_->properties();
}

void CsgRenderWidget::setRootObject(CsgRoot* r)
{
    p_shader_->setRootObject(r);
    update();
}

void CsgRenderWidget::setShaderProperties(const Properties& p)
{
    p_shader_->setProperties(p);
    update();
}

void CsgRenderWidget::releaseGL()
{
    p_shader_->releaseGl();
}

void CsgRenderWidget::drawGL(const Mat4 &projection,
                             const Mat4 &cubeViewTrans,
                             const Mat4 &viewtrans,
                             const Mat4 &trans)
{
    try
    {
        gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

        p_shader_->render(size(), projection, cubeViewTrans);
    }
    catch (const Exception& e)
    {
        MO_GL_WARNING("Failed in CsgRenderWidget::draw:\n" << e.what());
    }
}


} // namespace GUI
} // namespace MO
