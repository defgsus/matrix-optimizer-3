/***************************************************************************

Copyright (C) 2014  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#include <glm/gtx/polar_coordinates.hpp>
#include "projectorsetupview.h"

ProjectorSetupView::ProjectorSetupView(QWidget *parent) :
    Basic3DView(parent)
{
    calcDomeVerts_(10, 180);
}



void ProjectorSetupView::paintGL()
{
    Basic3DView::paintGL();


}



void ProjectorSetupView::calcDomeVerts_(MO::Float rad, MO::Float arc)
{
    domevert_.clear();

    for (MO::Float j=10; j<=arc; j+=10)
    for (int i=0; i<360; i+=10)
    {
        MO::Vec3
                v1 = rad * MO::pointOnSphere((MO::Float)i/360, (MO::Float)j/360);
                //glm::polar(MO::Vec3(rad, i, j));
        domevert_.push_back(v1[0]);
        domevert_.push_back(v1[1]);
        domevert_.push_back(v1[2]);
    }
}
