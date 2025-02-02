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
#include <QSet>
#include <QString>
#include <QPoint>

#include "object/Object_fwd.h"
#include "ViewSpace.h"

namespace MO {
namespace IO { class DataStream; }
namespace GUI {

/** Almost obsolete gui settings for Objects
    @deprecated */
class SceneSettings : public QObject
{
    Q_OBJECT
public:
    explicit SceneSettings(QObject * parent = 0);
    SceneSettings(const SceneSettings& other);
    SceneSettings& operator=(const SceneSettings& other);


    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    QString getSettingsFileName(const QString& sceneFilename) const;
    void saveFile(const QString& filename) const;
    void loadFile(const QString& filename);

    /** Clear everything */
    void clear();

    // --------------- compatibility ---------------------

    /** Attach old loaded data to objects directly */
    void updateTreeForCompatibility(Object * root);

    // -------- ViewSpaces accociated to Objects ---------

    void setViewSpace(const Object *obj, const UTIL::ViewSpace &viewspace);
    UTIL::ViewSpace getViewSpace(const Object * obj);

    // --------------- Track heights ---------------------

    //void setTrackHeight(const Track *, int);
    //int getTrackHeight(const Track *) const;

    // ------ expanded-flag of ParameterGroups -----------

#if (0)
    void setParameterGroupExpanded(const Object *, const QString& groupId, bool expanded);
    bool getParameterGroupExpanded(const Object *, const QString& groupId) const;

    // ------------- ObjectGraphView ---------------------

    bool hasLocalGridPos(const Object*, const QString& groupId) const;
    const QPoint &getLocalGridPos(const Object*, const QString& groupId) const;
    void setLocalGridPos(const Object*, const QString& groupdId, const QPoint&);

    void setExpanded(const Object *, const QString& groupdId, bool expanded);
    bool getExpanded(const Object *, const QString& groupdId) const;
#endif

    // -------------------- copy -------------------------

    /** Copies all settings from @p src to @p dst */
    void copySettings(const Object * dst, const Object * src);
    void copySettings(const QString& dstId, const QString& srcId);

private:

    QHash<QString, UTIL::ViewSpace> viewSpaces_;
    QHash<QString, int> trackHeights_;
    /** @deprecated */
    QHash<QString, QPoint> gridPos_;
    QSet<QString>
    /** @deprecated */
        paramGroupExpanded_,
    /** @deprecated */
        treeExpanded_;

    int readVersion_;

    // ---- config ----

    bool useCompression_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_SCENESETTINGS_H
