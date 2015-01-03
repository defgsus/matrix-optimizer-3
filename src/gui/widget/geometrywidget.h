/** @file geometrywidget.h

    @brief Viewer for Geometry/Drawable

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_GEOMETRYWIDGET_H
#define MOSRC_GUI_WIDGET_GEOMETRYWIDGET_H

#include "basic3dwidget.h"
#include "gl/opengl_fwd.h"

namespace MO {
namespace GL { class LightSettings; }
namespace GUI {


class GeometryWidget : public Basic3DWidget
{
    Q_OBJECT
public:

    explicit GeometryWidget(RenderMode mode, QWidget *parent = 0);
    ~GeometryWidget();

    bool isShowGrid() const { return showGrid_; }
    bool isShowTexture() const { return showTexture_; }
    bool isShowNormalMap() const { return showNormalMap_; }
    bool isShowLights() const { return showLights_; }
    int pointsize() const { return pointsize_; }

    // access to light settings
    //GL::LightSettings & lightSettings() { return *lights_; }

signals:

public slots:

    void setGeometry(GEOM::Geometry *);

    void setShowGrid(bool enable) { showGrid_ = enable; update(); }
    void setShowTexture(bool enable) { showTexture_ = enable; update(); }
    void setShowNormalMap(bool enable) { showNormalMap_ = enable; update(); }
    void setShowLights(bool enable) { setLights_(enable? 0.7:0); update(); }
    void setPointsize(int size) { pointsize_ = size; update(); }

protected:

    void setLights_(Float amp);

    void initGL() Q_DECL_OVERRIDE;

    void releaseGL() Q_DECL_OVERRIDE;

    void drawGL(const Mat4& projection,
                const Mat4& cubeViewTrans,
                const Mat4& viewTrans,
                const Mat4& trans) Q_DECL_OVERRIDE;

    GL::Properties * glprops_;
    GL::Drawable * drawable_;
    GL::Texture * tex_, * texNorm_;
    GL::LightSettings * lights_;
    bool showGrid_, showTexture_, showNormalMap_, showLights_;
    int pointsize_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_GEOMETRYWIDGET_H
