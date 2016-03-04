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
#include "py_tree.h"
#include "python.h"
#include "geom/geometry.h"
#include "object/scene.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {

namespace {
extern "C" {

#define MO_PY_DEF_DOC(name__, str__) \
    static constexpr const char* name__##_doc = str__;

    struct PythonFuncs
    {

        MO_PY_DEF_DOC(instance_id,
            "instance_id() -> long\n"
                      );
        static PyObject* instance_id(PyObject* , PyObject*)
        {
            long i = 0;
            if (auto inter = PythonInterpreter::current())
                i = (long)inter;
            return Py_BuildValue("n", i);
        }

        MO_PY_DEF_DOC(instance_count,
            "instance_count() -> long\n"
                      );
        static PyObject* instance_count(PyObject* , PyObject*)
        {
            long i = 0;
            if (auto inter = PythonInterpreter::current())
                i = inter->instanceCount();
            return Py_BuildValue("n", i);
        }

        MO_PY_DEF_DOC(geometry,
            "geometry() -> Geometry | None\n"
            "Returns the current Geometry instance, if any.\n"
            "This is usually only available within a geometry script."
                      );
        static PyObject* geometry(PyObject* , PyObject*)
        {
            if (auto inter = PythonInterpreter::current())
            if (inter->getGeometry())
            {
                auto p = createGeometryObject(inter->getGeometry());
                return reinterpret_cast<PyObject*>(p);
            }
            Py_RETURN_NONE;
        }

        MO_PY_DEF_DOC(object,
            "object() -> Object | None\n"
            "Returns the current Object instance, if any.\n"
            "This is usually only available from a script running in "
            "a python object or a geometry script."
                      );
        static PyObject* object(PyObject* , PyObject*)
        {
            if (auto inter = PythonInterpreter::current())
            if (inter->getObject())
            {
                auto p = createObjectWrapper(inter->getObject());
                return reinterpret_cast<PyObject*>(p);
            }
            Py_RETURN_NONE;
        }

        MO_PY_DEF_DOC(scene,
            "scene() -> Object | None\n"
            "Returns the currently loaded scene object, if any.\n"
            "This is the root object of everything."
                      );
        static PyObject* scene(PyObject* , PyObject*)
        {
            if (auto s = Scene::currentScene())
            {
                auto p = createObjectWrapper(s);
                return reinterpret_cast<PyObject*>(p);
            }
            Py_RETURN_NONE;
        }

        MO_PY_DEF_DOC(debug_tree,
            "debug_tree(Object) -> None\n"
                      );
        static PyObject* debug_tree(PyObject*, PyObject* arg)
        {
            PyObjectTree* tree = createPyObjectTree(arg);
            tree->dumpTree(std::cout);
            delete tree;
            Py_RETURN_NONE;
        }

        MO_PY_DEF_DOC(debug_object,
            "debug_object(Object) -> None\n"
                      );
        static PyObject* debug_object(PyObject*, PyObject* arg)
        {
            #define MO__PRINTPY(what__) \
                if (what__) \
                { \
                    auto s = PyObject_CallMethod(what__, "__str__", ""); \
                    MO_PRINT(#what__ ": " << PyUnicode_AsUTF8(s)); \
                } else MO_PRINT(#what__ ": NULL");
            #define MO__PRINT(what__) \
                MO_PRINT(#what__ ": " << what__);

            MO__PRINTPY(arg);
            if (arg)
            {
                MO__PRINT(arg->ob_refcnt);
                MO__PRINT(arg->ob_type);

                MO__PRINT(arg->ob_type->tp_name);
                MO__PRINT(arg->ob_type->tp_basicsize);
                MO__PRINT(arg->ob_type->tp_itemsize);

                MO__PRINT(arg->ob_type->tp_dealloc);
                MO__PRINT(arg->ob_type->tp_print);
                MO__PRINT(arg->ob_type->tp_getattr);
                MO__PRINT(arg->ob_type->tp_setattr);
                MO__PRINT(arg->ob_type->tp_reserved);
                MO__PRINT(arg->ob_type->tp_repr);
                MO__PRINT(arg->ob_type->tp_as_number);
                MO__PRINT(arg->ob_type->tp_as_sequence);
                MO__PRINT(arg->ob_type->tp_as_mapping);
                MO__PRINT(arg->ob_type->tp_hash);
                MO__PRINT(arg->ob_type->tp_call);
                MO__PRINT(arg->ob_type->tp_str);
                MO__PRINT(arg->ob_type->tp_getattro);
                MO__PRINT(arg->ob_type->tp_setattro);
                MO__PRINT(arg->ob_type->tp_as_buffer);
                MO__PRINT(arg->ob_type->tp_flags);
                //MO__PRINT(arg->ob_type->tp_doc);
                MO__PRINT(arg->ob_type->tp_traverse);
                MO__PRINT(arg->ob_type->tp_clear);
                MO__PRINT(arg->ob_type->tp_richcompare);
                MO__PRINT(arg->ob_type->tp_weaklistoffset);
                MO__PRINT(arg->ob_type->tp_iter);
                MO__PRINT(arg->ob_type->tp_iternext);
                MO__PRINT(arg->ob_type->tp_methods);
                MO__PRINT(arg->ob_type->tp_members);
                MO__PRINT(arg->ob_type->tp_getset);
                MO__PRINT(arg->ob_type->tp_base);
                MO__PRINTPY(arg->ob_type->tp_dict);
                MO__PRINT(arg->ob_type->tp_descr_get);
                MO__PRINT(arg->ob_type->tp_descr_set);
                MO__PRINT(arg->ob_type->tp_dictoffset);
                MO__PRINT(arg->ob_type->tp_init);
                MO__PRINT(arg->ob_type->tp_alloc);
                MO__PRINT(arg->ob_type->tp_new);
                MO__PRINT(arg->ob_type->tp_free);
                MO__PRINT(arg->ob_type->tp_is_gc);

                MO__PRINTPY(arg->ob_type->tp_bases);
                MO__PRINTPY(arg->ob_type->tp_mro);
                MO__PRINTPY(arg->ob_type->tp_cache);
                MO__PRINTPY(arg->ob_type->tp_subclasses);
                MO__PRINTPY(arg->ob_type->tp_weaklist);
                MO__PRINT(arg->ob_type->tp_del);
                MO__PRINT(arg->ob_type->tp_version_tag);
                MO__PRINT(arg->ob_type->tp_finalize);

#ifdef COUNT_ALLOCS
                MO__PRINT(arg->ob_type->tp_allocs);
                MO__PRINT(arg->ob_type->tp_frees);
                MO__PRINT(arg->ob_type->tp_maxalloc);
                MO__PRINT(arg->ob_type->tp_prev);
                MO__PRINT(arg->ob_type->tp_next);
#endif

            }

            #undef MO__PRINT
            #undef MO__PRINTPY

            Py_RETURN_NONE;
        }

    };

} // extern "C"
} // namespace

void* pythonFuncs()
{

#define MO__METHOD(name__, args__) \
    { #name__, (PyCFunction)PythonFuncs::name__, args__, PythonFuncs::name__##_doc },

    static PyMethodDef methods[] =
    {
        MO__METHOD(geometry,        METH_NOARGS)
        MO__METHOD(object,          METH_NOARGS)
        MO__METHOD(scene,           METH_NOARGS)
        MO__METHOD(instance_id,     METH_NOARGS)
        MO__METHOD(instance_count,  METH_NOARGS)
        MO__METHOD(debug_object,    METH_O)
        MO__METHOD(debug_tree,      METH_O)

        { NULL, NULL, 0, NULL }
    };
#undef MO__METHOD

    return &methods;
}

} // namespace PYTHON34
} // namespace MO

#endif // MO_ENABLE_PYTHON34
