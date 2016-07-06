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
#include "geom/geometryfactory.h"
#include "math/vector.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {


namespace
{
    bool checkIndex(long idx, long size, const QString& prefix="")
    {
        if (idx < 0 || idx >= size)
        {
            QString str = QString("index out of range %1/%2")
                    .arg(idx).arg(size);
            if (!prefix.isEmpty())
                str.prepend(prefix + " ");
            PyErr_Set(PyExc_IndexError, str);
            return false;
        }
        return true;
    }


    // all vector(2) convertible arguments
    bool py_get_vec2(PyObject* arg, Vec2* vec)
    {
        double v[2];
        if (get_vector(arg, 2, v))
        {
            vec->x = v[0];
            vec->y = v[1];
            return true;
        }
        return false;
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

    // all vector(4) convertible arguments
    bool py_get_vec4(PyObject* arg, Vec4* vec)
    {
        double v[4];
        if (get_vector(arg, 4, v))
        {
            vec->x = v[0];
            vec->y = v[1];
            vec->z = v[2];
            vec->w = v[3];
            return true;
        }
        return false;
    }


    // either (long, f,f) or (long, vec-convertible)
    bool py_get_index_and_vec2(PyObject* args_, long* idx, Vec2* v)
    {
        if (PyArg_ParseTuple(args_, "l|ff", idx, &v->x, &v->y))
            return true;
        PyErr_Clear();
        PyObject * second;
        if (!PyArg_ParseTuple(args_, "l|O", idx, &second))
            return false;
        if (py_get_vec2(second, v))
            return true;
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

    // either (long, f,f,f,f) or (long, vec-convertible)
    bool py_get_index_and_vec4(PyObject* args_, long* idx, Vec4* v)
    {
        if (PyArg_ParseTuple(args_, "l|ffff", idx, &v->x, &v->y, &v->z, &v->w))
            return true;
        PyErr_Clear();
        PyObject * second;
        if (!PyArg_ParseTuple(args_, "l|O", idx, &second))
            return false;
        if (py_get_vec4(second, v))
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
                if (!checkIndex(idx[i], geom->numVertices(),
                                QString("argument #%1").arg(i+1)))
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
                self->geometry->releaseRef("py geometry destroy");
            self->ob_base.ob_type->tp_free((PyObject*)self);
        }

        static int init(Python34Geom* self, PyObject* args, PyObject*)
        {
            PyObject* obj = 0;
            PyArg_ParseTuple(args, "|O", &obj);
            if (obj && isGeometry(obj))
            {
                auto other = reinterpret_cast<Python34Geom*>(obj);
                other->geometry->addRef("py geometry.__init__ copyctor");
                if (self->geometry)
                    self->geometry->releaseRef("py geometry.__init__ relprev");
                self->geometry = other->geometry;
            }
            else
            {
                if (self->geometry)
                    self->geometry->releaseRef("py geometry.__init__ relprev");
                self->geometry = new GEOM::Geometry;
            }

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

    MO_PY_DEF_DOC(geom_is_shared,
        "is_shared() -> bool\n"
        "Returns the current shared-vertex-mode."
    )
    static PyObject* geom_is_shared(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        return PyBool_FromLong(pgeom->geometry->sharedVertices());
    }

    MO_PY_DEF_DOC(geom_is_empty,
        "is_empty() -> bool\n"
        "Returns True if no primitive is created (regardless of vertices)"
    )
    static PyObject* geom_is_empty(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        return PyBool_FromLong(pgeom->geometry->isEmpty());
    }

    MO_PY_DEF_DOC(geom_num_vertices,
        "num_vertices() -> long\n"
        "Returns the number of vertices"
    )
    static PyObject* geom_num_vertices(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        return Py_BuildValue("n", pgeom->geometry->numVertices());
    }

    MO_PY_DEF_DOC(geom_num_points,
        "num_points() -> long\n"
        "Returns the number of points"
    )
    static PyObject* geom_num_points(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        return Py_BuildValue("n", pgeom->geometry->numPoints());
    }

    MO_PY_DEF_DOC(geom_num_lines,
        "num_lines() -> long\n"
        "Returns the number of lines"
    )
    static PyObject* geom_num_lines(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        return Py_BuildValue("n", pgeom->geometry->numLines());
    }

    MO_PY_DEF_DOC(geom_num_triangles,
        "num_triangles() -> long\n"
        "Returns the number of triangles"
    )
    static PyObject* geom_num_triangles(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        return Py_BuildValue("n", pgeom->geometry->numTriangles());
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
        if (!checkIndex(idx, p->geometry->numVertices(), "vertex"))
            return NULL;
        return buildVector(p->geometry->getVertex(idx));
    }

    MO_PY_DEF_DOC(geom_get_color,
        "get_color(long) -> Vec\n"
        "Returns the color of the vertex at the given index as 4d vector"
    )
    static PyObject* geom_get_color(PyObject* self, PyObject* arg)
    {
        long idx;
        if (!PyArg_ParseTuple(arg, "l", &idx))
            return NULL;
        MO__GETGEOM(p);
        if (!checkIndex(idx, p->geometry->numVertices(), "vertex"))
            return NULL;
        return buildVector(p->geometry->getColor(idx));
    }


    MO_PY_DEF_DOC(geom_get_normal,
        "get_normal(long) -> Vec\n"
        "Returns the normal of the vertex at the given index as 3d vector"
    )
    static PyObject* geom_get_normal(PyObject* self, PyObject* arg)
    {
        long idx;
        if (!PyArg_ParseTuple(arg, "l", &idx))
            return NULL;
        MO__GETGEOM(p);
        if (!checkIndex(idx, p->geometry->numVertices(), "vertex"))
            return NULL;
        return buildVector(p->geometry->getNormal(idx));
    }

    MO_PY_DEF_DOC(geom_get_tex_coord,
        "get_tex_coord(long) -> Vec\n"
        "Returns the texture coords of the vertex at the given index as 2d vector"
    )
    static PyObject* geom_get_tex_coord(PyObject* self, PyObject* arg)
    {
        long idx;
        if (!PyArg_ParseTuple(arg, "l", &idx))
            return NULL;
        MO__GETGEOM(p);
        if (!checkIndex(idx, p->geometry->numVertices(), "vertex"))
            return NULL;
        return buildVector(p->geometry->getTexCoord(idx));
    }

    MO_PY_DEF_DOC(geom_get_attribute,
        "get_attribute(str, long) -> float | Vec\n"
        "Returns the vertex attribute of the vertex at the given index as float "
        "or as 2-4d vector.\n"
        "The attribute name is specified as the first parameter.\n"
        "If the attribute does not exist, None is returned!"
    )
    static PyObject* geom_get_attribute(PyObject* self, PyObject* arg)
    {
        long idx;
        const char * utf8;
        if (!PyArg_ParseTuple(arg, "sl", &utf8, &idx))
            return NULL;
        MO__GETGEOM(p);
        if (!checkIndex(idx, p->geometry->numVertices(), "vertex"))
            return NULL;
        auto attName = QString::fromUtf8(utf8);
        auto att = p->geometry->getAttribute(attName);
        if (!att)
            Py_RETURN_NONE;
        if (att->numComponents > 4)
        {
            PyErr_Set(PyExc_TypeError, QString("attribute '%1' has invalid component "
                                               "size %2").arg(attName).arg(att->numComponents));
            return NULL;
        }
        double v[4];
        for (unsigned int i=0; i<att->numComponents; ++i)
            v[i] = att->value(idx, i);
        return buildVector(v, att->numComponents);
    }


    MO_PY_DEF_DOC(geom_get_point,
        "get_point(long) -> Vec\n"
        "Returns the position of the point at the given index."
    )
    static PyObject* geom_get_point(PyObject* self, PyObject* arg)
    {
        long idx;
        if (!PyArg_ParseTuple(arg, "l", &idx))
            return NULL;
        MO__GETGEOM(p);
        if (!checkIndex(idx, p->geometry->numPoints(), "point"))
            return NULL;
        return buildVector(p->geometry->point(idx), 3);
    }

    MO_PY_DEF_DOC(geom_get_line,
        "get_line(long) -> (Vec, Vec)\n"
        "Returns the endpoints positions of the line at the given index\n"
        "as tuple of 3d vectors."
    )
    static PyObject* geom_get_line(PyObject* self, PyObject* arg)
    {
        long idx;
        if (!PyArg_ParseTuple(arg, "l", &idx))
            return NULL;
        MO__GETGEOM(p);
        if (!checkIndex(idx, p->geometry->numLines(), "line"))
            return NULL;
        return Py_BuildValue("(OO)",
                             buildVector(p->geometry->line(idx, 0), 3),
                             buildVector(p->geometry->line(idx, 1), 3)
                             );
    }

    MO_PY_DEF_DOC(geom_get_triangle,
        "get_triangle(long) -> (Vec, Vec, Vec)\n"
        "Returns the corner point positions of the triangle at the given index\n"
        "as tuple of 3d vectors."
    )
    static PyObject* geom_get_triangle(PyObject* self, PyObject* arg)
    {
        long idx;
        if (!PyArg_ParseTuple(arg, "l", &idx))
            return NULL;
        MO__GETGEOM(p);
        if (!checkIndex(idx, p->geometry->numTriangles(), "triangle"))
            return NULL;
        return Py_BuildValue("(OOO)",
                             buildVector(p->geometry->triangle(idx, 0), 3),
                             buildVector(p->geometry->triangle(idx, 1), 3),
                             buildVector(p->geometry->triangle(idx, 2), 3)
                             );
    }

    MO_PY_DEF_DOC(geom_intersection,
        "intersects(Vec ro, dir) -> float\n"
        "Returns the distance from ro to the closest intersection with one\n"
        "of the triangles of the geometry, or a negative value when not hit"
    )
    static PyObject* geom_intersection(PyObject* self, PyObject* arg)
    {
        PyObject* vo1, *vo2;
        if (!PyArg_ParseTuple(arg, "OO", &vo1, &vo2))
            return NULL;
        double v1[3], v2[3];
        if (   !get_vector(vo1, 3, v1)
            || !get_vector(vo2, 3, v2))
            return NULL;
        MO__GETGEOM(p);
        Vec3 ro(v1[0], v1[1], v1[2]),
             rd(v2[0], v2[1], v2[2]), hit;
        if (!p->geometry->intersects(ro, rd, &hit))
            return PyFloat_FromDouble(-1.);
        return PyFloat_FromDouble(glm::distance(ro, hit));
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
            //MO_PRINT("ITEM[" << typeName(item) << "]");
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

    MO_PY_DEF_DOC(geom_add_geometry,
        "add_geometry(Geometry) -> long\n"
        "Adds the given Geometry data to self."
    )
    static PyObject* geom_add_geometry(PyObject* self, PyObject* arg, PyObject*)
    {
        MO__GETGEOM(pgeom);
        PyObject* other;
        PyArg_ParseTuple(arg, "O", &other);
        if (!isGeometry(other))
        {
            PyErr_Set(PyExc_TypeError, QString(
                          "expected Geometry, got %1").arg(typeName(other)));
            return NULL;
        }
        pgeom->geometry->addGeometry(
                    *(reinterpret_cast<Python34Geom*>(other)->geometry) );
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_add_geometry_only,
        "add_geometry_only(Geometry, bools..) -> long\n"
        "Adds only the selected data from the given Geometry to self.\n"
        "bools.. match the following keywords:\n"
        "triangle, line, point,\n"
        "color, normal, tex_coord\n"
        "By default, all are disabled, and the current value in self will be used."
    )
    static PyObject* geom_add_geometry_only(PyObject* self, PyObject* arg, PyObject* kwds)
    {
        MO__GETGEOM(pgeom);

        PyObject* other;
        int doTri = 0, doLine = 0, doPoint = 0,
            doColor = 0, doNormal = 0, doTex = 0;
        static const char* keywords[] =
        {
            "", "triangle", "line", "point",
            "color", "normal", "tex_coord", NULL
        };
        if (!PyArg_ParseTupleAndKeywords(
                    arg, kwds, "O|pppppp", const_cast<char**>(keywords),
                    &other,
                    &doTri, &doLine, &doPoint,
                    &doColor, &doNormal, &doTex))
            return NULL;

        if (!isGeometry(other))
        {
            PyErr_Set(PyExc_TypeError, QString(
                          "expected Geometry, got %1").arg(typeName(other)));
            return NULL;
        }
        pgeom->geometry->addGeometry(
                    *(reinterpret_cast<Python34Geom*>(other)->geometry),
                    Vec3(0),
                    doTri, doLine, doPoint, doColor, doNormal, doTex, false);
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
        if (!checkIndex(idx, pgeom->geometry->numVertices(), "vertex"))
            return NULL;
        pgeom->geometry->setVertex(idx, v);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_set_color,
        "set_color(long, vec4) -> None\n"
        //"First version sets the current color, used on the next call to add_vertex()\n"
        "Changes the color of the vertex at the given index."
    )
    static PyObject* geom_set_color(PyObject* self, PyObject* arg)
    {
        Vec4 v;
        long idx;
        if (!py_get_index_and_vec4(arg, &idx, &v))
            return NULL;
        MO__GETGEOM(pgeom);
        if (!checkIndex(idx, pgeom->geometry->numVertices(), "vertex"))
            return NULL;
        pgeom->geometry->setColor(idx, v);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_set_normal,
        "set_normal(long, vec3) -> None\n"
        "Changes the normal of the vertex at the given index."
    )
    static PyObject* geom_set_normal(PyObject* self, PyObject* arg)
    {
        Vec3 v;
        long idx;
        if (!py_get_index_and_vec3(arg, &idx, &v))
            return NULL;
        MO__GETGEOM(pgeom);
        if (!checkIndex(idx, pgeom->geometry->numVertices(), "vertex"))
            return NULL;
        pgeom->geometry->setNormal(idx, v);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_set_tex_coord,
        "set_tex_coord(long, vec2) -> None\n"
        "Changes the texture coords of the vertex at the given index."
    )
    static PyObject* geom_set_tex_coord(PyObject* self, PyObject* arg)
    {
        Vec2 v;
        long idx;
        if (!py_get_index_and_vec2(arg, &idx, &v))
            return NULL;
        MO__GETGEOM(pgeom);
        if (!checkIndex(idx, pgeom->geometry->numVertices(), "vertex"))
            return NULL;
        pgeom->geometry->setTexCoord(idx, v);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_set_attribute,
        "set_attribute(str, long, vec) -> None\n"
        "Changes the vertex attribute of the vertex at the given index.\n"
        "The vector can be 1 to 4 components long. "
    )
    static PyObject* geom_set_attribute(PyObject* self, PyObject* arg)
    {
        const char* utf8;
        long idx;
        PyObject * second;
        if (!PyArg_ParseTuple(arg, "slO", &utf8, &idx, &second))
            return NULL;
        MO__GETGEOM(pgeom);
        auto attName = QString::fromUtf8(utf8);
        if (attName.isEmpty())
        {
            PyErr_SetString(PyExc_TypeError, "empty attribute name");
            return NULL;
        }
        if (!checkIndex(idx, pgeom->geometry->numVertices(), "vertex"))
            return NULL;
        int num;
        double v[4];
        if (!get_vector_var(second, &num, v))
            return NULL;
        auto att = pgeom->geometry->getAttribute(attName);
        // create attribute if not there already
        if (!att)
            att = pgeom->geometry->addAttribute(attName, num);
        if ((int)att->numComponents != num)
        {
            PyErr_Set(PyExc_TypeError, QString("attribute '%1' is already "
                                               "defined with %2 component(s), not %3")
                      .arg(attName).arg(att->numComponents).arg(num));
            return NULL;
        }
        pgeom->geometry->setAttribute(attName, idx, v[0], v[1], v[2], v[3]);
        Py_RETURN_NONE;
    }


    MO_PY_DEF_DOC(geom_set_cur_color,
        "set_cur_color(vec4) -> None\n"
        "Sets the current color, used on the next call to add_vertex()\n"
    )
    static PyObject* geom_set_cur_color(PyObject* self, PyObject* arg)
    {
        Vec4 v;
        if (!py_get_vec4(arg, &v))
            return NULL;
        MO__GETGEOM(pgeom);
        pgeom->geometry->setColor(v.x, v.y, v.z, v.w);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_set_cur_normal,
        "set_cur_normal(vec3) -> None\n"
        "Sets the current normal, used on the next call to add_vertex()\n"
    )
    static PyObject* geom_set_cur_normal(PyObject* self, PyObject* arg)
    {
        Vec3 v;
        if (!py_get_vec3(arg, &v))
            return NULL;
        MO__GETGEOM(pgeom);
        pgeom->geometry->setNormal(v.x, v.y, v.z);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_set_cur_tex_coord,
        "set_cur_tex_coord(vec2) -> None\n"
        "Sets the current texture, used on the next call to add_vertex()\n"
    )
    static PyObject* geom_set_cur_tex_coord(PyObject* self, PyObject* arg)
    {
        Vec2 v;
        if (!py_get_vec2(arg, &v))
            return NULL;
        MO__GETGEOM(pgeom);
        pgeom->geometry->setTexCoord(v.x, v.y);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_set_cur_attribute,
        "set_cur_attribute(str, vec) -> None\n"
        "Sets the current user attribute, used on the next call to add_vertex()\n"
        "First argument is the name of the attribute.\n"
        "A vertex shader attribute can then be accessed with the same name.\n"
        "The vector can be 1 to 4 components long.\n"
        "If the attribute is already defined, the vector size must match."
    )
    static PyObject* geom_set_cur_attribute(PyObject* self, PyObject* arg)
    {
        const char* utf8;
        PyObject * second;
        if (!PyArg_ParseTuple(arg, "sO", &utf8, &second))
            return NULL;
        MO__GETGEOM(pgeom);
        auto attName = QString::fromUtf8(utf8);
        if (attName.isEmpty())
        {
            PyErr_SetString(PyExc_TypeError, "empty attribute name");
            return NULL;
        }
        int num;
        double v[4];
        if (!get_vector_var(second, &num, v))
            return NULL;
        auto att = pgeom->geometry->getAttribute(attName);
        // create attribute if not there already
        if (!att)
            att = pgeom->geometry->addAttribute(attName, num);
        if ((int)att->numComponents != num)
        {
            PyErr_Set(PyExc_TypeError, QString("attribute '%1' is already "
                                               "defined with %2 component(s), not %3")
                      .arg(attName).arg(att->numComponents).arg(num));
            return NULL;
        }
        pgeom->geometry->setAttribute(
                    attName, float(v[0]), float(v[1]), float(v[2]), float(v[3]));
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
        "add_triangle(long, long, long) -> long\n"
        "add_triangle(vec3, vec3, vec3) -> long\n"
        "Adds a triangle between the three vertex positions.\n"
        "Positions should be ordered counter-clockwise.\n"
        "Arguments can be either indices as returned by add_vertex() or 3d vectors\n"
        "In case of vectors, the vertices are created/reused as if called add_vertex()\n"
        "Returns the primitive index of the triangle (created or reused)"
    )
    static PyObject* geom_add_triangle(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        long idx[3];
        if (!py_get_index_make(pgeom->geometry, 3, arg, idx))
            return NULL;
        return PyLong_FromLong(
                    pgeom->geometry->addTriangle(idx[0], idx[1], idx[2]) );
    }

    MO_PY_DEF_DOC(geom_add_quad,
        "add_quad(long, long, long, long) -> None\n"
        "add_quad(vec3, vec3, vec3, vec3) -> None\n"
        "Adds a quad between the four vertex positions.\n"
        "Positions should be ordered counter-clockwise.\n"
        "Arguments can be either indices as returned by add_vertex() or 3d vectors\n"
        "In case of vectors, the vertices are created/reused as if called add_vertex()"
    )
    static PyObject* geom_add_quad(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        long idx[4];
        if (!py_get_index_make(pgeom->geometry, 4, arg, idx))
            return NULL;
        pgeom->geometry->addQuad(idx[0], idx[1], idx[2], idx[3]);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_add_box,
        "add_box(float | vec3) -> None\n"
        "Adds a cube or box with the given side length(s)."
    )
    static PyObject* geom_add_box(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        double v[4];
        int len;
        if (!get_vector_var(arg, &len, v))
            return NULL;
        if (len == 1)
            v[2] = v[1] = v[0];
        else if (len != 3)
            { PyErr_Set(PyExc_TypeError, QString("expect 1 or 3 component vector, got").arg(len)); return NULL; }
        if (!pgeom->geometry->sharedVertices())
            GEOM::GeometryFactory::createTexturedBox(pgeom->geometry, v[0], v[1], v[2]);
        else
            GEOM::GeometryFactory::createBox(pgeom->geometry, v[0], v[1], v[2]);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_add_icosahedron,
        "add_icosahederon(float) -> None\n"
        "Adds an icosahedron with the given size."
    )
    static PyObject* geom_add_icosahedron(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        float size;
        if (!PyArg_ParseTuple(arg, "f", &size))
            return NULL;
        GEOM::GeometryFactory::createIcosahedron(pgeom->geometry, size);
        Py_RETURN_NONE;
    }



    MO_PY_DEF_DOC(geom_clear,
        "clear() -> None\n"
        "Completely wipes out all data."
    )
    static PyObject* geom_clear(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        pgeom->geometry->clear();
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_copy,
        "copy() -> Geometry\n"
        "Creates a deep copy of all data"
    )
    static PyObject* geom_copy(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        auto geom = new GEOM::Geometry(*pgeom->geometry);
        return reinterpret_cast<PyObject*>(createGeometryObject(geom));
    }

    MO_PY_DEF_DOC(geom_calc_normals,
        "calc_normals() -> None\n"
        "Calculates the vertex normals from the triangle data.\n"
        "Does nothing, if no triangles are present."
    )
    static PyObject* geom_calc_normals(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        pgeom->geometry->calculateTriangleNormals();
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_invert_normals,
        "invert_normals() -> None\n"
        "Negates the vertex normals, which essentially turns the lighting inside-out."
    )
    static PyObject* geom_invert_normals(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        pgeom->geometry->invertNormals();
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_tesselate_triangles,
        "tesselate_triangles(int, float, float) -> None\n"
        "Tesselates all triangles of the geometry.\n"
        "All arguments are optional.\n"
        "The first is the level of tesselation, defaulting to one.\n"
        "Second is the smallest triangle area considered for tesselation, default 0.\n"
        "Third is the shortest length of a triangle edge that is considered for\n"
        "tesselation. Each triangle edge is considered individually."
    )
    static PyObject* geom_tesselate_triangles(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        int level = 1;
        float minArea = 0., minLength = 0.;
        if (!PyArg_ParseTuple(arg, "|iff", &level, &minArea, &minLength))
            return NULL;
        if (minArea <= 0. && minLength <= 0.)
            pgeom->geometry->tesselateTriangles(level);
        else
            pgeom->geometry->tesselateTriangles(minArea, minLength, level);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_tesselate_triangle,
        "tesselate_triangle(int, int) -> None\n"
        "Tesselates one triangle given by the primitive index (first argument).\n"
        "The second arg is the level of tesselation, defaulting to one."
    )
    static PyObject* geom_tesselate_triangle(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        long idx;
        int level = 1;
        if (!PyArg_ParseTuple(arg, "l|i", &idx, &level))
            return NULL;
        pgeom->geometry->tesselateTriangle(idx, level);
        Py_RETURN_NONE;
    }


    MO_PY_DEF_DOC(geom_tesselate_lines,
        "tesselate_lines(int) -> None\n"
        "Splits all lines into segments.\n"
        "The optional argument is the level of tesselation, defaulting to one."
    )
    static PyObject* geom_tesselate_lines(PyObject* self, PyObject* arg)
    {
        MO__GETGEOM(pgeom);
        int level = 1;
        if (!PyArg_ParseTuple(arg, "|i", &level))
            return NULL;
        pgeom->geometry->tesselateLines(level);
        Py_RETURN_NONE;
    }


    MO_PY_DEF_DOC(geom_convert_to_lines,
        "convert_to_lines() -> None\n"
        "Converts all triangles into line data.\n"
        "Subsequently, all triangles are removed from the geometry."
    )
    static PyObject* geom_convert_to_lines(PyObject* self, PyObject* )
    {
        MO__GETGEOM(pgeom);
        pgeom->geometry->convertToLines();
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_scale,
        "scale(float|vec3) -> None\n"
        "Multiply vertex position by factor."
    )
    static PyObject* geom_scale(PyObject* self, PyObject* args)
    {
        MO__GETGEOM(pgeom);
        int len;
        double v[4];
        if (!get_vector_var(args, &len, v))
            return NULL;
        if (len == 1)
            v[2] = v[1] = v[0];
        else if (len == 2)
            v[2] = 1;
        else if (len != 3)
        {
            PyErr_Set(PyExc_TypeError, QString("expected 1 - 3 component vector, "
                                               "got %1").arg(len));
            return NULL;
        }
        pgeom->geometry->scale(v[0], v[1], v[2]);
        Py_RETURN_NONE;
    }

    MO_PY_DEF_DOC(geom_translate,
        "translate(vec) -> None\n"
        "Add offset to vertex positions. Argument size can be 1-3."
    )
    static PyObject* geom_translate(PyObject* self, PyObject* args)
    {
        MO__GETGEOM(pgeom);
        int len;
        double v[4] { 0,0,0,0 };
        if (!get_vector_var(args, &len, v))
            return NULL;
        if (len > 3)
        {
            PyErr_Set(PyExc_TypeError, QString("expected 1 - 3 component vector, "
                                               "got %1").arg(len));
            return NULL;
        }
        pgeom->geometry->translate(v[0], v[1], v[2]);
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
        MO__METHOD(to_string,               METH_NOARGS)
        MO__METHOD(is_empty,                METH_NOARGS)
        MO__METHOD(is_shared,               METH_NOARGS)
        MO__METHOD(num_vertices,            METH_NOARGS)
        MO__METHOD(num_points,              METH_NOARGS)
        MO__METHOD(num_lines,               METH_NOARGS)
        MO__METHOD(num_triangles,           METH_NOARGS)

        MO__METHOD(get_vertex,              METH_VARARGS)
        MO__METHOD(get_color,               METH_VARARGS)
        MO__METHOD(get_normal,              METH_VARARGS)
        MO__METHOD(get_tex_coord,           METH_VARARGS)
        MO__METHOD(get_attribute,           METH_VARARGS)
        MO__METHOD(get_point,               METH_VARARGS)
        MO__METHOD(get_line,                METH_VARARGS)
        MO__METHOD(get_triangle,            METH_VARARGS)

        MO__METHOD(intersection,            METH_VARARGS)

        MO__METHOD(set_shared,              METH_VARARGS)
        MO__METHOD(set_vertex,              METH_VARARGS)
        MO__METHOD(set_color,               METH_VARARGS)
        MO__METHOD(set_normal,              METH_VARARGS)
        MO__METHOD(set_tex_coord,           METH_VARARGS)
        MO__METHOD(set_attribute,           METH_VARARGS)

        MO__METHOD(set_cur_color,           METH_VARARGS)
        MO__METHOD(set_cur_normal,          METH_VARARGS)
        MO__METHOD(set_cur_tex_coord,       METH_VARARGS)
        MO__METHOD(set_cur_attribute,       METH_VARARGS)

        MO__METHOD(add_vertex,              METH_VARARGS)
        MO__METHOD(add_vertices,            METH_VARARGS)
        MO__METHOD(add_point,               METH_VARARGS)
        MO__METHOD(add_line,                METH_VARARGS)
        MO__METHOD(add_triangle,            METH_VARARGS)
        MO__METHOD(add_quad,                METH_VARARGS)
        MO__METHOD(add_geometry,            METH_VARARGS)
        MO__METHOD(add_geometry_only,       METH_VARARGS | METH_KEYWORDS)

        MO__METHOD(add_box,                 METH_VARARGS)
        MO__METHOD(add_icosahedron,         METH_VARARGS)

        MO__METHOD(clear,                   METH_NOARGS)
        MO__METHOD(copy,                    METH_NOARGS)
        MO__METHOD(calc_normals,            METH_NOARGS)
        MO__METHOD(invert_normals,          METH_NOARGS)

        MO__METHOD(scale,                   METH_VARARGS)
        MO__METHOD(translate,               METH_VARARGS)

        MO__METHOD(tesselate_triangles,     METH_VARARGS)
        MO__METHOD(tesselate_triangle,      METH_VARARGS)
        MO__METHOD(tesselate_lines,         METH_VARARGS)
        MO__METHOD(convert_to_lines,        METH_NOARGS)

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
    pgeom->geometry->addRef("py geometry create c");
    return pgeom;
}

bool isGeometry(void *vobj)
{
    return PyObject_TypeCheck((PyObject*)vobj, &Python34Geom_type);
}

bool expectGeometry(PyObject* obj)
{
    if (!obj)
    {
        PyErr_SetString(PyExc_TypeError, "expected Geometry, got NULL");
        return false;
    }
    if (!isGeometry(obj))
    {
        PyErr_Set(PyExc_TypeError, QString("expected Geometry, got %1")
                                    .arg(typeName(obj)));
        return false;
    }
    return true;
}


MO::GEOM::Geometry* getGeometry(PyObject* o)
{
    if (!expectGeometry(o))
        return nullptr;
    return reinterpret_cast<Python34Geom*>(o)->geometry;
}


} // namespace PYTHON34
} // namespace MO

#endif // MO_ENABLE_PYTHON34

