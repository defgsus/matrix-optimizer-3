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

#ifndef MOSRC_TOOL_PROGRESSINFO_H
#define MOSRC_TOOL_PROGRESSINFO_H

#include <QString>
#include <QMetaType>

namespace MO {

class Object;

class ProgressInfo
{
public:
    ProgressInfo()
        : p_object(nullptr)
        , p_curItem(0), p_numItems(0), p_finished(false)
        , p_sendTime(-1.)
    { }

    ProgressInfo(const QString& taskName, Object* o)
        : p_object(o), p_taskName(taskName)
        , p_curItem(0), p_numItems(0), p_finished(false)
        , p_sendTime(-1.)
    { }

    // ----- getter ------

    const QString& taskName() const { return p_taskName; }
    Object* object() const { return p_object; }

    bool isFinished() const { return p_finished; }
    int curItem() const { return p_curItem; }
    int numItems() const { return p_numItems; }

    double percent() const;

    QString toString() const;

    // ---- setter ----

    void setProgress(int curItem) { p_curItem = curItem; }
    void setNumItems(int numItems) { p_numItems = numItems; }

    void setFinished() { p_finished = true; }

    /** Sends this class via ObjectEditor.
        Multiple send() calls will be ignored until @p intervalSec seconds
        have passed. */
    void send(double intervalSec = .5);

private:
    Object* p_object;
    QString p_taskName;
    int p_curItem, p_numItems;
    bool p_finished;
    double p_sendTime;
};

} // namespace MO

Q_DECLARE_METATYPE(MO::ProgressInfo);
static int mo_progressInfo_init__ =
        QMetaTypeId<MO::ProgressInfo>::qt_metatype_id();

#endif // MOSRC_TOOL_PROGRESSINFO_H
