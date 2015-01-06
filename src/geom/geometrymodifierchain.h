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
#include "io/filetypes.h"

namespace MO {
class Object;
namespace IO { class DataStream; }
namespace GEOM {

class Geometry;
class GeometryModifier;
class ObjLoader;

class GeometryModifierChain
{
public:
    GeometryModifierChain();
    ~GeometryModifierChain();

    GeometryModifierChain(const GeometryModifierChain& other);
    GeometryModifierChain& operator = (const GeometryModifierChain& other);

    // ---------------- io ----------------

    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    // ------------ factory ---------------

    /** Returns a list of names of possible modifiers */
    static QList<QString> modifierClassNames();

    /** Returns a list of friendly names of possible modifiers */
    static QList<QString> modifierGuiNames();

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

    /** Inserts a modifier to the execution chain.
        Ownership is taken */
    void insertModifier(GeometryModifier * g, uint index);

    /** Adds a new modifier to the execution chain.
        Returns the created instance, or NULL if @p className is not defined. */
    GeometryModifier * addModifier(const QString& className);

    /** Inserts a new modifier to the execution chain before the modifier @p before.
        Returns the created instance, or NULL if @p className is not defined. */
    GeometryModifier * insertModifier(const QString& className, GeometryModifier * before);

    /** Inserts a new modifier to the execution chain at given index.
        Returns the created instance, or NULL if @p className is not defined. */
    GeometryModifier * insertModifier(const QString& className, uint index);

    /** Moves the modifier up in the chain, returns true if moved */
    bool moveModifierUp(GeometryModifier * g);

    /** Moves the modifier down in the chain, returns true if moved */
    bool moveModifierDown(GeometryModifier * g);

    /** Deletes all instances of the modifier from the chain.
        Returns true if deleted (that is: if it was part of the chain).
        The class is only destroyed when it was part of the chain! */
    bool deleteModifier(GeometryModifier * g);

    // ----------- info -------------------

    void getNeededFiles(IO::FileList & files) const;

    // ----------- execution --------------

    /** Execute the whole chain.
        If an object is given, it is made available to scripts */
    void execute(Geometry * g, Object* o = 0) const;

private:

    QList<GeometryModifier*> modifiers_;

    static QMap<QString, GeometryModifier*> * registeredModifiers_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERCHAIN_H
