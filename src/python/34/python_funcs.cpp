/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include <python3.4/Python.h>

#include "python_funcs.h"
#include "python_object.h"
#include "python_geometry.h"
#include "python.h"
#include "geom/geometry.h"
#include "object/scene.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {

namespace {
extern "C" {

    struct PythonFuncs
    {
        static PyObject* get_geometry(PyObject* , PyObject*)
        {
            if (auto inter = PythonInterpreter::current())
            if (inter->getGeometry())
            {
                auto p = createGeometryObject(inter->getGeometry());
                return reinterpret_cast<PyObject*>(p);
            }
            Py_RETURN_NONE;
        }

        static PyObject* get_object(PyObject* , PyObject*)
        {
            if (auto inter = PythonInterpreter::current())
            if (inter->getObject())
            {
                auto p = createObjectWrapper(inter->getObject());
                return reinterpret_cast<PyObject*>(p);
            }
            Py_RETURN_NONE;
        }

        static PyObject* get_scene(PyObject* , PyObject*)
        {
            if (auto s = Scene::currentScene())
            {
                auto p = createObjectWrapper(s);
                return reinterpret_cast<PyObject*>(p);
            }
            Py_RETURN_NONE;
        }

    };

} // extern "C"
} // namespace

void* pythonFuncs()
{
    static PyMethodDef methods[] =
    {
        { "geometry",
          (PyCFunction)PythonFuncs::get_geometry,
          METH_NOARGS,
          "geometry() -> Geometry | None\n"
          "Returns the current Geometry instance, if any.\n"
          "This is usually only available within a geometry script."
        },

        { "object",
          (PyCFunction)PythonFuncs::get_object,
          METH_NOARGS,
          "object() -> Object | None\n"
          "Returns the current Object instance, if any.\n"
          "This is usually only available from a script running in "
          "a python object or a geometry script."
        },

        { "scene",
          (PyCFunction)PythonFuncs::get_scene,
          METH_NOARGS,
          "scene() -> Object | None\n"
          "Returns the currently loaded scene object, if any.\n"
          "This is the root object of everything."
        },

        { NULL, NULL, 0, NULL }
    };

    return &methods;
}

} // namespace PYTHON34
} // namespace MO

#endif // MO_ENABLE_PYTHON34
