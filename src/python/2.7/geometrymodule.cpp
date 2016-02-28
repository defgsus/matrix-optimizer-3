/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON27

#include <python2.7/Python.h>
#include <python2.7/structmember.h>

#include "geometrymodule.h"
#include "python.h"
#include "geom/geometry.h"
#include "io/error.h"

namespace MO {


extern "C"
{
    struct Python27GeomFuncs
    {
        static PyObject* num_vertices(PyObject* self, PyObject* )
        {
            return Py_BuildValue("n", 23);
        }
    };

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
            self->geometry = nullptr;
            return 0;
        }
    };

    PyMemberDef Python27Geom_members[] =
    {
        { NULL, 0, 0, 0, NULL }
    };

    PyMethodDef Python27Geom_methods[] =
    {
        { "num_vertices",
          (PyCFunction)Python27GeomFuncs::num_vertices,
          METH_NOARGS,
          "Returns the number of vertices defined in the Geometry"
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
        0,//(newfunc)Python27Geom::newfunc,       /* tp_new */
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



void python27_initGeometryModule()
{
    PyObject* module = static_cast<PyObject*>(getPython27Module());
    if (!module)
        MO_ERROR("Can't access Python 2.7 module for creating Geometry");

    // setup an object
    Python27Geom_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&Python27Geom_type) < 0)
        MO_ERROR("Failed to readify Python27Geom");

    Py_INCREF(&Python27Geom_type);
    PyModule_AddObject(module, "Geometry", (PyObject*)&Python27Geom_type);
}


} // namespace MO

#endif // MO_ENABLE_PYTHON27

