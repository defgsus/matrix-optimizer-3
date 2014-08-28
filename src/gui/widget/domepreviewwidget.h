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
class ProjectorSettings;
class ProjectionSystemSettings;

namespace GUI {


class DomePreviewWidget : public Basic3DWidget
{
    Q_OBJECT
public:
    explicit DomePreviewWidget(QWidget *parent = 0);
    ~DomePreviewWidget();

    bool getShowDome() const { return showDome_; }
    bool getShowGrid() const { return showGrid_; }
    bool getShowRays() const { return showRays_; }
    bool getShowCurrentCamera() const { return showCurrentCamera_; }

    /** Sets the view matrix when in CM_SET mode */
    void setViewMatrix(const Mat4&) Q_DECL_OVERRIDE;

signals:

public slots:

    void setShowDome(bool enable) { showDome_ = enable; update(); }
    void setShowGrid(bool enable) { showGrid_ = enable; update(); }
    void setShowRays(bool enable);
    void setShowCurrentCamera(bool enable);

    /** Updates the settings and graphical display.
        If @p currentProjectorIndex >= 0, then this projector will be highlighted. */
    void setProjectionSettings(const ProjectionSystemSettings&, int currentProjectorIndex = -1);

protected:

    void initGL() Q_DECL_OVERRIDE;
    void releaseGL() Q_DECL_OVERRIDE;

    void drawGL(const Mat4& projection,
                const Mat4& cubeViewTrans,
                const Mat4& viewTrans,
                const Mat4& trans) Q_DECL_OVERRIDE;

private:

    void createDomeGeometry_();
    void createProjectorGeometry_();
    void setCurrentCameraMatrix_();

    ProjectionSystemSettings * settings_;

    GEOM::Geometry * domeGeometry_;
    GEOM::Geometry * projectorGeometry_;
    GL::Drawable * domeDrawable_;
    GL::Drawable * projectorDrawable_;

    Mat4 domeTransform_;

    bool showGrid_,
         showDome_,
         showRays_,
         showCurrentCamera_,
         showProjectedSurface_;

    int projIndex_;

    RenderMode lastRenderMode_;
    CameraMode lastCameraMode_;
    QSize lastFboSize_;
};

} // namespace GUI
} // namespace MO

#endif // DOMEPREVIEWWIDGET_H
