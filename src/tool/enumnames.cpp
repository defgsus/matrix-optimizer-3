/** @file enumnames.cpp

    @brief strings for enums in Qt

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/9/2014</p>
*/

#include <QApplication>
#include "enumnames.h"

namespace MO {

QString enumName(Qt::Modifier m)
{
    switch (m)
    {
    case Qt::SHIFT: return QApplication::tr("Shift");
    case Qt::META: return QApplication::tr("Meta");
    case Qt::CTRL: return QApplication::tr("Ctrl");
    case Qt::ALT: return QApplication::tr("Alt");
    default: return QApplication::tr("*unknown*");
    }
}


} // namespace MO
