/** @file domepreviewwidget.h

    @brief Display of dome and projectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#ifndef DOMEPREVIEWWIDGET_H
#define DOMEPREVIEWWIDGET_H

#include "basic3dwidget.h"
#include "gl/opengl_fwd.h"

namespace MO {

class DomeSettings;

namespace GUI {


class DomePreviewWidget : public Basic3DWidget
{
    Q_OBJECT
public:
    explicit DomePreviewWidget(QWidget *parent = 0);
    ~DomePreviewWidget();

signals:

public slots:

    /** Updates the dome settings and the graphic */
    void setDomeSettings(const DomeSettings&);

protected:

    void initGL() Q_DECL_OVERRIDE;
    void releaseGL() Q_DECL_OVERRIDE;

    void drawGL(const Mat4& projection,
                const Mat4& cubeViewTrans,
                const Mat4& viewTrans,
                const Mat4& trans) Q_DECL_OVERRIDE;

private:

    void createDomeGeometry_();

    DomeSettings * domeSettings_;

    GEOM::Geometry * domeGeometry_;
    GL::Drawable * domeDrawable_;
    //GL::Texture * tex_;
    //GL::LightSettings * lights_;
    bool showGrid_;


};

} // namespace GUI
} // namespace MO

#endif // DOMEPREVIEWWIDGET_H
