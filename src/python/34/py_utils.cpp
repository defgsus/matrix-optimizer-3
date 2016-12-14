/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/2/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include "py_utils.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {

PyObject* toPython(const QString s)
{
    return PyUnicode_FromString(s.toUtf8().constData());
}

PyObject* toPython(long x)
{
    return PyLong_FromLong(x);
}

PyObject* toPython(double x)
{
    return PyFloat_FromDouble(x);
}

PyObject* toPython(bool b)
{
    if (b)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

bool fromPython(PyObject* obj, QString* s)
{
    if (PyUnicode_Check(obj))
    {
        *s = QString::fromUtf8( PyUnicode_AsUTF8(obj) );
        return true;
    }
    return false;
}

bool fromPython(PyObject* obj, long* s)
{
    if (PyLong_Check(obj))
    {
        *s = PyLong_AsLong(obj);
        return true;
    }
    return false;
}

bool fromPython(PyObject* obj, double* val)
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

bool expectFromPython(PyObject* obj, QString* s)
{
    if (fromPython(obj, s))
        return true;
    setPythonError(PyExc_TypeError, QString("Expected string, got %1").arg(typeName(obj)));
    return false;
}

bool expectFromPython(PyObject* obj, long* s)
{
    if (fromPython(obj, s))
        return true;
    setPythonError(PyExc_TypeError, QString("Expected int, got %1").arg(typeName(obj)));
    return false;
}

bool expectFromPython(PyObject* obj, double* val)
{
    if (fromPython(obj, val))
        return true;
    setPythonError(PyExc_TypeError, QString("Expected double, got %1").arg(typeName(obj)));
    return false;
}

template <class T>
bool fromPythonSequenceT(PyObject* seq, T* vec, size_t len)
{
    if (!PySequence_Check(seq))
        return false;
    if (PySequence_Size(seq) != len)
        return false;
    for (Py_ssize_t i=0; i<len; ++i)
    {
        if (!fromPython(PySequence_GetItem(seq, i), &vec[i]))
            return false;
    }
    return true;
}

bool fromPythonSequence(PyObject *seq, QString *vec, size_t len)
{
    return fromPythonSequenceT(seq, vec, len);
}

bool fromPythonSequence(PyObject *seq, long *vec, size_t len)
{
    return fromPythonSequenceT(seq, vec, len);
}

bool fromPythonSequence(PyObject *seq, double *vec, size_t len)
{
    return fromPythonSequenceT(seq, vec, len);
}


template <class T>
bool expectFromPythonSequenceT(PyObject* seq, T* vec, size_t len, const QString& type)
{
    if (!PySequence_Check(seq))
    {
        setPythonError(PyExc_TypeError, QString("Expected sequence of %1, got %2")
                       .arg(type).arg(typeName(seq)));
        return false;
    }
    if (PySequence_Size(seq) != len)
    {
        setPythonError(PyExc_ValueError, QString("Expected sequence of length %1, got %2")
                       .arg(len).arg(PySequence_Size(seq)));
        return false;
    }
    for (Py_ssize_t i=0; i<len; ++i)
    {
        if (!expectFromPython(PySequence_GetItem(seq, i), &vec[i]))
            return false;
    }
    return true;
}

bool expectFromPythonSequence(PyObject *seq, QString *vec, size_t len)
{
    return expectFromPythonSequenceT(seq, vec, len, "string");
}

bool expectFromPythonSequence(PyObject *seq, long *vec, size_t len)
{
    return expectFromPythonSequenceT(seq, vec, len, "int");
}

bool expectFromPythonSequence(PyObject *seq, double *vec, size_t len)
{
    return expectFromPythonSequenceT(seq, vec, len, "float");
}


bool checkIndex(Py_ssize_t index, Py_ssize_t len)
{
    if (index >= len)
    {
        PyErr_Set(PyExc_IndexError,
                  QString("Index out of range, %1 >= %2").arg(index).arg(len));
        return false;
    }
    return true;
}





void setPythonError(PyObject* exc, const QString& txt)
{
    PyErr_SetObject(exc, toPython(txt));
}

QString typeName(const PyObject *arg)
{
    if (!arg)
        return "NULL";
    auto s = QString(arg->ob_type->tp_name);
    return s;
}

bool iterateSequence(PyObject* seq, std::function<bool(PyObject*item)> foo)
{
    if (!PySequence_Check(seq))
    {
        PyErr_Set(PyExc_TypeError, QString("expected sequence, got %1")
                  .arg(typeName(seq)));
        return false;
    }
    Py_ssize_t size = PySequence_Size(seq);
    for (Py_ssize_t i = 0; i < size; ++i)
    {
        auto item = PySequence_GetItem(seq, i);
        if (!item)
        {
            PyErr_Set(PyExc_TypeError, QString("NULL object in sequence[%1]").arg(i));
            return false;
        }
        if (!foo(item))
            return false;
    }
    return true;
}









// ------- oldstuff --------

PyObject* fromString(const QString& s)
{
    return PyUnicode_FromString(s.toUtf8().constData());
}

QString toString(PyObject* o)
{
    return QString::fromUtf8( PyUnicode_AsUTF8(o) );
}


PyObject* fromInt(int i)
{
    auto obj = PyLong_FromLong(i);
    if (obj == NULL)
        PyErr_SetString(PyExc_MemoryError, "failed to create long object");
    return obj;
}

PyObject* fromLong(long i)
{
    auto obj = PyLong_FromLong(i);
    if (obj == NULL)
        PyErr_SetString(PyExc_MemoryError, "failed to create long object");
    return obj;
}

PyObject* fromDouble(double v)
{
    auto obj = PyFloat_FromDouble(v);
    if (obj == NULL)
        PyErr_SetString(PyExc_MemoryError, "failed to create float object");
    return obj;
}

bool toDouble(PyObject* obj, double* val)
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


void initObjectType(PyObject* module, PyTypeObject* type, const char* name)
{
    if (0 != PyType_Ready(type))
        MO_ERROR("Failed to readify " << name << " for Python 3.4");
    //dumpObject((PyObject*)type, false);

    PyObject* object = reinterpret_cast<PyObject*>(type);
    Py_INCREF(object);
    if (0 != PyModule_AddObject(module, name, object))
    {
        Py_DECREF(object);
        MO_ERROR("Failed to add " << name << " to Python 3.4");
    }
}

void PyErr_Set(PyObject* exc, const QString& txt)
{
    PyErr_SetObject(exc, fromString(txt));
}


PyObject* removeArgumentTuple(PyObject* arg)
{
    if (PyTuple_Check(arg) && PyTuple_Size(arg) == 1)
        return PyTuple_GetItem(arg, 0);
    return arg;
}


void dumpObject(PyObject* arg, bool introspect)
{
    #define MO__PRINTPY(what__) \
        if (what__) \
        { \
            if (introspect) \
            { \
                auto s = PyObject_CallMethod(what__, "__str__", ""); \
                MO_PRINT(#what__ ": " << PyUnicode_AsUTF8(s)); \
            } \
            else MO_PRINT(#what__ ": " << what__); \
        } else MO_PRINT(#what__ ": NULL");

    #define MO__PRINT(what__) \
        MO_PRINT(#what__ ": " << what__);

    MO__PRINTPY(arg);
    if (arg)
    {
        MO__PRINT(arg->ob_refcnt);
        MO__PRINT(arg->ob_type);
        if (arg->ob_type)
        {
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
    }
#undef MO__PRINT
#undef MO__PRINTPY
}

} // namespace PYTHON34
} // namespace MO

#endif // MO_ENABLE_PYTHON34
