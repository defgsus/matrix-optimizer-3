/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#include "py_mo_helper.h"
#include "math/ArithmeticArray.h"

namespace PyUtils {

PyObject* toPython(const QString& s)
{
    return PyUnicode_FromString(s.toUtf8().constData());
}

PyObject* toPython(const MO::MATH::ArithmeticArray<double>& a)
{
    auto list = PyList_New(a.numDimensions());
    for (size_t i=0; i<a.numDimensions(); ++i)
    {
        auto v = toPython(a[i]);
        if (!v)
        {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SET_ITEM(list, i, v);
    }
    return list;
}

/*void setPythonError(PyObject* exc, const QString& txt)
{
    PyErr_SetObject(exc, toPython(txt));
}*/


} // namespace PyUtils
