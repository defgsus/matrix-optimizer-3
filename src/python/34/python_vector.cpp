/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/2/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include "py_utils.h"

#include <functional>

#include <QString>

#include "python_vector.h"
#include "math/vector.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {

namespace {
extern "C" {

static const char* VectorDocString()
{
    static const char* str = "The geometric vector type";
    return str;
}




struct VectorStruct
{
    PyObject_HEAD
    double v[4];
    int len;

    static void dealloc(VectorStruct* self)
    {
        self->ob_base.ob_type->tp_free((PyObject*)self);
    }


};


bool get_vector(void* vargs_, int len, double v[])
{
    if (!vargs_)
    {
        PyErr_SetString(PyExc_TypeError, "NULL argument");
        return false;
    }
    PyObject* args_ = reinterpret_cast<PyObject*>(vargs_);
    if (isVector(args_))
    {
        auto vec = reinterpret_cast<VectorStruct*>(args_);
        if (vec->len != len)
        {
            PyErr_SetObject(PyExc_TypeError,
                            fromString(QString("vector has wrong size %1, expected %2")
                                       .arg(vec->len).arg(len)));
            return false;
        }
        for (int i=0; i<len; ++i)
            v[i] = vec->v[i];
        return true;
    }
    // scalar
    double tmp;
    if (toDouble(args_, &tmp))
    {
        for (int i=0; i<len; ++i)
            v[i] = tmp;
        return true;
    }
    PyErr_SetString(PyExc_TypeError, "expected Vec compatible");
    return false;
}

bool get_vector_var(void* vargs_, int *len, double vout[4])
{
    if (!vargs_)
    {
        PyErr_SetString(PyExc_TypeError, "NULL argument");
        return false;
    }
    PyObject* args_ = reinterpret_cast<PyObject*>(vargs_);
    if (toDouble(args_, &vout[0]))
    {
        *len = 1;
        return true;
    }
    if (isVector(args_))
    {
        auto vec = reinterpret_cast<VectorStruct*>(args_);
        *len = vec->len;
        for (int i=0; i<vec->len; ++i)
            vout[i] = vec->v[i];
        return true;
    }
    if (PySequence_Check(args_))
    {
        *len = PySequence_Size(args_);
        if (*len < 1 || *len > 4)
        {
            PyErr_Set(PyExc_TypeError, QString("number of arguments out of range 1<=%1<=4")
                                        .arg(*len));
            return false;
        }
        // read variaties like [1,2],3,[4]
        int i = 0, ilist = 0, iarg = 0;
        for (; iarg<*len; )
        {
            auto arg = PySequence_GetItem(args_, iarg);
//            MO_PRINT("ARG " << i << "/" << *len << " " << arg->ob_type->tp_name);
            double v;
            if (PyList_Check(arg))
            {
                //if (ilist == 0)
                //    *len = std::min(4, std::max(*len, i + (int)PyList_Size(arg)));
                if (ilist >= PyList_Size(arg))
                {
                    ilist = 0;
                    ++iarg;
                    continue;
                }
                arg = PyList_GetItem(arg, ilist++);
                --iarg;
            }
            if (i >= 4)
                break;
            ++iarg;
            if (!toDouble(arg, &v))
            {
                PyErr_Set(PyExc_TypeError, QString("argument %1 is not "
                                                   "convertible to float").arg(i));
                return false;
            }
            vout[i++] = v;
        }
        *len = i;
        return true;
    }
    PyErr_Set(PyExc_TypeError, QString("argument '%1' not convertible to sized vector")
              .arg(typeName(args_)));
    return false;
}




static int vec_init(VectorStruct* self, PyObject* args_, PyObject*)
{
    self->len = 2;
    self->v[0] = self->v[1] = self->v[2] = self->v[3] = 0.;
    int ret = 0;

    if (isVector(args_))
    {
        auto vec = reinterpret_cast<VectorStruct*>(args_);
        self->len = vec->len;
        for (int i=0; i<self->len; ++i)
            self->v[i] = vec->v[i];
        return ret;
    }
    PyErr_Clear();
    if (get_vector_var(args_, &self->len, self->v))
    {
        return ret;
    }

    return -1;
}

static PyObject* vec_newfunc(PyTypeObject* type, PyObject* , PyObject* )
{
    auto self = reinterpret_cast<VectorStruct*>(type->tp_alloc(type, 0));

    if (self != NULL)
    {
        self->v[0] = self->v[1] = self->v[2] = self->v[3] = 0.;
    }

    return reinterpret_cast<PyObject*>(self);
}



#define MO_PY_DEF_DOC(name__, str__) \
    static const char* name__ = str__;

    // ---------------- arithmetic --------------------


    static PyObject* vec_operator(VectorStruct* self, PyObject* other,
                                  std::function<double(double,double)> op)
    {
        double v[4], v2[] = { 0., 0., 0., 0. }; int len;
        if (!get_vector_var(other, &len, v))
            return NULL;
        // scalar
        if (len == 1)
            for (int i=0; i<self->len; ++i)
                v2[i] = op(self->v[i], v[0]);
        // vector x vector
        else
        {
            if (len != self->len)
            {
                PyErr_SetObject(PyExc_TypeError,
                    fromString(QString("operand has wrong size %1, expected %2")
                               .arg(len).arg(self->len)));
                return NULL;
            }

            for (int i=0; i<len; ++i)
                v2[i] = op(self->v[i], v[i]);
        }

        return (PyObject*)buildVector(v2, self->len);
    }

    static PyObject* vec_operator_inp(VectorStruct* self, PyObject* other,
                                       std::function<void(double&,double)> op)
    {
        double v[4]; int len;
        if (!get_vector_var(other, &len, v))
            return NULL;
        // scalar
        if (len == 1)
            for (int i=0; i<self->len; ++i)
                op(self->v[i], v[0]);
        // vector x vector
        else
        {
            if (len != self->len)
            {
                PyErr_SetObject(PyExc_TypeError,
                    fromString(QString("operand has wrong size %1, expected %2")
                               .arg(len).arg(self->len)));
                return NULL;
            }

            for (int i=0; i<len; ++i)
                op(self->v[i], v[i]);
        }
        Py_INCREF(self);
        return reinterpret_cast<PyObject*>(self);
    }


    static PyObject* vec_add(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y){ return x + y; });
    }

    static PyObject* vec_sub(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y){ return x - y; });
    }

    static PyObject* vec_mul(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y){ return x * y; });
    }

    static PyObject* vec_div(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y){ return x / y; });
    }

    static PyObject* vec_iadd(VectorStruct* self, PyObject* other)
    {
        return vec_operator_inp(self, other, [](double& x, double y){ x += y; });
    }

    static PyObject* vec_isub(VectorStruct* self, PyObject* other)
    {
        return vec_operator_inp(self, other, [](double& x, double y){ x -= y; });
    }

    static PyObject* vec_imul(VectorStruct* self, PyObject* other)
    {
        return vec_operator_inp(self, other, [](double& x, double y){ x *= y; });
    }

    static PyObject* vec_idiv(VectorStruct* self, PyObject* other)
    {
        return vec_operator_inp(self, other, [](double& x, double y){ x /= y; });
    }

    // ----------------- getter -----------------------

    MO_PY_DEF_DOC(vec_to_string_doc,
        "to_string() -> string\n"
        "Returns the string representation of the vector"
    )
    static PyObject* vec_to_string(VectorStruct* self, PyObject* )
    {
        if (self->len == 0)
            return fromString(QString("Vec(-)"));
        else if (self->len == 1)
            return fromString(QString("Vec(%1)")
                                .arg(self->v[0]));
        else if (self->len == 2)
            return fromString(QString("Vec(%1,%2)")
                                .arg(self->v[0])
                                .arg(self->v[1]));
        else if (self->len == 3)
            return fromString(QString("Vec(%1,%2,%3)")
                                .arg(self->v[0])
                                .arg(self->v[1])
                                .arg(self->v[2]));
        else
            return fromString(QString("Vec(%1,%2,%3,%4)")
                                .arg(self->v[0])
                                .arg(self->v[1])
                                .arg(self->v[2])
                                .arg(self->v[3]));
    }
    static PyObject* vec_repr(PyObject* self)
        { return vec_to_string(reinterpret_cast<VectorStruct*>(self), nullptr); }

