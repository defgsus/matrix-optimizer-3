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
#include "math/functions.h"
#include "math/arithmeticarray.h"
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


#define MO__VEC_STR "Vec"

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

static VectorStruct* new_vec();
static VectorStruct* copy_vec(VectorStruct*);

} // extern "C"
} // namespace


bool get_vector(void* vargs_, int len, double v[])
{
    if (!vargs_)
    {
        PyErr_SetString(PyExc_TypeError, "NULL argument");
        return false;
    }
    // get single argument from tuple
    auto args_ = removeArgumentTuple( reinterpret_cast<PyObject*>(vargs_) );
    if (PySequence_Check(args_))
    {
        if (PySequence_Size(args_) != len)
        {
            PyErr_Set(PyExc_TypeError, QString("sequence argument has wrong size %1, "
                                               "expected %2")
                                                .arg(PySequence_Size(args_))
                                                .arg(len));
            return false;
        }
        for (int i=0; i<len; ++i)
        {
            double val;
            if (!toDouble(PySequence_GetItem(args_, i), &val))
                return false;
            v[i] = val;
        }
        return true;
    }
    // check vector
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
    // check scalar
    double tmp;
    if (toDouble(args_, &tmp))
    {
        for (int i=0; i<len; ++i)
            v[i] = tmp;
        return true;
    }
    PyErr_Set(PyExc_TypeError, QString("%1 not convertible to " MO__VEC_STR)
                                .arg(typeName(args_)));
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
            PyErr_Set(PyExc_TypeError,
                      QString("number of arguments out of range 1<=%1<=4")
                      .arg(*len));
            return false;
        }
        // process all variaties like [1,2],3,[4]
        int i = 0, ilist = 0, iarg = 0;
        for (; iarg<*len; )
        {
            auto arg = PySequence_GetItem(args_, iarg);
//            MO_PRINT("ARG " << i << "/" << *len << " " << arg->ob_type->tp_name);
            double v;
            if (PyList_Check(arg))
            {
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
                PyErr_Set(PyExc_TypeError, QString("argument %1 (%2) is not "
                                                   "convertible to float")
                                                    .arg(i).arg(typeName(arg)));
                return false;
            }
            vout[i++] = v;
        }
        *len = i;
        return true;
    }
    PyErr_Set(PyExc_TypeError, QString("argument '%1' not convertible to vector "
                                       "of defined size")
              .arg(typeName(args_)));
    return false;
}






