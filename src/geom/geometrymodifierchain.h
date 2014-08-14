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

    /** Returns a list of names of possible modifiers */
    static QList<QString> modifierClassNames();

    /** Returns a new modifier with the specific class name, or NULL */
    static GeometryModifier * createModifier(const QString& className);

    /** Registers a modifier class.
        Use MO_REGISTER_GEOMETRYMODIFIER() instead. */
    static bool registerModifier(GeometryModifier * g);

    // ----------- module handling --------

    /** Clears all modifiers */
    void clear();

    /** Returns read access to the list of installed modifiers */
    const QList<GeometryModifier*> modifiers() const { return modifiers_; }

    /** Adds a modifier to the execution chain.
        Ownership is taken */
    void addModifier(GeometryModifier * g);

    /** Adds a new modifier to the execution chain.
        Returns the created instance, or NULL if @p className is not defined. */
    GeometryModifier * addModifier(const QString& className);

    /** Inserts a new modifier to the execution chain before the modifier @p before.
        Returns the created instance, or NULL if @p className is not defined. */
    GeometryModifier * insertModifier(const QString& className, GeometryModifier * before);

    /** Moves the modifier up in the chain, returns true if moved */
    bool moveModifierUp(GeometryModifier * g);

    /** Moves the modifier down in the chain, returns true if moved */
    bool moveModifierDown(GeometryModifier * g);

    /** Deletes all instances of the modifier from the chain.
        Returns true if deleted (that is: if it was part of the chain).
        The class is only destroyed when it was part of the chain! */
    bool deleteModifier(GeometryModifier * g);

    // ----------- execution --------------

    /** Execute the whole chain */
    void execute(Geometry * g);

private:

    QList<GeometryModifier*> modifiers_;

    static QMap<QString, GeometryModifier*> * registeredModifiers_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERCHAIN_H