static PyMemberDef Vector_members[] =
{
    { NULL, 0, 0, 0, NULL }
};

static PyMethodDef Vector_methods[] =
{
    { "to_string", (PyCFunction)vec_to_string,     METH_NOARGS, vec_to_string_doc },

    { NULL, NULL, 0, NULL }
};

static PyNumberMethods Vector_NumMethods = {
    (binaryfunc)    vec_add, /*nb_add*/
    (binaryfunc)    vec_sub, /*nb_subtract*/
    (binaryfunc)    vec_mul, /*nb_multiply*/
    NULL,                       /*nb_remainder*/
    NULL,                       /*nb_divmod*/
    NULL,                       /*nb_power*/
    NULL,//(unaryfunc)     Vector_neg, /*nb_negative*/
    NULL,//(unaryfunc)     Vector_copy,/*tp_positive*/
    (unaryfunc)     NULL,       /*tp_absolute*/
    (inquiry)   NULL,           /*tp_bool*/
    (unaryfunc) NULL,           /*nb_invert*/
    NULL,                       /*nb_lshift*/
    (binaryfunc)NULL,           /*nb_rshift*/
    NULL,                       /*nb_and*/
    NULL,                       /*nb_xor*/
    NULL,                       /*nb_or*/
    NULL,                       /*nb_int*/
    NULL,                       /*nb_reserved*/
    NULL,                       /*nb_float*/
    (binaryfunc)vec_iadd,                /* nb_inplace_add */
    (binaryfunc)vec_isub,                /* nb_inplace_subtract */
    (binaryfunc)vec_imul,                /* nb_inplace_multiply */
    NULL,                       /* nb_inplace_remainder */
    NULL,                       /* nb_inplace_power */
    NULL,                       /* nb_inplace_lshift */
    NULL,                       /* nb_inplace_rshift */
    NULL,                       /* nb_inplace_and */
    NULL,                       /* nb_inplace_xor */
    NULL,                       /* nb_inplace_or */
    NULL,                       /* nb_floor_divide */
    (binaryfunc)vec_div,                 /* nb_true_divide */
    NULL,                       /* nb_inplace_floor_divide */
    (binaryfunc)vec_idiv,                /* nb_inplace_true_divide */
    NULL                        /* nb_index */
};

