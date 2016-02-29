/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include <python3.4/Python.h>

#include "python_funcs.h"
#include "python_geometry.h"
#include "python.h"
#include "geom/geometry.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {

extern "C" {

struct Python34Funcs
{
    static PyObject* geometry(PyObject* , PyObject*)
    {
        if (auto inter = PythonInterpreter::current())
        if (inter->geometry())
        {
            auto pgeom = createGeometryObject(inter->geometry());
            return reinterpret_cast<PyObject*>(pgeom);
        }
        Py_RETURN_NONE;
    }
};


} // extern "C"

void* pythonFuncs()
{
    static PyMethodDef methods[] =
    {
        { "geometry",
          (PyCFunction)Python34Funcs::geometry,
          METH_NOARGS,
          "geometry() -> Geometry | None\n"
          "Returns the current Geometry instance, if any."
        },

        { NULL, NULL, 0, NULL }
    };

    return &methods;
}

} // namespace PYTHON34
} // namespace MO

#endif // MO_ENABLE_PYTHON34
