/** @file enumnames.h

    @brief strings for enums in Qt

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/9/2014</p>
*/

#ifndef MOSRC_TOOL_ENUMNAMES_H
#define MOSRC_TOOL_ENUMNAMES_H

#include <Qt>
#include <QString>
#include <QMap>

namespace MO {

/** Returns the name for a modifier */
QString enumName(Qt::Modifier);

QString enumName(Qt::Key);

const QMap<int, QString>& keycodeNames();

} // namespace MO

#endif // MOSRC_TOOL_ENUMNAMES_H
