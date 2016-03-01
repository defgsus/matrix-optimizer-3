/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include <python3.4/Python.h>
#include <python3.4/structmember.h>

#include "python_geometry.h"
#include "python.h"
#include "geom/geometry.h"
#include "math/vector.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

namespace
{
    bool py_to_double(PyObject* obj, double* val)
    {
        if (PyFloat_Check(obj))
        {
            *val = PyFloat_AsDouble(obj);
            return true;
        }
        if (PyLong_Check(obj))
        {
            *val = PyLong_AsLong(obj);
            return true;
        }
        return false;
    }


    bool py_array_or_tuple_to_vec3(PyObject* obj, Vec3* v)
    {
        if (PyArg_ParseTuple(obj, "(fff)", &v->x, &v->y, &v->z))
            return true;
        PyErr_Clear();
        if (PyArg_ParseTuple(obj, "fff", &v->x, &v->y, &v->z))
            return true;
        return false;
    }

    bool py_array_or_tuple_to_uint2(PyObject* obj,
                                    unsigned long* v1, unsigned long* v2)
    {
        if (PyArg_ParseTuple(obj, "(kk)", v1, v2))
            return true;
        PyErr_Clear();
        if (PyArg_ParseTuple(obj, "kk", v1, v2))
            return true;
        return false;
    }
}



