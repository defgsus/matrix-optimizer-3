/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#ifndef MSRC_GUI_WIDGET_CSGRENDERWIDGET_H
#define MSRC_GUI_WIDGET_CSGRENDERWIDGET_H

#include "Basic3dWidget.h"

namespace MO {
class CsgRoot;
class Properties;
namespace GL { class CsgShader; }
namespace GUI {


class CsgRenderWidget : public Basic3DWidget
{
    Q_OBJECT
public:
    explicit CsgRenderWidget(QWidget *parent = 0);
    ~CsgRenderWidget();

    const GL::CsgShader& shader() const { return *p_shader_; }
    const Properties& shaderProperties() const;

    void setShaderProperties(const Properties&);
    void setRootObject(CsgRoot*);

signals:

public slots:

protected:

    void releaseGL() Q_DECL_OVERRIDE;
    void drawGL(const Mat4& projection,
                const Mat4& cubeViewTrans,
                const Mat4& viewtrans,
                const Mat4& trans) Q_DECL_OVERRIDE;

private:
    GL::CsgShader * p_shader_;
};

} // namespace GUI
} // namespace MO

#endif // MSRC_GUI_WIDGET_CSGRENDERWIDGET_H
