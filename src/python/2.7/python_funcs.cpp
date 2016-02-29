/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON27

#include <python2.7/Python.h>

#include "python_funcs.h"
#include "python_geometry.h"
#include "python.h"
#include "geom/geometry.h"

namespace MO {
namespace PYTHON27 {

extern "C" {

struct Python27Funcs
{
    static PyObject* geometry(PyObject* self, PyObject*)
    {
        if (self)
        if (auto geom = reinterpret_cast<PythonInterpreter*>(self)->geometry())
        {
            auto pgeom = createGeometryObject(geom);
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
          (PyCFunction)Python27Funcs::geometry,
          METH_NOARGS,
          "Returns the current Geometry instance, if any."
        },

        { NULL, NULL, 0, NULL }
    };

    return &methods;
}

} // namespace PYTHON27
} // namespace MO

#endif // MO_ENABLE_PYTHON27
