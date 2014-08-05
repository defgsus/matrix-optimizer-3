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
namespace GUI {


class GeometryWidget : public Basic3DWidget
{
    Q_OBJECT
public:

    explicit GeometryWidget(RenderMode mode, QWidget *parent = 0);
    ~GeometryWidget();

    bool isShowGrid() const { return showGrid_; }
    bool isShowTexture() const { return showTexture_; }

signals:

public slots:

    void setGeometry(GEOM::Geometry *);

    void setShowGrid(bool enable) { showGrid_ = enable; update(); }
    void setShowTexture(bool enable) { showTexture_ = enable; update(); }

protected:

    void drawGL(const Mat4 &projection, const Mat4 &transformation) Q_DECL_OVERRIDE;

    GL::Drawable * drawable_;
    GL::Texture * tex_;

    bool showGrid_, showTexture_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_GEOMETRYWIDGET_H