namespace {
extern "C" {


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


    // -------------- various helper -------------------


    // executes foo on self and returns self
    PyObject* inplace_func(VectorStruct* self, std::function<void(VectorStruct*)> foo)
    {
        foo(self);
        Py_INCREF(self);
        return reinterpret_cast<PyObject*>(self);
    }

    // executes foo on copy of self and returns copy
    // foo is like inplace_func() above
    PyObject* copy_of_inplace(VectorStruct* self, PyObject*(*foo)(VectorStruct*, PyObject*))
    {
        auto copy = copy_vec(self);
        foo(copy, NULL);
        Py_DECREF(copy);
        return reinterpret_cast<PyObject*>(copy);
    }

    // reduces two vectors to one double
    // other is expected to have same size
    PyObject* vec_reduce_func2(VectorStruct* self, PyObject* other,
                               std::function<double(int len, double[], double[])> foo)
    {
        double v2[4];
        if (!get_vector(other, self->len, v2))
            return NULL;
        double ret = foo(self->len, self->v, v2);
        return fromDouble(ret);
    }


    static PyObject* vec_operator(
            VectorStruct* self, PyObject* other,
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

    static PyObject* vec_operator_ternary(
            VectorStruct* self, PyObject* other1, PyObject* other2,
            std::function<double(double,double,double)> op)
    {
        double vres[] = { 0., 0., 0., 0. }, v1[4], v2[4];
        int len1, len2;
        if (!get_vector_var(other1, &len1, v1))
            return NULL;
        if (!get_vector_var(other2, &len2, v2))
            return NULL;
        // scalar
        if (len1 == 1)
        {
            len1 = self->len;
            for (int i=1; i<self->len; ++i)
                v1[i] = v1[0];
        }
        if (len2 == 1)
        {
            len2 = self->len;
            for (int i=1; i<self->len; ++i)
                v2[i] = v2[0];
        }
        // vector x vector x vector
        if (len1 != self->len || len2 != self->len)
        {
            PyErr_SetObject(PyExc_TypeError,
                fromString(QString("operands have wrong size %1 and %2, expected %3")
                           .arg(len1).arg(len2).arg(self->len)));
            return NULL;
        }

        for (int i=0; i<self->len; ++i)
            vres[i] = op(self->v[i], v1[i], v2[i]);

        return (PyObject*)buildVector(vres, self->len);
    }


    static PyObject* vec_operator_inp(
            VectorStruct* self, PyObject* other,
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


#define MO_PY_DEF_DOC(name__, str__) \
    static const char* name__ = str__;

    // ---------------- arithmetic --------------------

    // classic operator

    static PyObject* vec_add(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y)
            { return x + y; });
    }

    static PyObject* vec_sub(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y)
            { return x - y; });
    }

    static PyObject* vec_mul(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y)
            { return x * y; });
    }

    static PyObject* vec_div(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y)
            { return x / y; });
    }

    // inplace operators

    static PyObject* vec_iadd(VectorStruct* self, PyObject* other)
    {
        return vec_operator_inp(self, other, [](double& x, double y)
            { x += y; });
    }

    static PyObject* vec_isub(VectorStruct* self, PyObject* other)
    {
        return vec_operator_inp(self, other, [](double& x, double y)
            { x -= y; });
    }

    static PyObject* vec_imul(VectorStruct* self, PyObject* other)
    {
        return vec_operator_inp(self, other, [](double& x, double y)
            { x *= y; });
    }

    static PyObject* vec_idiv(VectorStruct* self, PyObject* other)
    {
        return vec_operator_inp(self, other, [](double& x, double y)
            { x /= y; });
    }

    static PyObject* vec_imod(VectorStruct* self, PyObject* other)
    {
        return vec_operator_inp(self, other, [](double& x, double y)
            { x = MATH::modulo(x, y); });
    }

    // ----- other number methods -----

    static PyObject* vec_abs_unary(VectorStruct* self)
    {
        auto res = copy_vec(self);
        for (int i=0; i<res->len; ++i)
            res->v[i] = std::abs(res->v[i]);
        return reinterpret_cast<PyObject*>(res);
    }

    static PyObject* vec_neg_unary(VectorStruct* self)
    {
        auto res = copy_vec(self);
        for (int i=0; i<res->len; ++i)
            res->v[i] = -res->v[i];
        return reinterpret_cast<PyObject*>(res);
    }

    static PyObject* vec_pow(VectorStruct* self, PyObject* other);
    static PyObject* vec_pow_ternary(VectorStruct* self, PyObject* other, PyObject* z)
    {
        if (!z || z == Py_None)
            return vec_pow(self, other);

        return vec_operator_ternary(self, other, z,
            [](double x, double y, double z) -> double
            { return y > 0. && z > 0.
                    ? MATH::modulo(std::pow(x, y), z) : std::pow(x, y); });
    }


    // ---- operator-like functions, e.g. v1.pow(v2) ----

    MO_PY_DEF_DOC(vec_pow_doc,
        "pow(vec) -> " MO__VEC_STR "\n"
        "Calculates self to the power of the argument"
    )
    static PyObject* vec_pow(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y) -> double
            { return std::pow(x, y); });
    }

    MO_PY_DEF_DOC(vec_mod_doc,
        "mod(vec) -> " MO__VEC_STR "\n"
        "Calculates the modulo of self and vector argument"
    )
    static PyObject* vec_mod(VectorStruct* self, PyObject* other)
    {
        return vec_operator(self, other, [](double x, double y) -> double
            { return MATH::modulo(x, y); });
    }

    MO_PY_DEF_DOC(vec_normalize_doc,
        "normalize() -> " MO__VEC_STR "\n"
        "Normalizes the vector so it's length will be one.\n"
        "Ignored when all components are zero.\n"
        "Returns self"
    )
    static PyObject* vec_normalize(VectorStruct* self, PyObject* )
    {
        return inplace_func(self, [](VectorStruct* self)
        {
            double mag = 0;
            for (int i=0; i<self->len; ++i)
                mag += self->v[i] * self->v[i];
            if (mag > 0.)
            {
                mag = std::sqrt(mag);
                for (int i=0; i<self->len; ++i)
                    self->v[i] /= mag;
            }
        });
    }

    MO_PY_DEF_DOC(vec_normalized_doc,
        "normalized() -> " MO__VEC_STR "\n"
        "Returns a normalized vector who's length will be one.\n"
        "Returns a copy when all components are zero."
    )
    static PyObject* vec_normalized(VectorStruct* self, PyObject* )
    {
        return copy_of_inplace(self, vec_normalize);
    }

    MO_PY_DEF_DOC(vec_zero_doc,
        "zero() -> Vec\n"
        "Fills all components with zero.\n"
        "Note: same as vec = 0\n"
        "Returns self"
    )
    static PyObject* vec_zero(VectorStruct* self, PyObject* )
    {
        return inplace_func(self, [](VectorStruct* self)
        {
            for (int i=0; i<self->len; ++i)
                self->v[i] = 0.;
        });
    }


    MO_PY_DEF_DOC(vec_clamp_doc,
        "clamp(f, f) -> " MO__VEC_STR "\n"
        "Clamps all components to the range [f, f]"
    )
    static PyObject* vec_clamp(VectorStruct* self, PyObject* other)
    {
        double mi, ma;
        if (!PyArg_ParseTuple(other, "dd", &mi, &ma))
            return NULL;
        for (int i = 0; i<self->len; ++i)
            self->v[i] = std::max(mi,std::min(ma, self->v[i]));
        Py_INCREF(self);
        return reinterpret_cast<PyObject*>(self);
    }


    // ----------------- getter -----------------------

    MO_PY_DEF_DOC(vec_to_string_doc,
        "to_string() -> string\n"
        "Returns the string representation of the vector"
    )
    static PyObject* vec_to_string(VectorStruct* self, PyObject* )
    {
        if (self->len == 0)
            return fromString(QString(MO__VEC_STR "(-)"));
        else if (self->len == 1)
            return fromString(QString(MO__VEC_STR "(%1)")
                                .arg(self->v[0]));
        else if (self->len == 2)
            return fromString(QString(MO__VEC_STR "(%1,%2)")
                                .arg(self->v[0])
                                .arg(self->v[1]));
        else if (self->len == 3)
            return fromString(QString(MO__VEC_STR "(%1,%2,%3)")
                                .arg(self->v[0])
                                .arg(self->v[1])
                                .arg(self->v[2]));
        else
            return fromString(QString(MO__VEC_STR "(%1,%2,%3,%4)")
                                .arg(self->v[0])
                                .arg(self->v[1])
                                .arg(self->v[2])
                                .arg(self->v[3]));
    }
    static PyObject* vec_repr(PyObject* self)
        { return vec_to_string(reinterpret_cast<VectorStruct*>(self), nullptr); }


    MO_PY_DEF_DOC(vec_to_tuple_doc,
        "to_tuple() -> tuple\n"
        "Returns the vector components in a tuple"
    )
    static PyObject* vec_to_tuple(VectorStruct* self, PyObject* )
    {
        if (self->len < 1)
        {
            PyErr_SetString(PyExc_TypeError, "zero length vector can't be converted "
                                             "to tuple");
            return NULL;
        }
        auto tuple = PyTuple_New(self->len);
        if (!tuple)
        {
            PyErr_SetString(PyExc_MemoryError, "could not create tuple");
            return NULL;
        }
        for (int i=0; i<self->len; ++i)
        {
            auto obj = fromDouble(self->v[i]);
            if (!obj)
            {
                Py_DECREF(tuple);
                return NULL;
            }
            PyTuple_SET_ITEM(tuple, i, obj);
        }
        return tuple;
    }

    MO_PY_DEF_DOC(vec_to_list_doc,
        "to_list() -> list\n"
        "Returns the vector components in a list"
    )
    static PyObject* vec_to_list(VectorStruct* self, PyObject* )
    {
        auto list = PyList_New(self->len);
        if (!list)
        {
            PyErr_SetString(PyExc_MemoryError, "could not create list");
            return NULL;
        }
        for (int i=0; i<self->len; ++i)
        {
            auto obj = fromDouble(self->v[i]);
            if (!obj)
            {
                Py_DECREF(list);
                return NULL;
            }
            PyList_SET_ITEM(list, i, obj);
        }
        return list;
    }


    MO_PY_DEF_DOC(vec_len_doc,
        "len() -> int\n"
        "Returns the size / number of dimensions.\n"
    )
    static PyObject* vec_len(VectorStruct* self, PyObject* )
    {
        return Py_BuildValue("l", self->len);
    }

    MO_PY_DEF_DOC(vec_length_doc,
        "length() -> f\n"
        "Returns the length of the vector.\n"
    )
    static PyObject* vec_length(VectorStruct* self, PyObject* )
    {
        double mag = 0;
        for (int i=0; i<self->len; ++i)
            mag += self->v[i] * self->v[i];
        return fromDouble(mag > 0. ? std::sqrt(mag) : mag);
    }

    MO_PY_DEF_DOC(vec_length_squared_doc,
        "length_squared() -> f\n"
        "Returns the squared length of the vector.\n"
    )
    static PyObject* vec_length_squared(VectorStruct* self, PyObject* )
    {
        double mag = 0;
        for (int i=0; i<self->len; ++i)
            mag += self->v[i] * self->v[i];
        return fromDouble(mag);
    }


    MO_PY_DEF_DOC(vec_dot_doc,
        "dot(vec) -> f\n"
        "Calculates the dot product of self and the vector argument.\n"
    )
    static PyObject* vec_dot(VectorStruct* self, PyObject* other)
    {
        return vec_reduce_func2(self, other, [](int len, double v1[], double v2[]) -> double
        {
            double d = 0.;
            for (int i=0; i<len; ++i)
                d += v1[i] * v2[i];
            return d;
        });
    }

    MO_PY_DEF_DOC(vec_distance_doc,
        "distance(vec) -> f\n"
        "Calculates the distance between self and the vector argument.\n"
    )
    static PyObject* vec_distance(VectorStruct* self, PyObject* other)
    {
        return vec_reduce_func2(self, other, [](int len, double v1[], double v2[]) -> double
        {
            double d = 0.;
            for (int i=0; i<len; ++i)
                d += std::pow(v2[i] - v1[i], 2.);
            return d > 0. ? std::sqrt(d) : d;
        });
    }

    MO_PY_DEF_DOC(vec_distance_squared_doc,
        "distance_squared(vec) -> f\n"
        "Calculates the squared distance between self and the vector argument.\n"
    )
    static PyObject* vec_distance_squared(VectorStruct* self, PyObject* other)
    {
        return vec_reduce_func2(self, other, [](int len, double v1[], double v2[]) -> double
        {
            double d = 0.;
            for (int i=0; i<len; ++i)
                d += std::pow(v2[i] - v1[i], 2.);
            return d;
        });
    }


    // gets single component by index
    static PyObject* vec_getter(VectorStruct* self, void* ptr)
    {
        int idx = int64_t(ptr);
        if (idx < 0 || idx >= self->len)
        {
            PyErr_Set(PyExc_IndexError, QString("index in vector out of range %1/%2")
                      .arg(idx).arg(self->len));
            return NULL;
        }
        return PyFloat_FromDouble(self->v[idx]);
    }


    // gets multiple components by bit encoding
    // e.g. 0x1 = x, 0x123 = zyx, 0x1432 = yzwx
    static PyObject* vec_swizzle_getter(VectorStruct* self, void* ptr)
    {
        auto ret = new_vec();
        ret->len = 0;
        while (ret->len < 4)
        {
            int idx = (int64_t(ptr) >> (ret->len*4)) & 15;
            if (idx < 1 || idx > 4)
                break;
            ret->v[ret->len++] = self->v[idx-1];
        }
        return reinterpret_cast<PyObject*>(ret);
    }

    static int vec_swizzle_setter(VectorStruct* self, PyObject* other, void* ptr)
    {
        // determine swizzle size
        int slen=0;
        for (; slen<4; ++slen)
        {
            int idx = (int64_t(ptr) >> (slen*4)) & 15;
            if (idx < 1 || idx > 4)
                break;
        }
        if (slen > self->len)
        {
            PyErr_SetString(PyExc_TypeError, "sizzle exceeds vector size");
            return -1;
        }
        // get argument
        double v[4]; int olen;
        if (!get_vector_var(other, &olen, v))
            return -1;
        // scalar
        if (olen == 1)
            for (int i=1; i<slen; ++i)
                v[i] = v[0];
        // vector
        else if (olen != slen)
        {
            PyErr_Set(PyExc_TypeError,
                      QString("operand has wrong size %1, expected %2")
                      .arg(olen).arg(slen));
            return -1;
        }
        // apply
        for (int i=0; i<4; ++i)
        {
            int idx = (int64_t(ptr) >> (i*4)) & 15;
            if (idx < 1 || idx > 4)
                break;
            self->v[idx-1] = v[i];
        }
        return 0;
    }


    // --- sequence funcs ---
    static Py_ssize_t vec_sq_length(VectorStruct* self)
    {
        return self->len;
    }

    static PyObject* vec_sq_item(VectorStruct* self, Py_ssize_t idx)
    {
        return vec_getter(self, (void*)idx);
    }

    static int vec_sq_ass_item(VectorStruct* self, Py_ssize_t idx, PyObject* arg)
    {
        if (idx >= self->len)
        {
            PyErr_Set(PyExc_IndexError, QString("index in vector out of range %1/%2")
                      .arg(idx).arg(self->len));
            return -1;
        }
        return vec_swizzle_setter(self, arg, (void*)(idx+1));
    }



