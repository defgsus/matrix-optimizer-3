/** @file geometrymodifierchain.h

    @brief Container for GeometryModifier instances

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERCHAIN_H
#define MOSRC_GEOM_GEOMETRYMODIFIERCHAIN_H

#include <QList>
#include <QMap>

namespace MO {
namespace IO { class DataStream; }
namespace GEOM {

class Geometry;
class GeometryModifier;

class GeometryModifierChain
{
public:
    GeometryModifierChain();

    // ---------------- io ----------------

    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    // ------------ factory ---------------

    /** Returns a new modifier with the specific class name, or NULL */
    static GeometryModifier * createModifier(const QString& className);
    /** Registers a modifier class */
    static bool registerModifier(GeometryModifier * g);

    // ----------- module handling --------

    /** Clears all modifiers */
    void clear();

    /** Returns read access to the list of installed modifiers */
    const QList<GeometryModifier*> modifiers() const { return modifiers_; }

    /** Adds a midifier to the execution chain.
        Ownership is taken */
    void addModifier(GeometryModifier * g);

    // ----------- execution --------------

    void execute(Geometry * g);

private:

    QList<GeometryModifier*> modifiers_;

    static QMap<QString, GeometryModifier*> registeredModifiers_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERCHAIN_H
