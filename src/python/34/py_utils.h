/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/2/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PY_UTILS_H
#define MOSRC_PYTHON_34_PY_UTILS_H

#include <python3.4/Python.h>
#include <python3.4/structmember.h>
#undef T_NONE
#undef T_OBJECT

#include <QString>

namespace MO {
namespace PYTHON34 {


/** Initializes the object from the type struct.
    @throws Exception any error */
void initObjectType(PyObject* module, PyTypeObject* type, const char* name);

void dumpObject(PyObject* arg, bool introspect);

PyObject* fromString(const QString&);
QString typeName(PyObject* arg);

PyObject* fromInt(int v);
PyObject* fromLong(long v);
PyObject* fromDouble(double v);
bool toDouble(PyObject* arg, double* v);

void PyErr_Set(PyObject* exc, const QString& txt);

/** Iterates over every item in the PySequence. If seq is not sequencable,
    sets PyErr and returns false. If foo returns false, the iteration is stopped
    and false is returned. */
bool iterateSequence(PyObject* seq, std::function<bool(PyObject*item)> foo);

/** If @p arg is a tuple with one object, then return the object */
PyObject* removeArgumentTuple(PyObject* arg);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PY_UTILS_H

#endif // MO_ENABLE_PYTHON34