static PyMemberDef Vector_members[] =
{
    { NULL, 0, 0, 0, NULL }
};

#define MO__METHOD(name__, args__) \
    { #name__, (PyCFunction)vec_##name__, args__, vec_##name__##_doc },

static PyMethodDef Vector_methods[] =
{
    MO__METHOD(to_string,           METH_NOARGS)
    MO__METHOD(to_tuple,            METH_NOARGS)
    MO__METHOD(to_list,             METH_NOARGS)

    MO__METHOD(len,                 METH_NOARGS)
    MO__METHOD(length,              METH_NOARGS)
    MO__METHOD(length_squared,      METH_NOARGS)
    MO__METHOD(zero,                METH_NOARGS)
    MO__METHOD(clamp,               METH_VARARGS)

    MO__METHOD(normalize,           METH_NOARGS)
    MO__METHOD(normalized,          METH_NOARGS)

    MO__METHOD(pow,                 METH_VARARGS)
    MO__METHOD(mod,                 METH_VARARGS)

    MO__METHOD(dot,                 METH_VARARGS)
    MO__METHOD(distance,            METH_VARARGS)
    MO__METHOD(distance_squared,    METH_VARARGS)

    { NULL, NULL, 0, NULL }
};
#undef MO__METHOD

static PyNumberMethods Vector_NumMethods = {
    (binaryfunc)    vec_add, /*nb_add*/
    (binaryfunc)    vec_sub, /*nb_subtract*/
    (binaryfunc)    vec_mul, /*nb_multiply*/
    (binaryfunc)    vec_mod, /*nb_remainder*/
    NULL,                       /*nb_divmod*/
    (ternaryfunc)   vec_pow_ternary, /*nb_power*/
    (unaryfunc)     vec_neg_unary, /*nb_negative*/
    (unaryfunc)     copy_vec,/*tp_positive*/
    (unaryfunc)     vec_abs_unary,       /*tp_absolute*/
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
    (binaryfunc)vec_imod,       /* nb_inplace_remainder */
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

PySequenceMethods Vector_SeqMethods = {
    (lenfunc)vec_sq_length, /* lenfunc sq_length */
    NULL, /* binaryfunc sq_concat */
    NULL, /* ssizeargfunc sq_repeat */
    (ssizeargfunc)vec_sq_item, /* ssizeargfunc sq_item */
    NULL, /* void *was_sq_slice */
    (ssizeobjargproc)vec_sq_ass_item, /* ssizeobjargproc sq_ass_item */
    NULL, /* void *was_sq_ass_slice */
    NULL, /* objobjproc sq_contains */
    NULL, /* binaryfunc sq_inplace_concat */
    NULL  /* ssizeargfunc sq_inplace_repeat */
};


static PyGetSetDef Vector_getseters[] = {
    { (char*)"x"   , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1 },
    { (char*)"r"   , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1 },
    { (char*)"xx"  , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x11 },
    { (char*)"rr"  , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x11 },
    { (char*)"xxx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x111 },
    { (char*)"rrr" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x111 },
    { (char*)"xxxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1111 },
    { (char*)"rrrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1111 },
    { (char*)"xxxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2111 },
    { (char*)"rrrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2111 },
    { (char*)"xxxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3111 },
    { (char*)"rrrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3111 },
    { (char*)"xxxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4111 },
    { (char*)"rrra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4111 },
    { (char*)"xxy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x211 },
    { (char*)"rrg" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x211 },
    { (char*)"xxyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1211 },
    { (char*)"rrgr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1211 },
    { (char*)"xxyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2211 },
    { (char*)"rrgg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2211 },
    { (char*)"xxyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3211 },
    { (char*)"rrgb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3211 },
    { (char*)"xxyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4211 },
    { (char*)"rrga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4211 },
    { (char*)"xxz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x311 },
    { (char*)"rrb" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x311 },
    { (char*)"xxzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1311 },
    { (char*)"rrbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1311 },
    { (char*)"xxzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2311 },
    { (char*)"rrbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2311 },
    { (char*)"xxzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3311 },
    { (char*)"rrbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3311 },
    { (char*)"xxzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4311 },
    { (char*)"rrba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4311 },
    { (char*)"xxw" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x411 },
    { (char*)"rra" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x411 },
    { (char*)"xxwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1411 },
    { (char*)"rrar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1411 },
    { (char*)"xxwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2411 },
    { (char*)"rrag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2411 },
    { (char*)"xxwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3411 },
    { (char*)"rrab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3411 },
    { (char*)"xxww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4411 },
    { (char*)"rraa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4411 },
    { (char*)"xy"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x21 },
    { (char*)"rg"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x21 },
    { (char*)"xyx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x121 },
    { (char*)"rgr" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x121 },
    { (char*)"xyxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1121 },
    { (char*)"rgrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1121 },
    { (char*)"xyxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2121 },
    { (char*)"rgrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2121 },
    { (char*)"xyxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3121 },
    { (char*)"rgrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3121 },
    { (char*)"xyxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4121 },
    { (char*)"rgra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4121 },
    { (char*)"xyy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x221 },
    { (char*)"rgg" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x221 },
    { (char*)"xyyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1221 },
    { (char*)"rggr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1221 },
    { (char*)"xyyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2221 },
    { (char*)"rggg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2221 },
    { (char*)"xyyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3221 },
    { (char*)"rggb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3221 },
    { (char*)"xyyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4221 },
    { (char*)"rgga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4221 },
    { (char*)"xyz" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x321 },
    { (char*)"rgb" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x321 },
    { (char*)"xyzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1321 },
    { (char*)"rgbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1321 },
    { (char*)"xyzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2321 },
    { (char*)"rgbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2321 },
    { (char*)"xyzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3321 },
    { (char*)"rgbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3321 },
    { (char*)"xyzw", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4321 },
    { (char*)"rgba", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4321 },
    { (char*)"xyw" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x421 },
    { (char*)"rga" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x421 },
    { (char*)"xywx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1421 },
    { (char*)"rgar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1421 },
    { (char*)"xywy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2421 },
    { (char*)"rgag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2421 },
    { (char*)"xywz", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3421 },
    { (char*)"rgab", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3421 },
    { (char*)"xyww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4421 },
    { (char*)"rgaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4421 },
    { (char*)"xz"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x31 },
    { (char*)"rb"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x31 },
    { (char*)"xzx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x131 },
    { (char*)"rbr" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x131 },
    { (char*)"xzxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1131 },
    { (char*)"rbrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1131 },
    { (char*)"xzxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2131 },
    { (char*)"rbrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2131 },
    { (char*)"xzxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3131 },
    { (char*)"rbrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3131 },
    { (char*)"xzxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4131 },
    { (char*)"rbra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4131 },
    { (char*)"xzy" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x231 },
    { (char*)"rbg" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x231 },
    { (char*)"xzyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1231 },
    { (char*)"rbgr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1231 },
    { (char*)"xzyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2231 },
    { (char*)"rbgg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2231 },
    { (char*)"xzyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3231 },
    { (char*)"rbgb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3231 },
    { (char*)"xzyw", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4231 },
    { (char*)"rbga", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4231 },
    { (char*)"xzz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x331 },
    { (char*)"rbb" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x331 },
    { (char*)"xzzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1331 },
    { (char*)"rbbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1331 },
    { (char*)"xzzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2331 },
    { (char*)"rbbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2331 },
    { (char*)"xzzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3331 },
    { (char*)"rbbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3331 },
    { (char*)"xzzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4331 },
    { (char*)"rbba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4331 },
    { (char*)"xzw" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x431 },
    { (char*)"rba" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x431 },
    { (char*)"xzwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1431 },
    { (char*)"rbar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1431 },
    { (char*)"xzwy", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2431 },
    { (char*)"rbag", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2431 },
    { (char*)"xzwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3431 },
    { (char*)"rbab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3431 },
    { (char*)"xzww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4431 },
    { (char*)"rbaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4431 },
    { (char*)"xw"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x41 },
    { (char*)"ra"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x41 },
    { (char*)"xwx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x141 },
    { (char*)"rar" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x141 },
    { (char*)"xwxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1141 },
    { (char*)"rarr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1141 },
    { (char*)"xwxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2141 },
    { (char*)"rarg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2141 },
    { (char*)"xwxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3141 },
    { (char*)"rarb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3141 },
    { (char*)"xwxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4141 },
    { (char*)"rara", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4141 },
    { (char*)"xwy" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x241 },
    { (char*)"rag" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x241 },
    { (char*)"xwyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1241 },
    { (char*)"ragr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1241 },
    { (char*)"xwyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2241 },
    { (char*)"ragg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2241 },
    { (char*)"xwyz", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3241 },
    { (char*)"ragb", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3241 },
    { (char*)"xwyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4241 },
    { (char*)"raga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4241 },
    { (char*)"xwz" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x341 },
    { (char*)"rab" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x341 },
    { (char*)"xwzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1341 },
    { (char*)"rabr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1341 },
    { (char*)"xwzy", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2341 },
    { (char*)"rabg", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2341 },
    { (char*)"xwzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3341 },
    { (char*)"rabb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3341 },
    { (char*)"xwzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4341 },
    { (char*)"raba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4341 },
    { (char*)"xww" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x441 },
    { (char*)"raa" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x441 },
    { (char*)"xwwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1441 },
    { (char*)"raar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1441 },
    { (char*)"xwwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2441 },
    { (char*)"raag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2441 },
    { (char*)"xwwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3441 },
    { (char*)"raab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3441 },
    { (char*)"xwww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4441 },
    { (char*)"raaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4441 },
    { (char*)"y"   , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2 },
    { (char*)"g"   , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2 },
    { (char*)"yx"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x12 },
    { (char*)"gr"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x12 },
    { (char*)"yxx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x112 },
    { (char*)"grr" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x112 },
    { (char*)"yxxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1112 },
    { (char*)"grrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1112 },
    { (char*)"yxxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2112 },
    { (char*)"grrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2112 },
    { (char*)"yxxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3112 },
    { (char*)"grrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3112 },
    { (char*)"yxxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4112 },
    { (char*)"grra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4112 },
    { (char*)"yxy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x212 },
    { (char*)"grg" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x212 },
    { (char*)"yxyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1212 },
    { (char*)"grgr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1212 },
    { (char*)"yxyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2212 },
    { (char*)"grgg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2212 },
    { (char*)"yxyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3212 },
    { (char*)"grgb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3212 },
    { (char*)"yxyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4212 },
    { (char*)"grga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4212 },
    { (char*)"yxz" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x312 },
    { (char*)"grb" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x312 },
    { (char*)"yxzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1312 },
    { (char*)"grbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1312 },
    { (char*)"yxzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2312 },
    { (char*)"grbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2312 },
    { (char*)"yxzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3312 },
    { (char*)"grbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3312 },
    { (char*)"yxzw", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4312 },
    { (char*)"grba", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4312 },
    { (char*)"yxw" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x412 },
    { (char*)"gra" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x412 },
    { (char*)"yxwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1412 },
    { (char*)"grar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1412 },
    { (char*)"yxwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2412 },
    { (char*)"grag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2412 },
    { (char*)"yxwz", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3412 },
    { (char*)"grab", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3412 },
    { (char*)"yxww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4412 },
    { (char*)"graa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4412 },
    { (char*)"yy"  , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x22 },
    { (char*)"gg"  , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x22 },
    { (char*)"yyx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x122 },
    { (char*)"ggr" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x122 },
    { (char*)"yyxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1122 },
    { (char*)"ggrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1122 },
    { (char*)"yyxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2122 },
    { (char*)"ggrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2122 },
    { (char*)"yyxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3122 },
    { (char*)"ggrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3122 },
    { (char*)"yyxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4122 },
    { (char*)"ggra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4122 },
    { (char*)"yyy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x222 },
    { (char*)"ggg" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x222 },
    { (char*)"yyyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1222 },
    { (char*)"gggr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1222 },
    { (char*)"yyyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2222 },
    { (char*)"gggg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2222 },
    { (char*)"yyyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3222 },
    { (char*)"gggb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3222 },
    { (char*)"yyyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4222 },
    { (char*)"ggga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4222 },
    { (char*)"yyz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x322 },
    { (char*)"ggb" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x322 },
    { (char*)"yyzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1322 },
    { (char*)"ggbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1322 },
    { (char*)"yyzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2322 },
    { (char*)"ggbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2322 },
    { (char*)"yyzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3322 },
    { (char*)"ggbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3322 },
    { (char*)"yyzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4322 },
    { (char*)"ggba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4322 },
    { (char*)"yyw" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x422 },
    { (char*)"gga" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x422 },
    { (char*)"yywx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1422 },
    { (char*)"ggar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1422 },
    { (char*)"yywy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2422 },
    { (char*)"ggag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2422 },
    { (char*)"yywz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3422 },
    { (char*)"ggab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3422 },
    { (char*)"yyww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4422 },
    { (char*)"ggaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4422 },
    { (char*)"yz"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x32 },
    { (char*)"gb"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x32 },
    { (char*)"yzx" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x132 },
    { (char*)"gbr" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x132 },
    { (char*)"yzxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1132 },
    { (char*)"gbrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1132 },
    { (char*)"yzxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2132 },
    { (char*)"gbrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2132 },
    { (char*)"yzxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3132 },
    { (char*)"gbrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3132 },
    { (char*)"yzxw", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4132 },
    { (char*)"gbra", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4132 },
    { (char*)"yzy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x232 },
    { (char*)"gbg" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x232 },
    { (char*)"yzyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1232 },
    { (char*)"gbgr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1232 },
    { (char*)"yzyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2232 },
    { (char*)"gbgg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2232 },
    { (char*)"yzyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3232 },
    { (char*)"gbgb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3232 },
    { (char*)"yzyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4232 },
    { (char*)"gbga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4232 },
    { (char*)"yzz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x332 },
    { (char*)"gbb" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x332 },
    { (char*)"yzzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1332 },
    { (char*)"gbbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1332 },
    { (char*)"yzzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2332 },
    { (char*)"gbbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2332 },
    { (char*)"yzzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3332 },
    { (char*)"gbbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3332 },
    { (char*)"yzzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4332 },
    { (char*)"gbba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4332 },
    { (char*)"yzw" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x432 },
    { (char*)"gba" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x432 },
    { (char*)"yzwx", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1432 },
    { (char*)"gbar", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1432 },
    { (char*)"yzwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2432 },
    { (char*)"gbag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2432 },
    { (char*)"yzwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3432 },
    { (char*)"gbab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3432 },
    { (char*)"yzww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4432 },
    { (char*)"gbaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4432 },
    { (char*)"yw"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x42 },
    { (char*)"ga"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x42 },
    { (char*)"ywx" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x142 },
    { (char*)"gar" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x142 },
    { (char*)"ywxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1142 },
    { (char*)"garr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1142 },
    { (char*)"ywxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2142 },
    { (char*)"garg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2142 },
    { (char*)"ywxz", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3142 },
    { (char*)"garb", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3142 },
    { (char*)"ywxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4142 },
    { (char*)"gara", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4142 },
    { (char*)"ywy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x242 },
    { (char*)"gag" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x242 },
    { (char*)"ywyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1242 },
    { (char*)"gagr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1242 },
    { (char*)"ywyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2242 },
    { (char*)"gagg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2242 },
    { (char*)"ywyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3242 },
    { (char*)"gagb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3242 },
    { (char*)"ywyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4242 },
    { (char*)"gaga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4242 },
    { (char*)"ywz" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x342 },
    { (char*)"gab" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x342 },
    { (char*)"ywzx", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1342 },
    { (char*)"gabr", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1342 },
    { (char*)"ywzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2342 },
    { (char*)"gabg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2342 },
    { (char*)"ywzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3342 },
    { (char*)"gabb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3342 },
    { (char*)"ywzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4342 },
    { (char*)"gaba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4342 },
    { (char*)"yww" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x442 },
    { (char*)"gaa" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x442 },
    { (char*)"ywwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1442 },
    { (char*)"gaar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1442 },
    { (char*)"ywwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2442 },
    { (char*)"gaag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2442 },
    { (char*)"ywwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3442 },
    { (char*)"gaab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3442 },
    { (char*)"ywww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4442 },
    { (char*)"gaaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4442 },
    { (char*)"z"   , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3 },
    { (char*)"b"   , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3 },
    { (char*)"zx"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x13 },
    { (char*)"br"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x13 },
    { (char*)"zxx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x113 },
    { (char*)"brr" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x113 },
    { (char*)"zxxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1113 },
    { (char*)"brrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1113 },
    { (char*)"zxxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2113 },
    { (char*)"brrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2113 },
    { (char*)"zxxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3113 },
    { (char*)"brrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3113 },
    { (char*)"zxxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4113 },
    { (char*)"brra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4113 },
    { (char*)"zxy" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x213 },
    { (char*)"brg" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x213 },
    { (char*)"zxyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1213 },
    { (char*)"brgr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1213 },
    { (char*)"zxyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2213 },
    { (char*)"brgg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2213 },
    { (char*)"zxyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3213 },
    { (char*)"brgb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3213 },
    { (char*)"zxyw", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4213 },
    { (char*)"brga", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4213 },
    { (char*)"zxz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x313 },
    { (char*)"brb" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x313 },
    { (char*)"zxzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1313 },
    { (char*)"brbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1313 },
    { (char*)"zxzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2313 },
    { (char*)"brbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2313 },
    { (char*)"zxzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3313 },
    { (char*)"brbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3313 },
    { (char*)"zxzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4313 },
    { (char*)"brba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4313 },
    { (char*)"zxw" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x413 },
    { (char*)"bra" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x413 },
    { (char*)"zxwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1413 },
    { (char*)"brar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1413 },
    { (char*)"zxwy", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2413 },
    { (char*)"brag", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2413 },
    { (char*)"zxwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3413 },
    { (char*)"brab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3413 },
    { (char*)"zxww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4413 },
    { (char*)"braa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4413 },
    { (char*)"zy"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x23 },
    { (char*)"bg"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x23 },
    { (char*)"zyx" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x123 },
    { (char*)"bgr" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x123 },
    { (char*)"zyxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1123 },
    { (char*)"bgrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1123 },
    { (char*)"zyxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2123 },
    { (char*)"bgrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2123 },
    { (char*)"zyxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3123 },
    { (char*)"bgrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3123 },
    { (char*)"zyxw", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4123 },
    { (char*)"bgra", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4123 },
    { (char*)"zyy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x223 },
    { (char*)"bgg" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x223 },
    { (char*)"zyyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1223 },
    { (char*)"bggr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1223 },
    { (char*)"zyyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2223 },
    { (char*)"bggg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2223 },
    { (char*)"zyyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3223 },
    { (char*)"bggb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3223 },
    { (char*)"zyyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4223 },
    { (char*)"bgga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4223 },
    { (char*)"zyz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x323 },
    { (char*)"bgb" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x323 },
    { (char*)"zyzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1323 },
    { (char*)"bgbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1323 },
    { (char*)"zyzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2323 },
    { (char*)"bgbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2323 },
    { (char*)"zyzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3323 },
    { (char*)"bgbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3323 },
    { (char*)"zyzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4323 },
    { (char*)"bgba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4323 },
    { (char*)"zyw" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x423 },
    { (char*)"bga" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x423 },
    { (char*)"zywx", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1423 },
    { (char*)"bgar", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1423 },
    { (char*)"zywy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2423 },
    { (char*)"bgag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2423 },
    { (char*)"zywz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3423 },
    { (char*)"bgab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3423 },
    { (char*)"zyww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4423 },
    { (char*)"bgaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4423 },
    { (char*)"zz"  , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x33 },
    { (char*)"bb"  , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x33 },
    { (char*)"zzx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x133 },
    { (char*)"bbr" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x133 },
    { (char*)"zzxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1133 },
    { (char*)"bbrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1133 },
    { (char*)"zzxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2133 },
    { (char*)"bbrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2133 },
    { (char*)"zzxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3133 },
    { (char*)"bbrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3133 },
    { (char*)"zzxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4133 },
    { (char*)"bbra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4133 },
    { (char*)"zzy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x233 },
    { (char*)"bbg" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x233 },
    { (char*)"zzyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1233 },
    { (char*)"bbgr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1233 },
    { (char*)"zzyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2233 },
    { (char*)"bbgg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2233 },
    { (char*)"zzyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3233 },
    { (char*)"bbgb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3233 },
    { (char*)"zzyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4233 },
    { (char*)"bbga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4233 },
    { (char*)"zzz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x333 },
    { (char*)"bbb" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x333 },
    { (char*)"zzzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1333 },
    { (char*)"bbbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1333 },
    { (char*)"zzzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2333 },
    { (char*)"bbbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2333 },
    { (char*)"zzzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3333 },
    { (char*)"bbbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3333 },
    { (char*)"zzzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4333 },
    { (char*)"bbba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4333 },
    { (char*)"zzw" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x433 },
    { (char*)"bba" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x433 },
    { (char*)"zzwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1433 },
    { (char*)"bbar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1433 },
    { (char*)"zzwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2433 },
    { (char*)"bbag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2433 },
    { (char*)"zzwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3433 },
    { (char*)"bbab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3433 },
    { (char*)"zzww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4433 },
    { (char*)"bbaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4433 },
    { (char*)"zw"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x43 },
    { (char*)"ba"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x43 },
    { (char*)"zwx" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x143 },
    { (char*)"bar" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x143 },
    { (char*)"zwxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1143 },
    { (char*)"barr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1143 },
    { (char*)"zwxy", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2143 },
    { (char*)"barg", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2143 },
    { (char*)"zwxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3143 },
    { (char*)"barb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3143 },
    { (char*)"zwxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4143 },
    { (char*)"bara", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4143 },
    { (char*)"zwy" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x243 },
    { (char*)"bag" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x243 },
    { (char*)"zwyx", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1243 },
    { (char*)"bagr", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1243 },
    { (char*)"zwyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2243 },
    { (char*)"bagg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2243 },
    { (char*)"zwyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3243 },
    { (char*)"bagb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3243 },
    { (char*)"zwyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4243 },
    { (char*)"baga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4243 },
    { (char*)"zwz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x343 },
    { (char*)"bab" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x343 },
    { (char*)"zwzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1343 },
    { (char*)"babr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1343 },
    { (char*)"zwzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2343 },
    { (char*)"babg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2343 },
    { (char*)"zwzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3343 },
    { (char*)"babb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3343 },
    { (char*)"zwzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4343 },
    { (char*)"baba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4343 },
    { (char*)"zww" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x443 },
    { (char*)"baa" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x443 },
    { (char*)"zwwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1443 },
    { (char*)"baar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1443 },
    { (char*)"zwwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2443 },
    { (char*)"baag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2443 },
    { (char*)"zwwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3443 },
    { (char*)"baab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3443 },
    { (char*)"zwww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4443 },
    { (char*)"baaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4443 },
    { (char*)"w"   , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4 },
    { (char*)"a"   , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x4 },
    { (char*)"wx"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x14 },
    { (char*)"ar"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x14 },
    { (char*)"wxx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x114 },
    { (char*)"arr" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x114 },
    { (char*)"wxxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1114 },
    { (char*)"arrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1114 },
    { (char*)"wxxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2114 },
    { (char*)"arrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2114 },
    { (char*)"wxxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3114 },
    { (char*)"arrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3114 },
    { (char*)"wxxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4114 },
    { (char*)"arra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4114 },
    { (char*)"wxy" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x214 },
    { (char*)"arg" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x214 },
    { (char*)"wxyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1214 },
    { (char*)"argr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1214 },
    { (char*)"wxyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2214 },
    { (char*)"argg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2214 },
    { (char*)"wxyz", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3214 },
    { (char*)"argb", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3214 },
    { (char*)"wxyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4214 },
    { (char*)"arga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4214 },
    { (char*)"wxz" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x314 },
    { (char*)"arb" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x314 },
    { (char*)"wxzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1314 },
    { (char*)"arbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1314 },
    { (char*)"wxzy", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2314 },
    { (char*)"arbg", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2314 },
    { (char*)"wxzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3314 },
    { (char*)"arbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3314 },
    { (char*)"wxzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4314 },
    { (char*)"arba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4314 },
    { (char*)"wxw" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x414 },
    { (char*)"ara" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x414 },
    { (char*)"wxwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1414 },
    { (char*)"arar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1414 },
    { (char*)"wxwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2414 },
    { (char*)"arag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2414 },
    { (char*)"wxwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3414 },
    { (char*)"arab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3414 },
    { (char*)"wxww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4414 },
    { (char*)"araa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4414 },
    { (char*)"wy"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x24 },
    { (char*)"ag"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x24 },
    { (char*)"wyx" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x124 },
    { (char*)"agr" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x124 },
    { (char*)"wyxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1124 },
    { (char*)"agrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1124 },
    { (char*)"wyxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2124 },
    { (char*)"agrg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2124 },
    { (char*)"wyxz", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3124 },
    { (char*)"agrb", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x3124 },
    { (char*)"wyxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4124 },
    { (char*)"agra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4124 },
    { (char*)"wyy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x224 },
    { (char*)"agg" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x224 },
    { (char*)"wyyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1224 },
    { (char*)"aggr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1224 },
    { (char*)"wyyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2224 },
    { (char*)"aggg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2224 },
    { (char*)"wyyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3224 },
    { (char*)"aggb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3224 },
    { (char*)"wyyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4224 },
    { (char*)"agga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4224 },
    { (char*)"wyz" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x324 },
    { (char*)"agb" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x324 },
    { (char*)"wyzx", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1324 },
    { (char*)"agbr", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1324 },
    { (char*)"wyzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2324 },
    { (char*)"agbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2324 },
    { (char*)"wyzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3324 },
    { (char*)"agbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3324 },
    { (char*)"wyzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4324 },
    { (char*)"agba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4324 },
    { (char*)"wyw" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x424 },
    { (char*)"aga" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x424 },
    { (char*)"wywx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1424 },
    { (char*)"agar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1424 },
    { (char*)"wywy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2424 },
    { (char*)"agag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2424 },
    { (char*)"wywz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3424 },
    { (char*)"agab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3424 },
    { (char*)"wyww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4424 },
    { (char*)"agaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4424 },
    { (char*)"wz"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x34 },
    { (char*)"ab"  , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x34 },
    { (char*)"wzx" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x134 },
    { (char*)"abr" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x134 },
    { (char*)"wzxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1134 },
    { (char*)"abrr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1134 },
    { (char*)"wzxy", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2134 },
    { (char*)"abrg", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x2134 },
    { (char*)"wzxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3134 },
    { (char*)"abrb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3134 },
    { (char*)"wzxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4134 },
    { (char*)"abra", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4134 },
    { (char*)"wzy" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x234 },
    { (char*)"abg" , (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x234 },
    { (char*)"wzyx", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1234 },
    { (char*)"abgr", (getter)vec_swizzle_getter, (setter)vec_swizzle_setter, NULL, (void*)0x1234 },
    { (char*)"wzyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2234 },
    { (char*)"abgg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2234 },
    { (char*)"wzyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3234 },
    { (char*)"abgb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3234 },
    { (char*)"wzyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4234 },
    { (char*)"abga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4234 },
    { (char*)"wzz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x334 },
    { (char*)"abb" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x334 },
    { (char*)"wzzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1334 },
    { (char*)"abbr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1334 },
    { (char*)"wzzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2334 },
    { (char*)"abbg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2334 },
    { (char*)"wzzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3334 },
    { (char*)"abbb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3334 },
    { (char*)"wzzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4334 },
    { (char*)"abba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4334 },
    { (char*)"wzw" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x434 },
    { (char*)"aba" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x434 },
    { (char*)"wzwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1434 },
    { (char*)"abar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1434 },
    { (char*)"wzwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2434 },
    { (char*)"abag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2434 },
    { (char*)"wzwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3434 },
    { (char*)"abab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3434 },
    { (char*)"wzww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4434 },
    { (char*)"abaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4434 },
    { (char*)"ww"  , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x44 },
    { (char*)"aa"  , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x44 },
    { (char*)"wwx" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x144 },
    { (char*)"aar" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x144 },
    { (char*)"wwxx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1144 },
    { (char*)"aarr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1144 },
    { (char*)"wwxy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2144 },
    { (char*)"aarg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2144 },
    { (char*)"wwxz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3144 },
    { (char*)"aarb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3144 },
    { (char*)"wwxw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4144 },
    { (char*)"aara", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4144 },
    { (char*)"wwy" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x244 },
    { (char*)"aag" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x244 },
    { (char*)"wwyx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1244 },
    { (char*)"aagr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1244 },
    { (char*)"wwyy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2244 },
    { (char*)"aagg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2244 },
    { (char*)"wwyz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3244 },
    { (char*)"aagb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3244 },
    { (char*)"wwyw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4244 },
    { (char*)"aaga", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4244 },
    { (char*)"wwz" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x344 },
    { (char*)"aab" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x344 },
    { (char*)"wwzx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1344 },
    { (char*)"aabr", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1344 },
    { (char*)"wwzy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2344 },
    { (char*)"aabg", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2344 },
    { (char*)"wwzz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3344 },
    { (char*)"aabb", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3344 },
    { (char*)"wwzw", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4344 },
    { (char*)"aaba", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4344 },
    { (char*)"www" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x444 },
    { (char*)"aaa" , (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x444 },
    { (char*)"wwwx", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1444 },
    { (char*)"aaar", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x1444 },
    { (char*)"wwwy", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2444 },
    { (char*)"aaag", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x2444 },
    { (char*)"wwwz", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3444 },
    { (char*)"aaab", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x3444 },
    { (char*)"wwww", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4444 },
    { (char*)"aaaa", (getter)vec_swizzle_getter, (setter)NULL              , NULL, (void*)0x4444 },
    { NULL, NULL, NULL, NULL, NULL }
};
/*
# python script for generating the swizzle get/setters
swizz = set()
for x in range(5):
    for y in range(5):
        for z in range(5):
            for w in range(5):
                s = ''
                if (x > 0): s += str(x)
                if (y > 0): s += str(y)
                if (z > 0): s += str(z)
                if (w > 0): s += str(w)
                if len(s): swizz.add(s)
swizz = list(swizz)
swizz.sort()

for s in swizz:
    name = s.replace('1','x').replace('2','y').replace('3','z').replace('4','w')
    #print(s, name)
    setter = 'NULL              '
    if s.count('1') <= 1 and s.count('2') <= 1 and s.count('3') <= 1 and s.count('4') <= 1:
        setter = 'vec_swizzle_setter'
    print('{ (char*)"%s"%s, (getter)vec_swizzle_getter, (setter)%s, NULL, (void*)0x%s },' % (name, ' '*(4-len(name)), setter, s[::-1]) )
    name = s.replace('1','r').replace('2','g').replace('3','b').replace('4','a')
    print('{ (char*)"%s"%s, (getter)vec_swizzle_getter, (setter)%s, NULL, (void*)0x%s },' % (name, ' '*(4-len(name)), setter, s[::-1]) )

  */




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
        &Vector_SeqMethods,        /*tp_as_sequence*/
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
        Vector_getseters,    /* tp_getset */
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


VectorStruct* new_vec()
{
    return PyObject_New(VectorStruct, Vector_Type());
}

VectorStruct* copy_vec(VectorStruct* self)
{
    auto copy = PyObject_New(VectorStruct, Vector_Type());
    copy->len = self->len;
    for (int i=0; i<4; ++i)
        copy->v[i] = self->v[i];
    return copy;
}

} // extern "C"
} // namespace






bool isVector(void *obj)
{ return PyObject_TypeCheck(reinterpret_cast<PyObject*>(obj), Vector_Type()); }

void initVector(void* mod)
{
    PyObject* module = reinterpret_cast<PyObject*>(mod);
    initObjectType(module, Vector_Type(), MO__VEC_STR);
}

template <class VEC, int len>
void* tmpl_buildVector(const VEC& v)
{
    auto pobj = new_vec();
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
    else if (len == 1) return tmpl_buildVector<const double*, 1>(v);
    else if (len == 2) return tmpl_buildVector<const double*, 2>(v);
    else if (len == 3) return tmpl_buildVector<const double*, 3>(v);
    else if (len == 4) return tmpl_buildVector<const double*, 4>(v);
    else return NULL;
}

void* buildVector(double x, double y)
{
    auto pobj = new_vec();
    pobj->len = 2;
    pobj->v[0] = x; pobj->v[1] = y;
    return pobj;
}

void* buildVector(double x, double y, double z)
{
    auto pobj = new_vec();
    pobj->len = 3;
    pobj->v[0] = x; pobj->v[1] = y; pobj->v[2] = z;;
    return pobj;
}

void* buildVector(double x, double y, double z, double w)
{
    auto pobj = new_vec();
    pobj->len = 4;
    pobj->v[0] = x; pobj->v[1] = y; pobj->v[2] = z; pobj->v[3] = w;
    return pobj;
}

void* buildVector(const MATH::ArithmeticArray<double>& v)
{
    auto pobj = new_vec();
    pobj->len = std::min(size_t(4), v.numDimensions());
    for (int i=0; i<pobj->len; ++i)
        pobj->v[i] = v[i];
    return pobj;
}

DVec2 getVector2(void* pyObject) { return tmpl_getVector<DVec2, 2>(pyObject); }
DVec3 getVector3(void* pyObject) { return tmpl_getVector<DVec3, 3>(pyObject); }
DVec4 getVector4(void* pyObject) { return tmpl_getVector<DVec4, 4>(pyObject); }

bool getVector(void* vobj, int *len, double v[4])
{
    if (!isVector(vobj))
        return false;
    auto self = reinterpret_cast<VectorStruct*>(vobj);
    for (int i = 0; i < self->len; ++i)
        v[i] = self->v[i];
    *len = self->len;
    return true;
}

} // namespace PYTHON34
} // namespace MO


#endif
