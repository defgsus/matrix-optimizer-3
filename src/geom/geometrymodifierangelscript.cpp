/** @file geometrymodifierangelscript.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include "geometrymodifierangelscript.h"
#include "io/datastream.h"
#include "geometry.h"
#include "geometryfactory.h"
#include "geometryangelscript.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierAngelScript)

GeometryModifierAngelScript::GeometryModifierAngelScript()
    : GeometryModifier  ("CreateAS", QObject::tr("angelscript"))
{
    script_ =
            "// angelscript test\n\n"
            "void main()\n"
            "{\n"
                "\taddLine(vec3(0), vec3(1));\n"
            "}\n"
            ;
}

QString GeometryModifierAngelScript::statusTip() const
{
    return QObject::tr("Create any geometry by using the angelscript language");
}

void GeometryModifierAngelScript::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geocreateas", 1);

    io << script_;
}

void GeometryModifierAngelScript::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geocreateas", 1);

    io >> script_;
}

void GeometryModifierAngelScript::execute(Geometry * g)
{
    // shared vertices?
    //g->setSharedVertices(sharedVertices_);

    // initial color
    g->setColor(1, 1, 1, 1);

    try
    {
        GeometryAngelScript script(g);
        script.execute(script_);
    }
    catch (...)
    {
        // XXX Exceptions disturb the editor right now
    }
}


} // namespace GEOM
} // namespace MO

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
