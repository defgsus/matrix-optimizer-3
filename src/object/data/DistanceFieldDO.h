/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

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

#ifndef MOSRC_OBJECT_DATA_DISTANCEFIELDDO_H
#define MOSRC_OBJECT_DATA_DISTANCEFIELDDO_H

#include "object/Object.h"
#include "object/interface/ValueFloatMatrixInterface.h"

namespace MO {

/** Calculates the distance to a implicit surface in a FloatMatrix */
class DistanceFieldDO
                : public Object
                , public ValueFloatMatrixInterface
{
public:
    MO_OBJECT_CONSTRUCTOR(DistanceFieldDO);

    enum Inside
    {
        IN_ABOVE,
        IN_BELOW
    };

    virtual Type type() const Q_DECL_OVERRIDE { return T_CONTROL; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter* p) Q_DECL_OVERRIDE;

    virtual FloatMatrix valueFloatMatrix(
            uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;
    virtual bool hasFloatMatrixChanged(
            uint channel, const RenderTime &time) const Q_DECL_OVERRIDE;

private:

    struct Private;
    Private* p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_DATA_DISTANCEFIELDDO_H
