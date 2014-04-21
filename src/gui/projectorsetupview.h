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

#ifndef MOSRC_GUI_PROJECTORSETUPVIEW_H
#define MOSRC_GUI_PROJECTORSETUPVIEW_H

#include <vector>
#include "gui/basic3dview.h"

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

#endif // MOSRC_GUI_PROJECTORSETUPVIEW_H
