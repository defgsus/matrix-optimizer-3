/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include "geometrymodifierpython34.h"
#include "geometry.h"
#include "geometryfactory.h"
#include "object/object_fwd.h"
#include "python/34/python.h"
#include "gui/widget/python34widget.h"
#include "gui/texteditdialog.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierPython34)

GeometryModifierPython34::GeometryModifierPython34()
    : GeometryModifier  ("CreatePy34", QObject::tr("Python (3.4)"))
{
    static const QString script =
            "# Python interface (beta)\n"
            "import matrixoptimizer as mo\n\n"
            "# get the current geometry instance\n"
            "g = mo.geometry()\n\n"
            "# and do something with it\n"
            "i1 = g.add_vertex(0, 0, 0)\n"
            "i2 = g.add_vertex(5, 5, 0)\n"
            "g.add_line(i1, i2)\n"
            "\n";

    properties().set(
        "script", QObject::tr("python script"),
        QObject::tr("A piece of code to freely create/modify the geometry"),
        script);
    properties().setSubType("script", Properties::ST_TEXT | TT_PYTHON34);
    /*
    properties().setWidgetCallback("script", [](QWidget*w)
    {
        if (auto te = dynamic_cast<GUI::TextEditDialog*>(w))
            if (auto as = te->getWidgetAngelScript())
                as->setScriptEngine(
                    GeometryEngineAS::createNullEngine(true) );
    });
    */
}

QString GeometryModifierPython34::statusTip() const
{
    return QObject::tr("Create or modify any geometry by using the Python language");
}

void GeometryModifierPython34::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);
}

void GeometryModifierPython34::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);
}

void GeometryModifierPython34::execute(Geometry * g)
{
    // initial color
    if (g->numVertices() == 0)
        g->setColor(1, 1, 1, 1);

    try
    {
        PYTHON34::PythonInterpreter py;
        py.setGeometry(g);
        py.execute( properties().get("script").toString() );
    }
    catch (const Exception&e)
    {
        // XXX Exceptions disturb the editor right now
        // E.g. somehow the code get's compiled again leading to endless error windows
        MO_DEBUG("GeometryModifierPython34: XXX EXCEPTION: \""
                 << e.what() << "\"");
        throw;
    }
}


} // namespace GEOM
} // namespace MO

#endif // MO_ENABLE_PYTHON34
