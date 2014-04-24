/** @file

    @brief view of a dome with projectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_GUI_PROJECTORSETUPVIEW_H
#define MOSRC_GUI_PROJECTORSETUPVIEW_H

#include <vector>
#include "gui/basic3dview.h"

namespace MO {
namespace GUI {

class ProjectorSetupView : public Basic3DView
{
    Q_OBJECT
public:
    explicit ProjectorSetupView(QWidget * parent = 0);

    void setDome(MO::Float rad, MO::Float arc_degree)
        { calcDomeVerts_(rad, arc_degree); updateGL(); }

signals:

public slots:

protected:
    void paintGL();

private:
    void calcDomeVerts_(MO::Float rad, MO::Float arc);

    std::vector<MO::Float> domevert_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_PROJECTORSETUPVIEW_H
