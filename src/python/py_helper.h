/** @brief helper for python c-api

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>
*/

#ifndef PY_UTILS_H
#define PY_UTILS_H

#include <string>
#include <sstream>
#include <functional>

#include <python3.4/Python.h>
#include <python3.4/structmember.h>
#undef T_NONE
#undef T_OBJECT

#ifndef CPPY_PRINT
#   include <iostream>
#   define CPPY_PRINT(arg__) { std::cout << arg__ << std::endl; }
#endif

#ifndef CPPY_ERROR
#   define CPPY_ERROR(arg__) { CPPY_PRINT(arg__); exit(EXIT_FAILURE); }
#endif

#ifndef Py_RETURN_SELF
#   define Py_RETURN_SELF return Py_INCREF(self), reinterpret_cast<PyObject*>(self)
#endif

namespace PyUtils {

PyObject* toPython(const std::string&);
PyObject* toPython(const char*);
PyObject* toPython(double);
PyObject* toPython(bool);
PyObject* toPython(long);
PyObject* toPython(long unsigned);
PyObject* toPython(int);

template <class T0>
PyObject* toTuple(const T0& v0);
template <class T0, class T1>
PyObject* toTuple(const T0& v0, const T1& v1);
template <class T0, class T1, class T2>
PyObject* toTuple(const T0& v0, const T1& v1, const T2& v2);
template <class T0, class T1, class T2, class T3>
PyObject* toTuple(const T0& v0, const T1& v1, const T2& v2, const T3& v3);
template <class T0, class T1, class T2, class T3, class T4>
PyObject* toTuple(const T0& v0, const T1& v1, const T2& v2, const T3& v3, const T4& v4);

template <class T>
PyObject* toTuple(const T* vec, size_t num);
template <class T>
PyObject* toTuple(T* vec, size_t num) { return toTuple((const T*)vec, num); }

template <class T0>
PyObject* toList(const T0& v0);
template <class T0, class T1>
PyObject* toList(const T0& v0, const T1& v1);
template <class T0, class T1, class T2>
PyObject* toList(const T0& v0, const T1& v1, const T2& v2);
template <class T0, class T1, class T2, class T3>
PyObject* toList(const T0& v0, const T1& v1, const T2& v2, const T3& v3);
template <class T0, class T1, class T2, class T3, class T4>
PyObject* toList(const T0& v0, const T1& v1, const T2& v2, const T3& v3, const T4& v4);

template <class T>
PyObject* toList(const T* vec, size_t num);
template <class T>
PyObject* toList(T* vec, size_t num) { return toList((const T*)vec, num); }


bool fromPython(PyObject*, std::string*);
bool fromPython(PyObject*, long*);
bool fromPython(PyObject*, double*);

bool expectFromPython(PyObject*, std::string*);
bool expectFromPython(PyObject*, long*);
bool expectFromPython(PyObject*, double*);

// Writes 'len' entries from the python sequence into 'vec'
bool fromPythonSequence(PyObject* seq, std::string* vec, size_t len);
bool fromPythonSequence(PyObject* seq, long* vec, size_t len);
bool fromPythonSequence(PyObject* seq, double* vec, size_t len);

bool expectFromPythonSequence(PyObject* seq, std::string* vec, size_t len);
bool expectFromPythonSequence(PyObject* seq, long* vec, size_t len);
bool expectFromPythonSequence(PyObject* seq, double* vec, size_t len);

/** If @p arg is a tuple with one object, then return the object, otherwise arg */
PyObject* removeArgumentTuple(PyObject* arg);

void setPythonError(PyObject* exc, const std::string& txt);

std::string typeName(const PyObject* arg);

/** Iterates over every item in the PySequence. If seq is not sequencable,
    sets PyErr and returns false. If foo returns false, the iteration is stopped
    and false is returned. */
bool iterateSequence(PyObject* seq, std::function<bool(PyObject*item)> foo);

/** Verify that index < len, raise IndexError otherwise */
bool checkIndex(Py_ssize_t index, Py_ssize_t len);

/** print object internals */
void dumpObject(PyObject* arg, bool introspect);

/** A std::stringstream wrapper that converts to
    std::string or PyObject* automatically */
class SStream
{
    std::stringstream sstream_;
public:
    template <class T>
    SStream& operator << (const T& t) { sstream_ << t; return *this; }

