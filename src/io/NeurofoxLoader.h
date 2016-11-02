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

#ifndef MOSRC_IO_NEUROFOXLOADER_H
#define MOSRC_IO_NEUROFOXLOADER_H

#include <QString>
#include "types/float.h"

namespace MO {

namespace MATH { class Timeline1d; }

class NeurofoxLoader
{
public:
    enum Format
    {
        F_FLOAT,
        F_HEX
    };

    struct DataPoint
    {
        Double time,
            v1, v2;
    };

    NeurofoxLoader();
    ~NeurofoxLoader();

    void loadFile(const QString& filename, Format f);

    void getTimeline1d(MATH::Timeline1d* tl) const;

private:
    struct Private;
    Private* p_;
};

} // namespace MO

#endif // MOSRC_IO_NEUROFOXLOADER_H
