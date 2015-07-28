/** @file geometrymodifierangelscript.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include "geometrymodifierangelscript.h"
#include "geometry.h"
#include "geometryfactory.h"
#include "script/angelscript_geometry.h"
#include "object/object_fwd.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierAngelScript)

GeometryModifierAngelScript::GeometryModifierAngelScript()
    : GeometryModifier  ("CreateAS", QObject::tr("angelscript"))
{
    static const QString script =
            "// angelscript (beta)\n\n"
            "void main()\n"
            "{\n"
                "\t// get the handle to the current geometry\n"
                "\tGeometry@ g = geometry();\n"
                "\t//and do something with it\n"
                "\tg.addLine(vec3(0), vec3(1));\n"
            "}\n"
            ;

    properties().set(
        "script", QObject::tr("angelscript"),
        QObject::tr("A piece of code to freely create/modify the geometry"),
        script);
    properties().setSubType("script", Properties::ST_TEXT | TT_ANGELSCRIPT);
}

QString GeometryModifierAngelScript::statusTip() const
{
    return QObject::tr("Create any geometry by using the angelscript language");
}

void GeometryModifierAngelScript::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geocreateas", 2);
}

void GeometryModifierAngelScript::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    int ver = io.readHeader("geocreateas", 2);

    if (ver < 2)
    {
        QString script;
        io >> script;
        properties().set("script", script);
    }
}

void GeometryModifierAngelScript::execute(Geometry * g)
{
    // shared vertices?
    //g->setSharedVertices(sharedVertices_);

    // initial color
    g->setColor(1, 1, 1, 1);

    try
    {
        GeometryEngineAS script(g, currentObject());
        script.execute(properties().get("script").toString());
    }
    catch (const Exception&e)
    {
        // XXX Exceptions disturb the editor right now
        // E.g. somehow the code get's compiled again leading to endless error windows
        MO_DEBUG("GeometryModifierAngelScript: XXX EXCEPTION: \""
                 << e.what() << "\"");
    }
}


} // namespace GEOM
} // namespace MO

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
