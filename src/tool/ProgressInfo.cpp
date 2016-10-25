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

#include "ProgressInfo.h"
#include "object/Object.h"
#include "io/ApplicationTime.h"

namespace MO {

double ProgressInfo::percent() const
{
    return numItems() ? double(curItem()) / double(numItems()) * 100.
                      : 0.;
}

QString ProgressInfo::toString() const
{
    QString s = taskName();
    if (numItems())
        s += QString(" %1 (%2/%3)")
                .arg(percent())
                .arg(curItem())
                .arg(numItems());
    if (isFinished())
        s += " finished";
    return s;
}

void ProgressInfo::send(double interval)
{
    if (interval > 0. && !isFinished())
    {
        Double t = applicationTime();
        if (p_sendTime > 0. && t < interval + p_sendTime)
            return;
        p_sendTime = t;
    }

    if (object())
        object()->emitProgress(*this);

    qApp->flush();
}


} // namespace MO
