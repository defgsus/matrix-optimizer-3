/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON27

#include <python2.7/Python.h>
#include <python2.7/structmember.h>

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
        if (PyInt_Check(obj))
        {
            *val = PyInt_AsLong(obj);
            return true;
        }
        if (PyLong_Check(obj))
        {
            *val = PyLong_AsLong(obj);
            return true;
        }
        return false;
    }
#if 0
    bool py_array_to_vec3(PyObject* obj, Vec3* vec)
    {
        if (!PySequence_Check(obj))
        {
            PyErr_SetString(PyExc_TypeError, "expected float array");
            return false;
        }
        if (PySequence_Size(obj) != 3)
        {
            PyErr_SetString(PyExc_ValueError, "invalid size of array");
            return false;
        }
        for (int i=0; i<3; ++i)
        {
            PyObject* val = PySequence_GetItem(obj, i);
            //if (val) MO_PRINT("refcnt: " << val->ob_refcnt);
            double d;
            bool worked = py_to_double(val, &d);
            //MO_PRINT("conv: " << yes);
            Py_XDECREF(val);
            if (!worked)
            {
                PyErr_SetString(PyExc_TypeError,
                                "invalid type in array, expected float");
                return false;
            }
            (*vec)[i] = d;
        }
        return true;
    }
#endif

    bool py_array_or_tuple_to_vec3(PyObject* obj, Vec3* v)
    {
        if (PyArg_ParseTuple(obj, "(fff)", &v->x, &v->y, &v->z))
            return true;
        PyErr_Clear();
        if (PyArg_ParseTuple(obj, "fff", &v->x, &v->y, &v->z))
            return true;
        return false;
    }
}



extern "C"
{
    struct Python27Geom
    {
        PyObject_HEAD
        GEOM::Geometry* geometry;

        static constexpr const char* docString =
                "The Geometry object";

        static void dealloc(Python27Geom* self)
        {
            if (self->geometry)
                self->geometry->releaseRef();
            self->ob_type->tp_free((PyObject*)self);
        }

        static int init(Python27Geom* self, PyObject* args, PyObject* kwds)
        {
            if (self->geometry)
                self->geometry->releaseRef();
            self->geometry = new GEOM::Geometry;

            return 0;
        }

        static PyObject* newfunc(PyTypeObject* type, PyObject* args, PyObject* kwds)
        {
            Python27Geom* self = (Python27Geom*)type->tp_alloc(type, 0);

            if (self != NULL)
            {
                self->geometry = nullptr;
                init(self, args, kwds);
            }

            return (PyObject*)self;
        }
    };

    struct Python27GeomFuncs
    {
        #define MO__GETGEOM(name__) \
            if (self == nullptr) \
            { \
                PyErr_SetString(PyExc_RuntimeError, "self is NULL"); \
                return NULL; \
            } \
            auto name__ = reinterpret_cast<Python27Geom*>(self);

        static PyObject* num_vertices(PyObject* self, PyObject* )
        {
            MO__GETGEOM(pgeom);
            return Py_BuildValue("n", pgeom->geometry->numVertices());
        }

        static PyObject* to_string(PyObject* self, PyObject* )
        {
            MO__GETGEOM(pgeom);
            auto str = pgeom->geometry->infoString();
            str += QString(" (refcnt:%1)").arg(self->ob_refcnt);
            return Py_BuildValue("s", str.toLatin1().constData());
        }

        static PyObject* add_vertex(PyObject* self, PyObject* obj)
        {
            Vec3 v;
            if (!py_array_or_tuple_to_vec3(obj, &v))
                return NULL;
            MO__GETGEOM(pgeom);
            pgeom->geometry->addVertex(v);
            Py_INCREF(self);
            return self;
        }


        #undef MO__GETGEOM
    };

    PyMemberDef Python27Geom_members[] =
    {
        { NULL, 0, 0, 0, NULL }
    };

    PyMethodDef Python27Geom_methods[] =
    {
        { "__str__",
          (PyCFunction)Python27GeomFuncs::to_string,
          METH_NOARGS,
          "Returns an informative string"
        },

        { "num_vertices",
          (PyCFunction)Python27GeomFuncs::num_vertices,
          METH_NOARGS,
          "Returns the number of vertices defined"
        },

        { "add_vertex",
          (PyCFunction)Python27GeomFuncs::add_vertex,
          METH_VARARGS,
          "Adds a new vertex. "
          "Function arguments can either be 3 floats or a list of three floats."
        },

        { NULL, NULL, 0, NULL }
    };


    static PyTypeObject Python27Geom_type = {
        PyObject_HEAD_INIT(NULL)
        0,                         /*ob_size*/
        "matrixoptimizer.Geometry",/*tp_name*/
        sizeof(Python27Geom),      /*tp_basicsize*/
        0,                         /*tp_itemsize*/
        (destructor)Python27Geom::dealloc,/*tp_dealloc*/
        0,                         /*tp_print*/
        0,                         /*tp_getattr*/
        0,                         /*tp_setattr*/
        0,                         /*tp_compare*/
        0,                         /*tp_repr*/
        0,                         /*tp_as_number*/
        0,                         /*tp_as_sequence*/
        0,                         /*tp_as_mapping*/
        0,                         /*tp_hash */
        0,                         /*tp_call*/
        0,                         /*tp_str*/
        0,                         /*tp_getattro*/
        0,                         /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
        Python27Geom::docString,             /* tp_doc */
        0,		               /* tp_traverse */
        0,		               /* tp_clear */
        0,		               /* tp_richcompare */
        0,		               /* tp_weaklistoffset */
        0,		               /* tp_iter */
        0,		               /* tp_iternext */
        Python27Geom_methods,             /* tp_methods */
        Python27Geom_members,             /* tp_members */
        0,                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        (initproc)Python27Geom::init,      /* tp_init */
        0,                         /* tp_alloc */
        (newfunc)Python27Geom::newfunc,       /* tp_new */
        0, /*tp_free*/
        0, /*tp_is_gc*/
        0, /*tp_bases*/
        0, /*tp_mro*/
        0, /*tp_cache*/
        0, /*tp_subclasses*/
        0, /*tp_weaklist*/
        0, /*tp_del*/
        0, /*tp_version_tag*/
    #ifdef COUNT_ALLOCS
        0, /*tp_allocs*/
        0, /*tp_frees*/
        0, /*tp_maxalloc*/
        0, /*tp_prev*/
        0, /*tp_next*/
    #endif
    };

} // extern "C"


namespace PYTHON27 {

void initGeometry(void* mod)
{
    PyObject* module = reinterpret_cast<PyObject*>(mod);

    if (0 != PyType_Ready(&Python27Geom_type))
        MO_ERROR("Failed to readify Geometry object with Python 2.7");

    Py_INCREF(&Python27Geom_type);
    if (0 != PyModule_AddObject(module, "Geometry", (PyObject*)&Python27Geom_type))
    {
        Py_DECREF(&Python27Geom_type);
        MO_ERROR("Failed to add Geometry object to Python 2.7");
    }

    //auto ins = PyInstance_New((PyObject*)&Python27Geom_type, NULL, NULL);
    //PyModule_AddObject(module, "k", ins);
}

void* createGeometryObject(MO::GEOM::Geometry* geom)
{
    auto pgeom = PyObject_New(Python27Geom, &Python27Geom_type);
    pgeom->geometry = geom;
    return pgeom;
}

} // namespace PYTHON27
} // namespace MO

#endif // MO_ENABLE_PYTHON27

