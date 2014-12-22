/** @file geometryangelscript.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_GEOM_GEOMETRYANGELSCRIPT_H
#define MOSRC_GEOM_GEOMETRYANGELSCRIPT_H


class asIScriptEngine;
class QString;


namespace MO {
namespace GEOM {

class Geometry;

/** For immediate access to one Geometry */
class GeometryAngelScript
{
public:
    /** Creates a script engine wrapper for the given geometry */
    GeometryAngelScript(Geometry * );
    ~GeometryAngelScript();

    /** Returns an engine with all the functionality
        to modify this Geometry instance */
    asIScriptEngine * scriptEngine();

    /** Runs the script on the geometry */
    void execute(const QString& script);

    /** Creates an engine with the correct namespace but no assign Geometry.
        Must not be executed! */
    static asIScriptEngine * createNullEngine();

private:

    class Private;
    Private * p_;
};

} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYANGELSCRIPT_H

#endif // MO_DISABLE_ANGELSCRIPT
