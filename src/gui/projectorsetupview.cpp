/** @file

    @brief view of a dome with projectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/
#include <glm/gtx/polar_coordinates.hpp>

#include "math/vector.h"
#include "projectorsetupview.h"

namespace MO {
namespace GUI {

ProjectorSetupView::ProjectorSetupView(QWidget *parent) :
    Basic3DView(parent)
{
    calcDomeVerts_(10, 180);
    viewInit(20);
}



void ProjectorSetupView::paintGL()
{
    Basic3DView::paintGL();

    drawCoords_(10);

    glColor3f(0.7,0.7,0.7);
    glBegin(GL_LINES);
    for (size_t i=0; i<domevert_.size(); i+=3)
    {
        glVertex3f(domevert_[i], domevert_[i+1], domevert_[i+2]);
    }
    glEnd();
}



void ProjectorSetupView::calcDomeVerts_(MO::Float rad, MO::Float arc)
{
    domevert_.clear();

    for (int j=10; j<=arc; j+=10)
    for (int i=0; i<360; i+=10)
    {
        MO::Vec3
            v1 = rad * MATH::pointOnSphere((MO::Float)i/360, (MO::Float)j/360),
            v2 = rad * MATH::pointOnSphere((MO::Float)(i+10)/360, (MO::Float)j/360),
            v3 = rad * MATH::pointOnSphere((MO::Float)i/360, (MO::Float)(j-10)/360);

        domevert_.push_back(v1[0]);
        domevert_.push_back(v1[1]);
        domevert_.push_back(v1[2]);
        domevert_.push_back(v2[0]);
        domevert_.push_back(v2[1]);
        domevert_.push_back(v2[2]);
        domevert_.push_back(v1[0]);
        domevert_.push_back(v1[1]);
        domevert_.push_back(v1[2]);
        domevert_.push_back(v3[0]);
        domevert_.push_back(v3[1]);
        domevert_.push_back(v3[2]);
    }
}

} // namespace GUI
} // namespace MO
