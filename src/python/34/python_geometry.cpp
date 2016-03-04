/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include "py_utils.h"
#ifdef MO_ENABLE_NUMPY
#   define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#   include <numpy/arrayobject.h>
#endif

#include "python_geometry.h"
#include "python_vector.h"
#include "python.h"
#include "geom/geometry.h"
#include "math/vector.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {


namespace
{
    bool checkIndex(long idx, long size)
    {
        if (idx < 0 || idx >= size)
        {
            PyErr_SetString(PyExc_IndexError,
                            QString("index out of range %1/%2")
                            .arg(idx).arg(size).toUtf8().constData());
            return false;
        }
        return true;
    }


    // all vector(3) convertible arguments
    bool py_get_vec3(PyObject* arg, Vec3* vec)
    {
        double v[3];
        if (get_vector(arg, 3, v))
        {
            vec->x = v[0];
            vec->y = v[1];
            vec->z = v[2];
            return true;
        }
        return false;
    }

    // either (long, f,f,f) or (long, vec-convertible)
    bool py_get_index_and_vec3(PyObject* args_, long* idx, Vec3* v)
    {
        if (PyArg_ParseTuple(args_, "l|fff", idx, &v->x, &v->y, &v->z))
            return true;
        PyErr_Clear();
        PyObject * second;
        if (!PyArg_ParseTuple(args_, "l|O", idx, &second))
            return false;
        if (py_get_vec3(second, v))
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

    bool py_array_or_tuple_to_uint3(PyObject* obj,
                                    unsigned long* v1, unsigned long* v2,
                                    unsigned long* v3)
    {
        if (PyArg_ParseTuple(obj, "(kkk)", v1, v2, v3))
            return true;
        PyErr_Clear();
        if (PyArg_ParseTuple(obj, "kkk", v1, v2, v3))
            return true;
        return false;
    }

    bool py_get_index_make_impl(GEOM::Geometry* geom, int num,
                                PyObject* arg[], long idx[])
    {
        for (int i=0; i<num; ++i)
        {
            double v[4];
            int vlen;
            if (getVector(arg[i], &vlen, v))
            {
                if (vlen != 3)
                {
                    PyErr_Set(PyExc_TypeError,
                              QString("expected vector of length 3 not %1 "
                                      "for argument #%2").arg(vlen).arg(i+1));
                    return false;
                }
                idx[i] = geom->addVertex(v[0], v[1], v[2]);
            }
            else if (PyLong_Check(arg[i]))
            {
                idx[i] = PyLong_AsLong(arg[i]);
                if (!checkIndex(idx[i], geom->numVertices()))
                    return false;
            }
            else
            {
                PyErr_Set(PyExc_TypeError,
                          QString("expected vector or index at argument #%1, "
                                  "got %2 instead").arg(i+1).arg(typeName(arg[i])));
                return false;
            }
        }
        return true;
    }

    // returns num indices for either long arguments or vec3s wich
    // implicitly create indices in the geometry
    bool py_get_index_make(GEOM::Geometry* geom, int num, PyObject* args_,
                           long idx[])
    {
        PyObject* arg[num];
        if (num == 1)
        {
            if (!PyArg_ParseTuple(args_, "O", &arg[0]))
                return false;
        }
        else if (num == 2)
        {
            if (!PyArg_ParseTuple(args_, "OO", &arg[0], &arg[1]))
                return false;
        }
        else if (num == 3)
        {
            if (!PyArg_ParseTuple(args_, "OOO", &arg[0], &arg[1], &arg[2]))
                return false;
        }
        else if (num == 4)
        {
            if (!PyArg_ParseTuple(args_, "OOOO", &arg[0], &arg[1], &arg[2], &arg[4]))
                return false;
        }
        return py_get_index_make_impl(geom, num, arg, idx);
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


#define MO_PY_DEF_DOC(name__, str__) \
    static const char* name__##_doc = str__;

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

    // ---------- getter ---------------

    MO_PY_DEF_DOC(geom_to_string,
        "to_string() -> str\n"
        "Returns an informative string"
    )
    static PyObject* geom_to_string(PyObject* self, PyObject* )
    {
        MO__GETGEOM0(pgeom);
        if (!pgeom->geometry)
            return Py_BuildValue("s", "*empty*");
        auto str = pgeom->geometry->infoString();
        str += QString(" (refcnt:%1)").arg(self->ob_refcnt);
        return Py_BuildValue("s", str.toLatin1().constData());
    }
    static PyObject* geom_repr(PyObject* self) { return geom_to_string(self, nullptr); }

    MO_PY_DEF_DOC(geom_num_vertices,
        "num_vertices() -> long\n"
        "Returns the number of vertices"
    )
    static PyObject* geom_num_vertices(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        return Py_BuildValue("n", pgeom->geometry->numVertices());
    }

    MO_PY_DEF_DOC(geom_get_vertex,
        "get_vertex(long) -> Vec\n"
        "Returns the vertex position at the given index as 3d vector"
    )
    static PyObject* geom_get_vertex(PyObject* self, PyObject* arg)
    {
        long idx;
        if (!PyArg_ParseTuple(arg, "l", &idx))
            return NULL;
        MO__GETGEOM(p);
        if (!checkIndex(idx, p->geometry->numVertices()))
            return NULL;
        return buildVector(p->geometry->getVertex(idx));
    }

    MO_PY_DEF_DOC(geom_is_shared,
        "is_shared() -> bool\n"
        "Returns the current shared-vertex-mode."
    )
    static PyObject* geom_is_shared(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        return PyBool_FromLong(pgeom->geometry->sharedVertices());
    }

    // -------------- setter ----------------

    MO_PY_DEF_DOC(geom_set_shared,
        "set_shared() -> None\n"
        "set_shared(bool, float) -> None\n"
        "Decides whether the geometry should reuse existing vertex points (shared)\n"
        "or create different vertex points even for the same points in space (unshared).\n"
        "Unshared mode might be necessary for correct texture mapping while shared mode\n"
        "can greatly reduce the memory footprint of a mesh.\n"
        "Optionally, a minimum distance can be given (defaults to 0.001) below which\n"
        "vertex positions are considered 'the same' in shared mode.\n"
        "When no argument is given, True and 0.001 is assumed.\n"
        "Changing the shared-state does not affect previously created verices."
    )
    static PyObject* geom_set_shared(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        int is_true = true;
        float thresh = GEOM::Geometry::minimumThreshold;
        if (!PyArg_ParseTuple(arg, "|pf", &is_true, &thresh))
            return NULL;
        pgeom->geometry->setSharedVertices(is_true, thresh);
        Py_RETURN_NONE;
    }


    MO_PY_DEF_DOC(geom_add_vertex,
        "add_vertex(vector3) -> long\n"
        "Adds a new vertex. The returned value is the index of the vertex.\n"
        "For unshared geometries this value is equal to num_vertices() - 1,\n"
        "for shared geometries the vertex might have been present already\n"
        "and it's index is returned."
    )
    static PyObject* geom_add_vertex(PyObject* self, PyObject* arg)
    {
        Vec3 v;
        if (!py_get_vec3(arg, &v))
            return NULL;
        MO__GETGEOM(pgeom);
        auto i = pgeom->geometry->addVertex(v);
        return Py_BuildValue("n", i);
    }

    MO_PY_DEF_DOC(geom_add_vertices,
        "add_vertices(vector3 list) -> None\n"
        "Adds the list of vertices.\n"
    )
    static PyObject* geom_add_vertices(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        auto foo = [pgeom](PyObject* item)
        {
            MO_PRINT("ITEM[" << typeName(item) << "]");
            Vec3 v;
            if (!py_get_vec3(item, &v))
                return false;
            pgeom->geometry->addVertex(v);
            return true;
        };
        arg = removeArgumentTuple(arg);
        if (!iterateSequence(arg, foo))
            return NULL;
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_set_vertex,
        "set_vertex(long, vec3) -> None\n"
        "Changes the vertex position at the given index."
    )
    static PyObject* geom_set_vertex(PyObject* self, PyObject* arg)
    {
        Vec3 v;
        long idx;
        if (!py_get_index_and_vec3(arg, &idx, &v))
            return NULL;
        MO__GETGEOM(pgeom);
        if (!checkIndex(idx, pgeom->geometry->numVertices()))
            return NULL;
        pgeom->geometry->setVertex(idx, v);
        Py_RETURN_NONE;
    }


    MO_PY_DEF_DOC(geom_add_point,
        "add_point(long) -> None\n"
        "add_point(vec3) -> None\n"
        "Adds a point at the vertex position.\n"
        "Argument can be either index as returned by add_vertex() or 3d vector\n"
        "In case of a vector, the vertex is created/reused as if called add_vertex()"
    )
    static PyObject* geom_add_point(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        long idx;
        if (!py_get_index_make(pgeom->geometry, 1, arg, &idx))
            return NULL;
        pgeom->geometry->addPoint(idx);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_add_line,
        "add_line(long, long) -> None\n"
        "add_line(vec3, vec3) -> None\n"
        "Adds a line between two vertex positions.\n"
        "Arguments can be either indices as returned by add_vertex() or 3d vectors\n"
        "In case of vectors, the vertices are created/reused as if called add_vertex()"
    )
    static PyObject* geom_add_line(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        long idx[2];
        if (!py_get_index_make(pgeom->geometry, 2, arg, idx))
            return NULL;
        pgeom->geometry->addLine(idx[0], idx[1]);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_add_triangle,
        "add_triangle(long, long, long) -> None\n"
        "add_triangle(vec3, vec3, vec3) -> None\n"
        "Adds a triangle between the three vertex positions.\n"
        "Arguments can be either indices as returned by add_vertex() or 3d vectors\n"
        "In case of vectors, the vertices are created/reused as if called add_vertex()"
    )
    static PyObject* geom_add_triangle(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        long idx[3];
        if (!py_get_index_make(pgeom->geometry, 3, arg, idx))
            return NULL;
        pgeom->geometry->addTriangle(idx[0], idx[1], idx[2]);
        Py_RETURN_NONE;
    }

#undef MO__GETGEOM
#undef MO__GETGEOM0



    PyMemberDef Python34Geom_members[] =
    {
        { NULL, 0, 0, 0, NULL }
    };

#define MO__METHOD(name__, args__) \
    { #name__, (PyCFunction)geom_##name__, args__, geom_##name__##_doc },

    static PyMethodDef Python34Geom_methods[] =
    {
        MO__METHOD(to_string,       METH_NOARGS)
        MO__METHOD(is_shared,       METH_NOARGS)
        MO__METHOD(num_vertices,    METH_NOARGS)

        MO__METHOD(set_shared,      METH_VARARGS)
        MO__METHOD(get_vertex,      METH_VARARGS)
        MO__METHOD(add_vertex,      METH_VARARGS)
        MO__METHOD(set_vertex,      METH_VARARGS)
        MO__METHOD(add_vertices,    METH_VARARGS)
        MO__METHOD(add_point,       METH_VARARGS)
        MO__METHOD(add_line,        METH_VARARGS)
        MO__METHOD(add_triangle,    METH_VARARGS)


        { NULL, NULL, 0, NULL }
    };
#undef MO__METHOD


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
        geom_repr,                /*tp_str*/
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



void initGeometry(void* mod)
{
    PyObject* module = reinterpret_cast<PyObject*>(mod);
    initObjectType(module, &Python34Geom_type, "Geometry");
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