extern "C"
{
    struct Python34Geom
    {
        PyObject_HEAD
        GEOM::Geometry* geometry;

        static constexpr const char* docString =
                "The Geometry object";

        static void dealloc(Python34Geom* self)
        {
            //MO_PRINT("Geom dealloc");
            if (self->geometry)
                self->geometry->releaseRef();
            self->ob_base.ob_type->tp_free((PyObject*)self);
        }

        static int init(Python34Geom* self, PyObject* args, PyObject*)
        {
            auto owngeom = self->geometry;

            PyObject* obj = 0;
            PyArg_ParseTuple(args, "|O", &obj);
            if (obj && 0==strcmp(obj->ob_type->tp_name, "matrixoptimizer.Geometry"))
            {
                auto other = reinterpret_cast<Python34Geom*>(obj);
                self->geometry = other->geometry;
                self->geometry->addRef();
            }
            else
                self->geometry = new GEOM::Geometry;

            if (owngeom)
                owngeom->releaseRef();

            return 0;
        }

        static PyObject* newfunc(PyTypeObject* type, PyObject* args, PyObject* kwds)
        {
            Python34Geom* self = (Python34Geom*)type->tp_alloc(type, 0);

            if (self != NULL)
            {
                self->geometry = nullptr;
                init(self, args, kwds);
            }

            return reinterpret_cast<PyObject*>(self);
        }
    };

    struct Python34GeomFuncs
    {
        #define MO__GETGEOM0(name__) \
            if (self == nullptr) \
            { \
                PyErr_SetString(PyExc_RuntimeError, "self is NULL"); \
                return NULL; \
            } \
            auto name__ = reinterpret_cast<Python34Geom*>(self);

        #define MO__GETGEOM(name__) \
            MO__GETGEOM0(name__) \
            if (name__->geometry == nullptr) { Py_RETURN_NONE; }

        static PyObject* num_vertices(PyObject* self, PyObject* )
        {
            MO__GETGEOM(pgeom);
            return Py_BuildValue("n", pgeom->geometry->numVertices());
        }

        static PyObject* to_string(PyObject* self, PyObject* )
        {
            MO__GETGEOM0(pgeom);
            if (!pgeom->geometry)
                return Py_BuildValue("s", "*empty*");
            auto str = pgeom->geometry->infoString();
            str += QString(" (refcnt:%1)").arg(self->ob_refcnt);
            return Py_BuildValue("s", str.toLatin1().constData());
        }
        static PyObject* repr(PyObject* self) { return to_string(self, nullptr); }

        static PyObject* add_vertex(PyObject* self, PyObject* obj)
        {
            Vec3 v;
            if (!py_array_or_tuple_to_vec3(obj, &v))
                return NULL;
            MO__GETGEOM(pgeom);
            auto i = pgeom->geometry->addVertex(v);
            return Py_BuildValue("n", i);
        }

        static PyObject* add_line(PyObject* self, PyObject* obj)
        {
            unsigned long i1, i2;
            if (!py_array_or_tuple_to_uint2(obj, &i1, &i2))
                return NULL;
            MO__GETGEOM(pgeom);
            if (i1 >= pgeom->geometry->numVertices()
             || i2 >= pgeom->geometry->numVertices())
            {
                PyErr_SetString(PyExc_IndexError, "index out of range");
                return NULL;
            }
            pgeom->geometry->addLine(i1, i2);
            Py_RETURN_NONE;
        }

        #undef MO__GETGEOM
        #undef MO__GETGEOM0
    };

    PyMemberDef Python34Geom_members[] =
    {
        { NULL, 0, 0, 0, NULL }
    };

    PyMethodDef Python34Geom_methods[] =
    {
        { "to_string",
          (PyCFunction)Python34GeomFuncs::to_string,
          METH_NOARGS,
          "to_string() -> string\n"
          "Returns an informative string"
        },

        { "num_vertices",
          (PyCFunction)Python34GeomFuncs::num_vertices,
          METH_NOARGS,
          "num_vertices() -> long\n"
          "Returns the number of vertices"
        },

        { "add_vertex",
          (PyCFunction)Python34GeomFuncs::add_vertex,
          METH_VARARGS,
          "add_vertex(f, f, f) -> long\n"
          "add_vertex([f, f, f]) -> long\n"
          "Adds a new vertex. The returned value is the index of the vertex. "
          "For unshared geometries this value is equal to num_vertices() - 1, "
          "for shared geometries the vertex might have been present already "
          "and it's index is returned."
        },

        { "add_line",
          (PyCFunction)Python34GeomFuncs::add_line,
          METH_VARARGS,
          "add_line(long, long) -> None\n"
          "add_line([long, long]) -> None\n"
          "Adds a line between two vertices."
        },

        { NULL, NULL, 0, NULL }
    };


    static PyTypeObject Python34Geom_type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        "matrixoptimizer.Geometry",/*tp_name*/
        sizeof(Python34Geom),      /*tp_basicsize*/
        0,                         /*tp_itemsize*/
        (destructor)Python34Geom::dealloc,/*tp_dealloc*/
        0,                         /*tp_print*/
        0,                         /*tp_getattr*/
        0,                         /*tp_setattr*/
        0,                         /*tp_reserved*/
        0,                         /*tp_repr*/
        0,                         /*tp_as_number*/
        0,                         /*tp_as_sequence*/
        0,                         /*tp_as_mapping*/
        0,                         /*tp_hash */
        0,                         /*tp_call*/
        Python34GeomFuncs::repr,   /*tp_str*/
        0,                         /*tp_getattro*/
        0,                         /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
        Python34Geom::docString,   /* tp_doc */
        0,		               /* tp_traverse */
        0,		               /* tp_clear */
        0,		               /* tp_richcompare */
        0,		               /* tp_weaklistoffset */
        0,		               /* tp_iter */
        0,		               /* tp_iternext */
        Python34Geom_methods,      /* tp_methods */
        Python34Geom_members,      /* tp_members */
        0,                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        (initproc)Python34Geom::init,      /* tp_init */
        0,                         /* tp_alloc */
        (newfunc)Python34Geom::newfunc,       /* tp_new */
        0, /*tp_free*/
        0, /*tp_is_gc*/
        0, /*tp_bases*/
        0, /*tp_mro*/
        0, /*tp_cache*/
        0, /*tp_subclasses*/
        0, /*tp_weaklist*/
        0, /*tp_del*/
        0, /*tp_version_tag*/
        0, /*tp_finalize*/
    #ifdef COUNT_ALLOCS
        0, /*tp_allocs*/
        0, /*tp_frees*/
        0, /*tp_maxalloc*/
        0, /*tp_prev*/
        0, /*tp_next*/
    #endif
    };

} // extern "C"


namespace PYTHON34 {

void initGeometry(void* mod)
{
    PyObject* module = reinterpret_cast<PyObject*>(mod);

    if (0 != PyType_Ready(&Python34Geom_type))
        MO_ERROR("Failed to readify Geometry object with Python 3.4");


    PyObject* object = reinterpret_cast<PyObject*>(&Python34Geom_type);
    //PyObject* object = reinterpret_cast<PyObject*>(createGeometryObject(nullptr));
    Py_INCREF(object);
    if (0 != PyModule_AddObject(module, "Geometry", object))
    {
        Py_DECREF(object);
        MO_ERROR("Failed to add Geometry object to Python 3.4");
    }
}


void* createGeometryObject(MO::GEOM::Geometry* geom)
{
    auto pgeom = PyObject_New(Python34Geom, &Python34Geom_type);
    pgeom->geometry = geom;
    pgeom->geometry->addRef();
    return pgeom;
}

} // namespace PYTHON34
} // namespace MO

#endif // MO_ENABLE_PYTHON34

