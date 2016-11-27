#ifdef MO_ENABLE_PYTHON34

#include "py_utils.h"

#include <functional>

#include <QString>

#include <glm/gtc/matrix_transform.hpp>

#include "python_matrix4.h"
#include "python_vector.h"
#include "math/vector.h"
#include "math/functions.h"
#include "math/ArithmeticArray.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {

namespace {
extern "C" {

static const char* Mat4DocString()
{
    static const char* str =
            "A 4x4 geometric matrix";
    return str;
}


#define MO__MAT4_STR "Mat4"

struct Mat4Struct
{
    PyObject_HEAD
    DMat4 mat;
    double& v(int idx) { return data()[idx]; }
    double* data() { return &mat[0][0]; }
    static const int len = 16;

    static void dealloc(Mat4Struct* self)
    {
        self->ob_base.ob_type->tp_free((PyObject*)self);
    }


};

static Mat4Struct* new_mat4();
static Mat4Struct* copy_mat4(Mat4Struct*);

} // extern "C"
} // namespace




namespace {
extern "C" {


static int mat4_init(Mat4Struct* self, PyObject* args_, PyObject*)
{
    self->mat = Mat4(1);

    bool was_scalar;
    if (get_vector(args_, self->len, self->data(), &was_scalar))
    {
        if (was_scalar)
            self->mat = DMat4(self->mat[0][0]);
        return 0;
    }

    PyErr_Set(PyExc_TypeError,
              QString("argument '%1' not convertible to Mat4"));

    return -1;
}

static PyObject* mat4_newfunc(PyTypeObject* type, PyObject* , PyObject* )
{
    auto self = reinterpret_cast<Mat4Struct*>(type->tp_alloc(type, 0));

    if (self != NULL)
    {
        self->mat = Mat4(1);
    }

    return reinterpret_cast<PyObject*>(self);
}


    // -------------- various helper -------------------


    // executes foo on self and returns self
    PyObject* mat4_inplace_func(Mat4Struct* self,
                           std::function<void(Mat4Struct*)> foo)
    {
        foo(self);
        Py_INCREF(self);
        return reinterpret_cast<PyObject*>(self);
    }

    // executes foo on copy of self and returns copy
    // foo is the varags CPython func
    PyObject* mat4_copy_of_inplace(Mat4Struct* self, PyObject* args,
                              PyObject*(*foo)(Mat4Struct*, PyObject*))
    {
        auto copy = copy_mat4(self);
        foo(copy, args);
        Py_DECREF(copy);
        return reinterpret_cast<PyObject*>(copy);
    }


    static PyObject* mat4_operator(
            Mat4Struct* self, PyObject* other,
            std::function<double(double,double)> op)
    {
        double v[self->len], v2[self->len];
        if (!get_vector(other, self->len, v))
            return NULL;

        for (int i=0; i<self->len; ++i)
            v2[i] = op(self->v(i), v[i]);

        return buildMat4(v2);
    }



    static PyObject* mat4_operator_inp(
            Mat4Struct* self, PyObject* other,
            std::function<void(double&,double)> op)
    {
        double v[self->len];
        if (!get_vector(other, self->len, v))
            return NULL;

        for (int i=0; i<self->len; ++i)
            op(self->v(i), v[i]);

        Py_INCREF(self);
        return reinterpret_cast<PyObject*>(self);
    }


#define MO_PY_DEF_DOC(name__, str__) \
    static const char* name__##_doc = str__;

    // ---------------- arithmetic --------------------

    // classic operator

    static PyObject* mat4_add(Mat4Struct* self, PyObject* other)
    {
        return mat4_operator(self, other, [](double x, double y)
            { return x + y; });
    }

    static PyObject* mat4_sub(Mat4Struct* self, PyObject* other)
    {
        return mat4_operator(self, other, [](double x, double y)
            { return x - y; });
    }

    static PyObject* mat4_mul(Mat4Struct* self, PyObject* other)
    {
        int len; DVec4 vec(0,0,0,1);
        if (isMat4(other))
        {
            return buildMat4(self->mat * getMat4(other));
        }
        if (get_vector_var(other, &len, &vec[0]))
        {
            if (len != 1)
            {
                if (len != 3 && len != 4)
                {
                    PyErr_Set(PyExc_TypeError,
                        QString("Can't multiply Mat4 and Vec with length %1")
                              .arg(len));
                    return NULL;
                }
                vec = self->mat * vec;
                if (len == 4)
                    return buildVector(vec);
                else
                    return buildVector(vec[0], vec[1], vec[2]);
            }
            auto m = new_mat4();
            for (int i=0; i<m->len; ++i)
                m->v(i) = self->v(i) * vec[0];
            return reinterpret_cast<PyObject*>(m);
        }
        PyErr_Set(PyExc_TypeError,
            QString("Can't multiply Mat4 and %1").arg(typeName(other)));
        return NULL;
    }

    static PyObject* mat4_div(Mat4Struct* self, PyObject* other)
    {
        return mat4_operator(self, other, [](double x, double y)
            { return x / y; });
    }

    // inplace operators

    static PyObject* mat4_iadd(Mat4Struct* self, PyObject* other)
    {
        return mat4_operator_inp(self, other, [](double& x, double y)
            { x += y; });
    }

    static PyObject* mat4_isub(Mat4Struct* self, PyObject* other)
    {
        return mat4_operator_inp(self, other, [](double& x, double y)
            { x -= y; });
    }

    static PyObject* mat4_imul(Mat4Struct* self, PyObject* other)
    {
        if (!isMat4(other))
        {
            PyErr_Set(PyExc_TypeError,
                      QString("Can't multiply %s and %s")
                      .arg(MO__MAT4_STR).arg(typeName(other)));
            return NULL;
        }
        self->mat *= getMat4(other);
        Py_IncRef(reinterpret_cast<PyObject*>(self));
        return reinterpret_cast<PyObject*>(self);
    }

    static PyObject* mat4_idiv(Mat4Struct* self, PyObject* other)
    {
        return mat4_operator_inp(self, other, [](double& x, double y)
            { x /= y; });
    }

    static PyObject* mat4_imod(Mat4Struct* self, PyObject* other)
    {
        return mat4_operator_inp(self, other, [](double& x, double y)
            { x = MATH::modulo(x, y); });
    }

    // ----- other number methods -----

    static PyObject* mat4_abs_unary(Mat4Struct* self)
    {
        auto res = copy_mat4(self);
        for (int i=0; i<res->len; ++i)
            res->v(i) = std::abs(res->v(i));
        return reinterpret_cast<PyObject*>(res);
    }

    static PyObject* mat4_neg_unary(Mat4Struct* self)
    {
        auto res = copy_mat4(self);
        for (int i=0; i<res->len; ++i)
            res->v(i) = -res->v(i);
        return reinterpret_cast<PyObject*>(res);
    }


    // ---- operator-like functions, e.g. m.rotate(v) ----

    MO_PY_DEF_DOC(mat4_rotate_x,
        "rotate_x(degree) -> " MO__MAT4_STR "\n"
        "Rotates the matrix about X axis, *INPLACE*"
    )
    static PyObject* mat4_rotate_x(Mat4Struct* self, PyObject* arg)
    {
        return mat4_inplace_func(self, [=](Mat4Struct* self)
        {
            double deg;
            if (!PyArg_ParseTuple(arg, "d", &deg))
                return false;
            self->mat = MATH::rotate(self->mat, deg, DVec3(1,0,0));
            return true;
        });
    }

    MO_PY_DEF_DOC(mat4_rotate_y,
        "rotate_y(degree) -> " MO__MAT4_STR "\n"
        "Rotates the matrix about Y axis, *INPLACE*"
    )
    static PyObject* mat4_rotate_y(Mat4Struct* self, PyObject* arg)
    {
        return mat4_inplace_func(self, [=](Mat4Struct* self)
        {
            double deg;
            if (!PyArg_ParseTuple(arg, "d", &deg))
                return false;
            self->mat = MATH::rotate(self->mat, deg, DVec3(0,1,0));
            return true;
        });
    }

    MO_PY_DEF_DOC(mat4_rotate_z,
        "rotate_z(degree) -> " MO__MAT4_STR "\n"
        "Rotates the matrix about Z axis, *INPLACE*"
    )
    static PyObject* mat4_rotate_z(Mat4Struct* self, PyObject* arg)
    {
        return mat4_inplace_func(self, [=](Mat4Struct* self)
        {
            double deg;
            if (!PyArg_ParseTuple(arg, "d", &deg))
                return false;
            self->mat = MATH::rotate(self->mat, deg, DVec3(0,0,1));
            return true;
        });
    }

    MO_PY_DEF_DOC(mat4_rotated_x,
        "rotated_x(degree) -> " MO__MAT4_STR "\n"
        "Returns Rotated matrix about X axis"
    )
    static PyObject* mat4_rotated_x(Mat4Struct* self, PyObject* arg)
    {
        return mat4_copy_of_inplace(self, arg, mat4_rotate_x);
    }

    MO_PY_DEF_DOC(mat4_rotated_y,
        "rotated_y(degree) -> " MO__MAT4_STR "\n"
        "Returns Rotated matrix about Y axis"
    )
    static PyObject* mat4_rotated_y(Mat4Struct* self, PyObject* arg)
    {
        return mat4_copy_of_inplace(self, arg, mat4_rotate_y);
    }

    MO_PY_DEF_DOC(mat4_rotated_z,
        "rotated_z(degree) -> " MO__MAT4_STR "\n"
        "Returns Rotated matrix about Z axis"
    )
    static PyObject* mat4_rotated_z(Mat4Struct* self, PyObject* arg)
    {
        return mat4_copy_of_inplace(self, arg, mat4_rotate_z);
    }



    MO_PY_DEF_DOC(mat4_translate,
        "translate(Vec) -> " MO__MAT4_STR "\n"
        "Translates the matrix, *INPLACE*"
    )
    static PyObject* mat4_translate(Mat4Struct* self, PyObject* arg)
    {
        return mat4_inplace_func(self, [=](Mat4Struct* self)
        {
            double v[4];
            if (!get_vector(arg, 3, v))
                return false;

            self->mat = glm::translate(self->mat, DVec3(v[0], v[1], v[2]));
            return true;
        });
    }

    MO_PY_DEF_DOC(mat4_normalize,
        "normalize() -> " MO__MAT4_STR "\n"
        "Normalizes each column of the matrix.\n"
        "Ignored when all components are zero.\n"
        "Returns self"
    )
    static PyObject* mat4_normalize(Mat4Struct* self, PyObject* )
    {
        return mat4_inplace_func(self, [](Mat4Struct* self)
        {
            for (int j=0; j<4; ++j)
            {
                double mag = 0;
                for (int i=0; i<4; ++i)
                    mag += self->v(j*4+i) * self->v(j*4+i);
                if (mag > 0.)
                {
                    mag = std::sqrt(mag);
                    for (int i=0; i<self->len; ++i)
                        self->v(j*4+i) /= mag;
                }
            }
        });
    }

    MO_PY_DEF_DOC(mat4_normalized,
        "normalized() -> " MO__MAT4_STR "\n"
        "Returns a column-normalized matrix.\n"
        "Returns a copy when all components are zero."
    )
    static PyObject* mat4_normalized(Mat4Struct* self, PyObject* )
    {
        return mat4_copy_of_inplace(self, NULL, mat4_normalize);
    }


    MO_PY_DEF_DOC(mat4_invert,
        "invert() -> " MO__MAT4_STR "\n"
        "Inverts the matrix INPLACE.\n"
        "Returns self"
    )
    static PyObject* mat4_invert(Mat4Struct* self, PyObject* )
    {
        self->mat = glm::inverse(self->mat);
        Py_IncRef(reinterpret_cast<PyObject*>(self));
        return reinterpret_cast<PyObject*>(self);
    }

    MO_PY_DEF_DOC(mat4_inverted,
        "inverted() -> " MO__MAT4_STR "\n"
        "Returns a inverted matrix."
    )
    static PyObject* mat4_inverted(Mat4Struct* self, PyObject* )
    {
        return mat4_copy_of_inplace(self, NULL, mat4_invert);
    }


    MO_PY_DEF_DOC(mat4_clear_position,
        "clear_position() -> " MO__MAT4_STR "\n"
        "Deletes the translation data.\n"
        "Returns self"
    )
    static PyObject* mat4_clear_position(Mat4Struct* self, PyObject* )
    {
        self->mat[3][0] = 0.;
        self->mat[3][1] = 0.;
        self->mat[3][2] = 0.;
        Py_IncRef(reinterpret_cast<PyObject*>(self));
        return reinterpret_cast<PyObject*>(self);
    }

    MO_PY_DEF_DOC(mat4_position_cleared,
        "position_cleared() -> " MO__MAT4_STR "\n"
        "Returns a matrix with translation deleted."
    )
    static PyObject* mat4_position_cleared(Mat4Struct* self, PyObject* )
    {
        return mat4_copy_of_inplace(self, NULL, mat4_clear_position);
    }

    MO_PY_DEF_DOC(mat4_position,
        "position() -> Vec\n"
        "Returns a Vec3 with the translational matrix data."
    )
    static PyObject* mat4_position(Mat4Struct* self, PyObject* )
    {
        return buildVector(self->mat[3][0], self->mat[3][1], self->mat[3][2]);
    }


    MO_PY_DEF_DOC(mat4_clamp,
        "clamp(f, f) -> " MO__MAT4_STR "\n"
        "Clamps all components to the range [f, f]"
    )
    static PyObject* mat4_clamp(Mat4Struct* self, PyObject* other)
    {
        double mi, ma;
        if (!PyArg_ParseTuple(other, "dd", &mi, &ma))
            return NULL;
        for (int i = 0; i<self->len; ++i)
            self->v(i) = std::max(mi,std::min(ma, self->v(i)));
        Py_INCREF(self);
        return reinterpret_cast<PyObject*>(self);
    }


    // ----------------- getter -----------------------

    MO_PY_DEF_DOC(mat4_to_string,
        "to_string() -> string\n"
        "Returns the string representation of the matrix"
    )
    static PyObject* mat4_to_string(Mat4Struct* self, PyObject* )
    {
        QString s = MO__MAT4_STR;
        for (int i=0; i<4; ++i)
            s += QString( "(%1,%2,%3,%4)")
                    .arg(self->v(i*4+0))
                    .arg(self->v(i*4+1))
                    .arg(self->v(i*4+2))
                    .arg(self->v(i*4+3));
        return fromString(s);
    }
    static PyObject* mat4_repr(PyObject* self)
        { return mat4_to_string(reinterpret_cast<Mat4Struct*>(self), nullptr); }


    MO_PY_DEF_DOC(mat4_to_tuple,
        "to_tuple() -> tuple\n"
        "Returns the matrix components in a tuple"
    )
    static PyObject* mat4_to_tuple(Mat4Struct* self, PyObject* )
    {
        auto tuple = PyTuple_New(self->len);
        if (!tuple)
        {
            PyErr_SetString(PyExc_MemoryError, "could not create tuple");
            return NULL;
        }
        for (int i=0; i<self->len; ++i)
        {
            auto obj = fromDouble(self->v(i));
            if (!obj)
            {
                Py_DECREF(tuple);
                return NULL;
            }
            PyTuple_SET_ITEM(tuple, i, obj);
        }
        return tuple;
    }

    MO_PY_DEF_DOC(mat4_to_list,
        "to_list() -> list\n"
        "Returns the matrix components in a list"
    )
    static PyObject* mat4_to_list(Mat4Struct* self, PyObject* )
    {
        auto list = PyList_New(self->len);
        if (!list)
        {
            PyErr_SetString(PyExc_MemoryError, "could not create list");
            return NULL;
        }
        for (int i=0; i<self->len; ++i)
        {
            auto obj = fromDouble(self->v(i));
            if (!obj)
            {
                Py_DECREF(list);
                return NULL;
            }
            PyList_SET_ITEM(list, i, obj);
        }
        return list;
    }

    MO_PY_DEF_DOC(mat4_copy,
        "copy() -> Vec\n"
        "Returns a copy of the vector matrix"
    )
    static PyObject* mat4_copy(Mat4Struct* self, PyObject* )
    {
        return buildMat4(self->mat);
    }


    MO_PY_DEF_DOC(mat4_len,
        "len() -> int\n"
        "Returns the length of the matrix (16)."
    )
    static PyObject* mat4_len(Mat4Struct* self, PyObject* )
    {
        return Py_BuildValue("l", self->len);
    }




    // gets single component by index [1,4]
    static PyObject* mat4_single_getter(Mat4Struct* self, void* ptr)
    {
        int idx = int64_t(ptr) - 1;
        if (idx < 0 || idx >= self->len)
        {
            PyErr_Set(PyExc_IndexError, QString("index in vector out of range %1/%2")
                      .arg(idx).arg(self->len));
            return NULL;
        }
        return PyFloat_FromDouble(self->v(idx));
    }



    // --- sequence funcs ---
    static Py_ssize_t mat4_sq_length(Mat4Struct* self)
    {
        return self->len;
    }

    static PyObject* mat4_sq_item(Mat4Struct* self, Py_ssize_t idx)
    {
        return mat4_single_getter(self, (void*)(idx+1));
    }

    static int mat4_sq_ass_item(Mat4Struct* self, Py_ssize_t idx, PyObject* arg)
    {
        if (idx >= self->len)
        {
            PyErr_Set(PyExc_IndexError,
                      QString("index in vector out of range %1/%2")
                      .arg(idx).arg(self->len));
            return -1;
        }
        double tmp;
        if (!toDouble(arg, &tmp))
            return -1;

        self->v(idx) = tmp;
        return 0;
    }



static PyMemberDef Mat4_members[] =
{
    { NULL, 0, 0, 0, NULL }
};

#define MO__METHOD(name__, args__) \
    { #name__, (PyCFunction)mat4_##name__, args__, mat4_##name__##_doc },

static PyMethodDef Mat4_methods[] =
{
    MO__METHOD(to_string,           METH_NOARGS)
    MO__METHOD(to_tuple,            METH_NOARGS)
    MO__METHOD(to_list,             METH_NOARGS)
    MO__METHOD(copy,                METH_NOARGS)

    MO__METHOD(len,                 METH_NOARGS)
    MO__METHOD(clamp,               METH_VARARGS)

    MO__METHOD(normalize,           METH_NOARGS)
    MO__METHOD(normalized,          METH_NOARGS)
    MO__METHOD(invert,              METH_NOARGS)
    MO__METHOD(inverted,            METH_NOARGS)
    MO__METHOD(clear_position,      METH_NOARGS)
    MO__METHOD(position_cleared,    METH_NOARGS)
    MO__METHOD(position,            METH_NOARGS)

    MO__METHOD(rotate_x,            METH_VARARGS)
    MO__METHOD(rotate_y,            METH_VARARGS)
    MO__METHOD(rotate_z,            METH_VARARGS)
    MO__METHOD(rotated_x,           METH_VARARGS)
    MO__METHOD(rotated_y,           METH_VARARGS)
    MO__METHOD(rotated_z,           METH_VARARGS)
    MO__METHOD(translate,           METH_VARARGS)

    { NULL, NULL, 0, NULL }
};
#undef MO__METHOD

static PyNumberMethods Mat4_NumMethods = {
    (binaryfunc)    mat4_add, /*nb_add*/
    (binaryfunc)    mat4_sub, /*nb_subtract*/
    (binaryfunc)    mat4_mul, /*nb_multiply*/
    (binaryfunc)    NULL, /*nb_remainder*/
    NULL,                       /*nb_divmod*/
    (ternaryfunc)   NULL, /*nb_power*/
    (unaryfunc)     mat4_neg_unary, /*nb_negative*/
    (unaryfunc)     copy_mat4,/*tp_positive*/
    (unaryfunc)     mat4_abs_unary,       /*tp_absolute*/
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
    (binaryfunc)mat4_iadd,                /* nb_inplace_add */
    (binaryfunc)mat4_isub,                /* nb_inplace_subtract */
    (binaryfunc)mat4_imul,                /* nb_inplace_multiply */
    (binaryfunc)mat4_imod,       /* nb_inplace_remainder */
    NULL,                       /* nb_inplace_power */
    NULL,                       /* nb_inplace_lshift */
    NULL,                       /* nb_inplace_rshift */
    NULL,                       /* nb_inplace_and */
    NULL,                       /* nb_inplace_xor */
    NULL,                       /* nb_inplace_or */
    NULL,                       /* nb_floor_divide */
    (binaryfunc)mat4_div,                 /* nb_true_divide */
    NULL,                       /* nb_inplace_floor_divide */
    (binaryfunc)mat4_idiv,                /* nb_inplace_true_divide */
    NULL                        /* nb_index */
};

PySequenceMethods Mat4_SeqMethods = {
    (lenfunc)mat4_sq_length, /* lenfunc sq_length */
    NULL, /* binaryfunc sq_concat */
    NULL, /* ssizeargfunc sq_repeat */
    (ssizeargfunc)mat4_sq_item, /* ssizeargfunc sq_item */
    NULL, /* void *was_sq_slice */
    (ssizeobjargproc)mat4_sq_ass_item, /* ssizeobjargproc sq_ass_item */
    NULL, /* void *was_sq_ass_slice */
    NULL, /* objobjproc sq_contains */
    NULL, /* binaryfunc sq_inplace_concat */
    NULL  /* ssizeargfunc sq_inplace_repeat */
};




static PyTypeObject* Matrix_Type()
{
    static PyTypeObject type =
    {
        PyVarObject_HEAD_INIT(NULL, 0)
        "matrixoptimizer.Mat",  /*tp_name*/
        sizeof(Mat4Struct),  /*tp_basicsize*/
        0,                         /*tp_itemsize*/
        (destructor)Mat4Struct::dealloc,/*tp_dealloc*/
        0,                         /*tp_print*/
        0,                         /*tp_getattr*/
        0,                         /*tp_setattr*/
        0,                         /*tp_reserved*/
        mat4_repr,     /*tp_repr*/
        &Mat4_NumMethods,         /*tp_as_number*/
        &Mat4_SeqMethods,        /*tp_as_sequence*/
        0,                         /*tp_as_mapping*/
        0,                         /*tp_hash */
        0,                         /*tp_call*/
        mat4_repr,     /*tp_str*/
        PyObject_GenericGetAttr,   /*tp_getattro*/
        PyObject_GenericSetAttr,   /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
        Mat4DocString(),   /* tp_doc */
        0,		               /* tp_traverse */
        0,		               /* tp_clear */
        0,		               /* tp_richcompare */
        0,		               /* tp_weaklistoffset */
        0,//VectorFuncs::get_iter,	/* tp_iter */
        0,//VectorFuncs::next_iter, /* tp_iternext */
        Mat4_methods,      /* tp_methods */
        Mat4_members,      /* tp_members */
        0,    /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        (initproc)mat4_init,      /* tp_init */
        0,                         /* tp_alloc */
        (newfunc)mat4_newfunc,       /* tp_new */
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


Mat4Struct* new_mat4()
{
    return PyObject_New(Mat4Struct, Matrix_Type());
}

Mat4Struct* copy_mat4(Mat4Struct* self)
{
    auto copy = PyObject_New(Mat4Struct, Matrix_Type());
    copy->mat = self->mat;
    return copy;
}

} // extern "C"
} // namespace






bool isMat4(PyObject* obj)
{ return PyObject_TypeCheck(obj, Matrix_Type()); }

void initMat4(PyObject* module)
{
    initObjectType(module, Matrix_Type(), MO__MAT4_STR);
}

template <class VEC, int len>
PyObject* tmpl_buildVector(const VEC& v)
{
    auto pobj = new_mat4();
    pobj->len = len;
    for (int i=0; i<len; ++i)
        pobj->v(i) = v[i];
    return reinterpret_cast<PyObject*>(pobj);
}


PyObject* buildMat4(const double v[16])
{
    auto m = new_mat4();
    for (int i=0; i<16; ++i)
        m->v(i) = v[i];
    return reinterpret_cast<PyObject*>(m);
}

PyObject* buildMat4(const DMat4& mat)
{
    auto m = new_mat4();
    m->mat = mat;
    return reinterpret_cast<PyObject*>(m);
}

PyObject* buildVector(const Mat4& mat)
{
    auto m = new_mat4();
    m->mat = mat;
    return reinterpret_cast<PyObject*>(m);
}


DMat4 getMat4(PyObject* arg)
{
    if (!isMat4(arg))
        return DMat4(0.);
    auto self = reinterpret_cast<Mat4Struct*>(arg);
    return self->mat;
}

bool getMat4(PyObject* arg, double v[16])
{
    if (!isMat4(arg))
        return false;
    auto self = reinterpret_cast<Mat4Struct*>(arg);
    for (int i = 0; i < self->len; ++i)
        v[i] = self->v(i);
    return true;
}











} // namespace PYTHON34
} // namespace MO

#endif // MO_ENABLE_PYTHON34
