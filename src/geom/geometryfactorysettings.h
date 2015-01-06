/** @file geometryfactorysettings.h

    @brief Settings for dynamically creating Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYFACTORYSETTINGS_H
#define MOSRC_GEOM_GEOMETRYFACTORYSETTINGS_H

#include <QStringList>

#include "types/int.h"
#include "types/float.h"
#include "io/filetypes.h"

namespace MO {
class Object;
namespace IO { class DataStream; }
namespace GEOM {

class GeometryModifierChain;

/** Container of settings to create a Geometry */
class GeometryFactorySettings
{
public:
    GeometryFactorySettings(Object * o);
    ~GeometryFactorySettings();

    GeometryFactorySettings(const GeometryFactorySettings& other);
    GeometryFactorySettings& operator=(const GeometryFactorySettings& other);

    // ---------- io ---------

    void serialize(IO::DataStream&) const;
    void deserialize(IO::DataStream&);

    void saveFile(const QString& filename) const;
    void loadFile(const QString& filename);

    /** Appends the geometry file to the list if type is T_FILE */
    void getNeededFiles(IO::FileList &files);

    // ---- settings ----

    /** Assigns an object */
    void setObject(Object*o) { object_ = o; }

    /** Returns assigned object */
    Object * object() const { return object_; }

    /** Access to the create/modify chain */
    GeometryModifierChain * modifierChain() const { return modifierChain_; }

private:

    Object * object_;

    GeometryModifierChain * modifierChain_;

};


} // namespace GEOM
} // namespace MO

#endif // GEOMETRYFACTORYSETTINGS_H
