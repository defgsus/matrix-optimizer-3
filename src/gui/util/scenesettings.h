/** @file scenesettings.h

    @brief ViewSpaces and stuff for scene objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#ifndef MOSRC_GUI_UTIL_SCENESETTINGS_H
#define MOSRC_GUI_UTIL_SCENESETTINGS_H

#include <QObject>
#include <QHash>
#include <QString>

#include "object/object_fwd.h"
#include "viewspace.h"

namespace MO {
namespace GUI {


class SceneSettings : public QObject
{
    Q_OBJECT
public:
    SceneSettings(QObject * parent = 0);

    /** Clear everything */
    void clear();

    void setViewSpace(const Object *obj, const UTIL::ViewSpace &viewspace);
    UTIL::ViewSpace getViewSpace(const Object * obj);

    void setTrackHeight(const Track *, int);
    int getTrackHeight(const Track *) const;

private:

    QHash<QString, UTIL::ViewSpace> viewSpaces_;
    QHash<QString, int> trackHeights_;

    // ---- config ----

    int defaultTrackHeight_;

};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_SCENESETTINGS_H
