/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include "GeometryModifierPython34.h"
#include "Geometry.h"
#include "GeometryFactory.h"
#include "object/Object_fwd.h"
#include "python/34/python.h"
#include "gui/widget/Python34Widget.h"
#include "gui/TextEditDialog.h"
#include "io/DataStream.h"
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
        g->setColor(.5, .5, .5, 1);

    // brute-force to attach error message to widget
    GUI::Python34Widget* widget = dynamic_cast<GUI::Python34Widget*>
            (GUI::AbstractScriptWidget::instanceForScriptText(
                 properties().get("script").toString()));

    PYTHON34::PythonInterpreter py;
    try
    {
        py.setGeometry(g);
        py.execute( properties().get("script").toString() );
    }
    catch (const Exception&e)
    {
        MO_DEBUG("GeometryModifierPython34: XXX EXCEPTION: \""
                 << e.what() << "\"");
        if (widget)
        {
            widget->setErrorFrom(&py);
            widget->addCompileMessage(0, widget->M_ERROR, e.what());
        }
        // if widget is not attached pass through to system
        else throw;
    }

    if (widget)
        widget->setErrorFrom(&py);
}


} // namespace GEOM
} // namespace MO

#endif // MO_ENABLE_PYTHON34
