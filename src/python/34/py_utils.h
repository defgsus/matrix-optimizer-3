/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/2/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PY_UTILS_H
#define MOSRC_PYTHON_34_PY_UTILS_H

#include <functional>

#include <python3.5/Python.h>
#include <python3.5/structmember.h>
#undef T_NONE
#undef T_OBJECT

#include <QString>

namespace MO {
namespace PYTHON34 {

#ifndef Py_RETURN_SELF
#   define Py_RETURN_SELF return Py_INCREF(self), reinterpret_cast<PyObject*>(self)
#endif

PyObject* toPython(const QString);
PyObject* toPython(long);
PyObject* toPython(double);
PyObject* toPython(bool);

bool fromPython(PyObject*, QString*);
bool fromPython(PyObject*, long*);
bool fromPython(PyObject*, double*);

bool expectFromPython(PyObject*, QString*);
bool expectFromPython(PyObject*, long*);
bool expectFromPython(PyObject*, double*);

// Writes 'len' entries from the python sequence into 'vec'
bool fromPythonSequence(PyObject* seq, QString* vec, size_t len);
bool fromPythonSequence(PyObject* seq, long* vec, size_t len);
bool fromPythonSequence(PyObject* seq, double* vec, size_t len);

bool expectFromPythonSequence(PyObject* seq, QString* vec, size_t len);
bool expectFromPythonSequence(PyObject* seq, long* vec, size_t len);
bool expectFromPythonSequence(PyObject* seq, double* vec, size_t len);

/** If @p arg is a tuple with one object, then return the object, otherwise arg */
PyObject* removeArgumentTuple(PyObject* arg);

void setPythonError(PyObject* exc, const QString& txt);

QString typeName(const PyObject* arg);

/** Iterates over every item in the PySequence. If seq is not sequencable,
    sets PyErr and returns false. If foo returns false, the iteration is stopped
    and false is returned. */
bool iterateSequence(PyObject* seq, std::function<bool(PyObject*item)> foo);

/** Verify that index < len, raise PyError otherwise */
bool checkIndex(Py_ssize_t index, Py_ssize_t len);


// ------- old stuff ------

/** Initializes the object from the type struct.
    @throws Exception any error */
void initObjectType(PyObject* module, PyTypeObject* type, const char* name);

void dumpObject(PyObject* arg, bool introspect);

PyObject* fromString(const QString&);
QString toString(PyObject* unicode);

PyObject* fromInt(int v);
PyObject* fromLong(long v);
PyObject* fromDouble(double v);
bool toDouble(PyObject* arg, double* v);

void PyErr_Set(PyObject* exc, const QString& txt);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PY_UTILS_H

#endif // MO_ENABLE_PYTHON34
