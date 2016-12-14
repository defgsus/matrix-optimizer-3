#include "py_utils.h"

namespace PyUtils {


PyObject* toPython(const std::string& s)
{
    return PyUnicode_FromString(s.data());
}

PyObject* toPython(const char* s)
{
    return PyUnicode_FromString(s);
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

PyObject* toPython(long unsigned x) { return toPython((long)x); }
PyObject* toPython(int x) { return toPython((long)x); }


bool fromPython(PyObject* obj, std::string* s)
{
    if (PyUnicode_Check(obj))
    {
        *s = PyUnicode_AsUTF8(obj);
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

bool expectFromPython(PyObject* obj, std::string* s)
{
    if (fromPython(obj, s))
        return true;
    setPythonError(PyExc_TypeError,
                   SStream() << "Expected string, got " << typeName(obj));
    return false;
}

bool expectFromPython(PyObject* obj, long* s)
{
    if (fromPython(obj, s))
        return true;
    setPythonError(PyExc_TypeError,
                   SStream() << "Expected int, got " << typeName(obj));
    return false;
}

bool expectFromPython(PyObject* obj, double* val)
{
    if (fromPython(obj, val))
        return true;
    setPythonError(PyExc_TypeError,
                   SStream() << "Expected double, got " << typeName(obj));

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

bool fromPythonSequence(PyObject *seq, std::string *vec, size_t len)
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
bool expectFromPythonSequenceT(PyObject* seq, T* vec, size_t len, const std::string& type)
{
    if (!PySequence_Check(seq))
    {
        setPythonError(PyExc_TypeError,
                       SStream() << "Expected sequence of "
                       << type << ", got " << typeName(seq));
        return false;
    }
    if (PySequence_Size(seq) != len)
    {
        setPythonError(PyExc_ValueError,
                       SStream() << "Expected sequence of length "
                        << len << ", got " << PySequence_Size(seq));
        return false;
    }
    for (Py_ssize_t i=0; i<len; ++i)
    {
        if (!expectFromPython(PySequence_GetItem(seq, i), &vec[i]))
            return false;
    }
    return true;
}

bool expectFromPythonSequence(PyObject *seq, std::string *vec, size_t len)
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
        setPythonError(PyExc_IndexError,
                  SStream() << "Index out of range, " << index << " >= " << len);
        return false;
    }
    return true;
}





void setPythonError(PyObject* exc, const std::string& txt)
{
    PyErr_SetObject(exc, toPython(txt));
}

std::string typeName(const PyObject *arg)
{
    if (!arg)
        return "NULL";
    auto s = std::string(arg->ob_type->tp_name);
    return s;
}

bool iterateSequence(PyObject* seq, std::function<bool(PyObject*item)> foo)
{
    if (!PySequence_Check(seq))
    {
        setPythonError(PyExc_TypeError,
                       SStream() << "expected sequence, got " << typeName(seq));
        return false;
    }
    Py_ssize_t size = PySequence_Size(seq);
    for (Py_ssize_t i = 0; i < size; ++i)
    {
        auto item = PySequence_GetItem(seq, i);
        if (!item)
        {
            setPythonError(PyExc_TypeError,
                       SStream() << "NULL object in sequence[" << i << "]");
            return false;
        }
        if (!foo(item))
            return false;
    }
    return true;
}


PyObject* removeArgumentTuple(PyObject* arg)
{
    if (PyTuple_Check(arg) && PyTuple_Size(arg) == 1)
        return PyTuple_GetItem(arg, 0);
    return arg;
}


void dumpObject(PyObject* arg, bool introspect)
{
    #define CPPY__PRINTPY(what__) \
        if (what__) \
        { \
            if (introspect) \
            { \
                auto s = PyObject_CallMethod(what__, "__str__", ""); \
                CPPY_PRINT(#what__ ": " << PyUnicode_AsUTF8(s)); \
            } \
            else CPPY_PRINT(#what__ ": " << what__); \
        } else CPPY_PRINT(#what__ ": NULL");

    #define CPPY__PRINT(what__) \
        CPPY_PRINT(#what__ ": " << what__);

    CPPY__PRINTPY(arg);
    if (arg)
    {
        CPPY__PRINT(arg->ob_refcnt);
        CPPY__PRINT(arg->ob_type);
        if (arg->ob_type)
        {
            CPPY__PRINT(arg->ob_type->tp_name);
            CPPY__PRINT(arg->ob_type->tp_basicsize);
            CPPY__PRINT(arg->ob_type->tp_itemsize);

            CPPY__PRINT(arg->ob_type->tp_dealloc);
            CPPY__PRINT(arg->ob_type->tp_print);
            CPPY__PRINT(arg->ob_type->tp_getattr);
            CPPY__PRINT(arg->ob_type->tp_setattr);
            CPPY__PRINT(arg->ob_type->tp_reserved);
            CPPY__PRINT(arg->ob_type->tp_repr);
            CPPY__PRINT(arg->ob_type->tp_as_number);
            CPPY__PRINT(arg->ob_type->tp_as_sequence);
            CPPY__PRINT(arg->ob_type->tp_as_mapping);
            CPPY__PRINT(arg->ob_type->tp_hash);
            CPPY__PRINT(arg->ob_type->tp_call);
            CPPY__PRINT(arg->ob_type->tp_str);
            CPPY__PRINT(arg->ob_type->tp_getattro);
            CPPY__PRINT(arg->ob_type->tp_setattro);
            CPPY__PRINT(arg->ob_type->tp_as_buffer);
            CPPY__PRINT(arg->ob_type->tp_flags);
            //CPPY__PRINT(arg->ob_type->tp_doc);
            CPPY__PRINT(arg->ob_type->tp_traverse);
            CPPY__PRINT(arg->ob_type->tp_clear);
            CPPY__PRINT(arg->ob_type->tp_richcompare);
            CPPY__PRINT(arg->ob_type->tp_weaklistoffset);
            CPPY__PRINT(arg->ob_type->tp_iter);
            CPPY__PRINT(arg->ob_type->tp_iternext);
            CPPY__PRINT(arg->ob_type->tp_methods);
            CPPY__PRINT(arg->ob_type->tp_members);
            CPPY__PRINT(arg->ob_type->tp_getset);
            CPPY__PRINT(arg->ob_type->tp_base);
            CPPY__PRINTPY(arg->ob_type->tp_dict);
            CPPY__PRINT(arg->ob_type->tp_descr_get);
            CPPY__PRINT(arg->ob_type->tp_descr_set);
            CPPY__PRINT(arg->ob_type->tp_dictoffset);
            CPPY__PRINT(arg->ob_type->tp_init);
            CPPY__PRINT(arg->ob_type->tp_alloc);
            CPPY__PRINT(arg->ob_type->tp_new);
            CPPY__PRINT(arg->ob_type->tp_free);
            CPPY__PRINT(arg->ob_type->tp_is_gc);

            CPPY__PRINTPY(arg->ob_type->tp_bases);
            CPPY__PRINTPY(arg->ob_type->tp_mro);
            CPPY__PRINTPY(arg->ob_type->tp_cache);
            CPPY__PRINTPY(arg->ob_type->tp_subclasses);
            CPPY__PRINTPY(arg->ob_type->tp_weaklist);
            CPPY__PRINT(arg->ob_type->tp_del);
            CPPY__PRINT(arg->ob_type->tp_version_tag);
            CPPY__PRINT(arg->ob_type->tp_finalize);

    #ifdef COUNT_ALLOCS
            CPPY__PRINT(arg->ob_type->tp_allocs);
            CPPY__PRINT(arg->ob_type->tp_frees);
            CPPY__PRINT(arg->ob_type->tp_maxalloc);
            CPPY__PRINT(arg->ob_type->tp_prev);
            CPPY__PRINT(arg->ob_type->tp_next);
    #endif
        }
    }
#undef CPPY__PRINT
#undef CPPY__PRINTPY
}

} // namespace PyUtils
