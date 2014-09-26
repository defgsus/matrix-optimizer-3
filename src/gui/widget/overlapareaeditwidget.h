/** @file overlapareaeditwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.09.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_OVERLAPAREAEDITWIDGET_H
#define MOSRC_GUI_WIDGET_OVERLAPAREAEDITWIDGET_H

#include "basic3dwidget.h"
#include "gl/opengl_fwd.h"

namespace MO {

class ProjectorSettings;

namespace GUI {


class OverlapAreaEditWidget : public Basic3DWidget
{
    Q_OBJECT
public:
    explicit OverlapAreaEditWidget(QWidget *parent = 0);
    ~OverlapAreaEditWidget();

signals:

public slots:

    /** Updates the settings and graphical display. */
    void setProjector(const ProjectorSettings&);

protected:

    void resizeGL(int w, int h);

    void initGL() Q_DECL_OVERRIDE;
    void releaseGL() Q_DECL_OVERRIDE;

    void drawGL(const Mat4& projection,
                const Mat4& cubeViewTrans,
                const Mat4& viewTrans,
                const Mat4& trans) Q_DECL_OVERRIDE;

private:

    void updateFboSize_();
    void createGeometry_();

    ProjectorSettings * projector_;

    GEOM::Geometry * triangleGeom_, *lineGeom_;
    GL::Drawable * triangles_, *lines_;

    bool showTesselation_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_OVERLAPAREAEDITWIDGET_H