    operator std::string() { return sstream_.str(); }
    operator PyObject*() { return toPython(sstream_.str()); }
};



// #################### template impl. #####################

template <class T>
PyObject* toTuple(const T& v0)
{
    auto o = PyTuple_New(1);
    PyTuple_SetItem(o, 0, toPython(v0));
    return o;
}

template <class T0, class T1>
PyObject* toTuple(const T0& v0, const T1& v1)
{
    auto o = PyTuple_New(2);
    PyTuple_SetItem(o, 0, toPython(v0));
    PyTuple_SetItem(o, 1, toPython(v1));
    return o;
}

template <class T0, class T1, class T2>
PyObject* toTuple(const T0& v0, const T1& v1, const T2& v2)
{
    auto o = PyTuple_New(3);
    PyTuple_SetItem(o, 0, toPython(v0));
    PyTuple_SetItem(o, 1, toPython(v1));
    PyTuple_SetItem(o, 2, toPython(v2));
    return o;
}

template <class T0, class T1, class T2, class T3>
PyObject* toTuple(const T0& v0, const T1& v1, const T2& v2, const T3& v3)
{
    auto o = PyTuple_New(4);
    PyTuple_SetItem(o, 0, toPython(v0));
    PyTuple_SetItem(o, 1, toPython(v1));
    PyTuple_SetItem(o, 2, toPython(v2));
    PyTuple_SetItem(o, 3, toPython(v3));
    return o;
}

template <class T0, class T1, class T2, class T3, class T4>
PyObject* toTuple(const T0& v0, const T1& v1, const T2& v2, const T3& v3, const T4& v4)
{
    auto o = PyTuple_New(5);
    PyTuple_SetItem(o, 0, toPython(v0));
    PyTuple_SetItem(o, 1, toPython(v1));
    PyTuple_SetItem(o, 2, toPython(v2));
    PyTuple_SetItem(o, 3, toPython(v3));
    PyTuple_SetItem(o, 4, toPython(v4));
    return o;
}



template <class T>
PyObject* toList(const T& v0)
{
    auto o = PyList_New(1);
    PyList_SetItem(o, 0, toPython(v0));
    return o;
}

template <class T0, class T1>
PyObject* toList(const T0& v0, const T1& v1)
{
    auto o = PyList_New(2);
    PyList_SetItem(o, 0, toPython(v0));
    PyList_SetItem(o, 1, toPython(v1));
    return o;
}

template <class T0, class T1, class T2>
PyObject* toList(const T0& v0, const T1& v1, const T2& v2)
{
    auto o = PyList_New(3);
    PyList_SetItem(o, 0, toPython(v0));
    PyList_SetItem(o, 1, toPython(v1));
    PyList_SetItem(o, 2, toPython(v2));
    return o;
}

template <class T0, class T1, class T2, class T3>
PyObject* toList(const T0& v0, const T1& v1, const T2& v2, const T3& v3)
{
    auto o = PyList_New(4);
    PyList_SetItem(o, 0, toPython(v0));
    PyList_SetItem(o, 1, toPython(v1));
    PyList_SetItem(o, 2, toPython(v2));
    PyList_SetItem(o, 3, toPython(v3));
    return o;
}

template <class T0, class T1, class T2, class T3, class T4>
PyObject* toList(const T0& v0, const T1& v1, const T2& v2, const T3& v3, const T4& v4)
{
    auto o = PyList_New(5);
    PyList_SetItem(o, 0, toPython(v0));
    PyList_SetItem(o, 1, toPython(v1));
    PyList_SetItem(o, 2, toPython(v2));
    PyList_SetItem(o, 3, toPython(v3));
    PyList_SetItem(o, 4, toPython(v4));
    return o;
}

template <class T>
PyObject* toTuple(const T* vec, size_t num)
{
    auto o = PyTuple_New(num);
    for (size_t i = 0; i < num; ++i)
        PyTuple_SetItem(o, i, toPython(*vec++));
    return o;
}

template <class T>
PyObject* toList(const T* vec, size_t num)
{
    auto o = PyList_New(num);
    for (size_t i = 0; i < num; ++i)
        PyList_SetItem(o, i, toPython(*vec++));
    return o;
}




} // namespace PyUtils


#endif // PY_UTILS_H

