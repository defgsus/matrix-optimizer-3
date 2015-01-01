/** @file angelscript_geometry.h

    @brief Total script access to GEOM::Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 22.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_SCRIPT_ANGELSCRIPT_GEOMETRY_H
#define MOSRC_SCRIPT_ANGELSCRIPT_GEOMETRY_H

class asIScriptEngine;
class QString;

namespace MO {
namespace GEOM { class Geometry; }

class Object;
class Scene;

    /** The internal type to wrap Geometry for AngelScript.
        General access to this class is not public.
        The interface is designed to be used by scripts only. */
    class GeometryAS;

    /** Returns the assigned geometry widget of the script object, or NULL */
    GEOM::Geometry * getGeometry(const GeometryAS*);

    /** Put the object type and related functions into the namespace */
    void registerAngelScript_geometry(asIScriptEngine *engine);


    /** For immediate script-access to one Geometry instance.
        The usage is easy:
        @code
        // create an instance to wrap a GEOM::Geometry
        GeometryEngineAS script(geometry);
        // compile and run the script (QString)
        script.execute(scriptText);
        @endcode
        To check a script for errors, call createNullEngine(),
        to receive an engine with the full namespace defined
        but without the assigned geometry.
        */
    class GeometryEngineAS
    {
    public:
        /** Creates a script engine wrapper for the given geometry */
        GeometryEngineAS(GEOM::Geometry * );
        /** Creates a script engine wrapper for the given geometry with READ access to the object */
        GeometryEngineAS(GEOM::Geometry *, Object * o);

        ~GeometryEngineAS();

        /** Returns the script engine with all the functionality
            to modify this Geometry instance */
        asIScriptEngine * scriptEngine();

        /** Runs the script on the geometry */
        void execute(const QString& script);

        /** Creates an engine with the correct namespace but no assigned Geometry.
            Must not be executed!
            This is used by the editor for syntax highlighting.
            Ownership is on caller. */
        static asIScriptEngine * createNullEngine(bool withObject);

    private:

        class Private;
        Private * p_;
    };


} // namespace MO


#endif // MOSRC_SCRIPT_ANGELSCRIPT_GEOMETRY_H

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