static PyTypeObject* Vector_Type()
{
    static PyTypeObject type =
    {
        PyVarObject_HEAD_INIT(NULL, 0)
        "matrixoptimizer.Vec",  /*tp_name*/
        sizeof(VectorStruct),  /*tp_basicsize*/
        0,                         /*tp_itemsize*/
        (destructor)VectorStruct::dealloc,/*tp_dealloc*/
        0,                         /*tp_print*/
        0,                         /*tp_getattr*/
        0,                         /*tp_setattr*/
        0,                         /*tp_reserved*/
        0,//vec_repr,     /*tp_repr*/
        &Vector_NumMethods,         /*tp_as_number*/
        0,                         /*tp_as_sequence*/
        0,                         /*tp_as_mapping*/
        0,                         /*tp_hash */
        0,                         /*tp_call*/
        vec_repr,     /*tp_str*/
        PyObject_GenericGetAttr,   /*tp_getattro*/
        PyObject_GenericSetAttr,   /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
        VectorDocString(),   /* tp_doc */
        0,		               /* tp_traverse */
        0,		               /* tp_clear */
        0,		               /* tp_richcompare */
        0,		               /* tp_weaklistoffset */
        0,//VectorFuncs::get_iter,	/* tp_iter */
        0,//VectorFuncs::next_iter, /* tp_iternext */
        Vector_methods,      /* tp_methods */
        Vector_members,      /* tp_members */
        0,                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        (initproc)vec_init,      /* tp_init */
        0,                         /* tp_alloc */
        (newfunc)vec_newfunc,       /* tp_new */
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
    return &type;
}



} // extern "C"
} // namespace






bool isVector(void *obj)
{ return PyObject_TypeCheck(reinterpret_cast<PyObject*>(obj), Vector_Type()); }

void initVector(void* mod)
{
    PyObject* module = reinterpret_cast<PyObject*>(mod);
    initObjectType(module, Vector_Type(), "Vec");
}


template <class VEC, int len>
void* tmpl_buildVector(const VEC& v)
{
    auto pobj = PyObject_New(VectorStruct, Vector_Type());
    pobj->len = len;
    for (int i=0; i<len; ++i)
        pobj->v[i] = v[i];
    return pobj;
}

template <class VEC, int len>
VEC tmpl_getVector(void* vobj)
{
    VEC v;
    if (!PyObject_TypeCheck(reinterpret_cast<PyTypeObject*>(vobj), Vector_Type()))
        return v;
    auto pvec = reinterpret_cast<VectorStruct*>(vobj);
    int size = std::min(len, pvec->len);
    for (int i=0; i<size; ++i)
        v[i] = pvec->v[i];
    return v;
}

void* buildVector(const Vec2& v) { return tmpl_buildVector<Vec2,2>(v); }
void* buildVector(const Vec3& v) { return tmpl_buildVector<Vec3,3>(v); }
void* buildVector(const Vec4& v) { return tmpl_buildVector<Vec4,4>(v); }
void* buildVector(const DVec2& v) { return tmpl_buildVector<DVec2,2>(v); }
void* buildVector(const DVec3& v) { return tmpl_buildVector<DVec3,3>(v); }
void* buildVector(const DVec4& v) { return tmpl_buildVector<DVec4,4>(v); }
void* buildVector(const double v[], int len)
{
    if (len == 0) return tmpl_buildVector<const double*, 0>(v);
    if (len == 1) return tmpl_buildVector<const double*, 1>(v);
    if (len == 2) return tmpl_buildVector<const double*, 2>(v);
    else if (len == 3) return tmpl_buildVector<const double*, 3>(v);
    else if (len == 4) return tmpl_buildVector<const double*, 4>(v);
    else return NULL;
}

DVec2 py_getVector2(void* pyObject) { return tmpl_getVector<DVec2, 2>(pyObject); }
DVec3 py_getVector3(void* pyObject) { return tmpl_getVector<DVec3, 3>(pyObject); }
DVec4 py_getVector4(void* pyObject) { return tmpl_getVector<DVec4, 4>(pyObject); }


} // namespace PYTHON34
} // namespace MO


#endif
