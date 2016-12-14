#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_VECTOR_HELPER_H
#define MOSRC_PYTHON_34_VECTOR_HELPER_H

#include <functional>

#include "py_utils.h"

namespace MO {
namespace PYTHON34 {

template <class T>
PyObject* vectorSet(T* self, PyObject* args);

template <class T>
bool vectorBinaryOpInplace(T* self, T* arg, std::function<void(double&, double)> op);

template <class T>
bool vectorBinaryOpInplace(T* self, PyObject* arg, std::function<void(double&,double)> op);

template <class T>
PyObject* vectorUnaryOpInplace(T* self, std::function<void(double&)> op);

template <class T>
PyObject* vectorRichCompare(T* self, PyObject* arg, int cmp);

/*template <class T, class TS>
PyObject* vectorBinaryOpCopy(PyObject* left, PyObject* right,
                             TS* typeStruct,
                             std::function<T*()> new_func,
                             std::function<double(double,double)> op);
*/

// --------------------- implementation --------------------------

template <class T>
PyObject* vectorSet(T* self, PyObject* args)
{
    PyObject* arg = nullptr;
    if (!PyArg_ParseTuple(args, "|O", &arg))
        return NULL;
    if (!arg)
    {
        for (size_t i=0; i<self->len; ++i)
            self->v[i] = 0.;
        Py_RETURN_SELF;
    }
    double v;
    if (fromPython(arg, &v))
    {
        for (size_t i=0; i<self->len; ++i)
            self->v[i] = v;
        Py_RETURN_SELF;
    }
    if (!expectFromPythonSequence(arg, self->v, self->len))
        return NULL;
    Py_RETURN_SELF;
}

/*
template <class T, class TS>
PyObject* vectorBinaryOpCopy(PyObject* left, PyObject* right,
                             TS* typeStruct,
                             std::function<T*()> new_func,
                             std::function<double(double,double)> op)
{
    bool reverse = false;
    if (!PyObject_TypeCheck(left, typeStruct))
    {
        reverse = true;
        std::swap(left, right);
        if (!PyObject_TypeCheck(left, typeStruct))
        {
            setPythonError(PyExc_TypeError,
                      QString("Wrong arguments to binary operator, %1 and %2")
                      .arg(typeName(left)).arg(typeName(right)));
            return NULL;
        }
    }
    if (!left)
    {
        setPythonError(PyExc_TypeError, QString("Null argument to operator"));
        return NULL;
    }
    auto self = (T*)left;
    auto ret = (T*)new_func();
    double v;
    if (fromPython(right, &v))
    {
        if (!reverse)
        {
            for (size_t i=0; i<self->len; ++i)
                ret->v[i] = op(self->v[i], v);
        }
        else
        {
            for (size_t i=0; i<self->len; ++i)
                ret->v[i] = op(v, self->v[i]);
        }
        return (PyObject*)ret;
    }
    double vec[self->len];
    if (!expectFromPython(right, vec, self->len))
    {
        Py_DECREF(ret);
        return NULL;
    }
    if (!reverse)
    {
        for (size_t i=0; i<self->len; ++i)
            ret->v[i] = op(self->v[i], vec[i]);
    }
    else
    {
        for (size_t i=0; i<self->len; ++i)
            ret->v[i] = op(vec[i], self->v[i]);
    }
    return (PyObject*)ret;
}
*/

template <class T>
bool vectorBinaryOpInplace(T* self, T* arg, std::function<void(double&, double)> op)
{
    for (size_t i=0; i<self->len; ++i)
        op(self->v[i], arg->v[i]);
    return true;
}

template <class T>
bool vectorBinaryOpInplace(T* self, PyObject* right,
                           std::function<void(double&, double)> op)
{
    if (!self)
    {
        setPythonError(PyExc_TypeError, QString("Null argument to operator"));
        return false;
    }
    double v;
    if (fromPython(right, &v))
    {
        for (size_t i=0; i<self->len; ++i)
            op(self->v[i], v);
        return true;
    }
    double vec[self->len];
    if (!expectFromPythonSequence(right, vec, self->len))
        return false;
    for (size_t i=0; i<self->len; ++i)
        op(self->v[i], vec[i]);
    return true;
}

template <class T>
PyObject* vectorUnaryOpInplace(T* self, std::function<void(double&)> op)
{
    for (size_t i=0; i<self->len; ++i)
        op(self->v[i]);
    Py_RETURN_SELF;
}

template <class T>
PyObject* vectorRichCompare(T* self, PyObject* arg, int cmp)
{
    double v[self->len];
    if (!fromPythonSequence(arg, v, self->len))
        Py_RETURN_FALSE;

    #define MO__COMP(op__) \
        for (size_t i=0; i<self->len; ++i) \
            if (!(self->v[i] op__ v[i])) \
                Py_RETURN_FALSE; \
        Py_RETURN_TRUE; \

    switch (cmp)
    {
        case Py_LT: MO__COMP(<)
        case Py_LE: MO__COMP(<=)
        default:
        case Py_EQ: MO__COMP(==)
        case Py_NE: MO__COMP(!=)
        case Py_GT: MO__COMP(>)
        case Py_GE: MO__COMP(>=)
    }

    #undef MO__COMP
}


} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_VECTOR_HELPER_H

#endif // MO_ENABLE_PYTHON34
