/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/3/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PYTHON_TIMELINE_H
#define MOSRC_PYTHON_34_PYTHON_TIMELINE_H

#include "py_utils.h"

namespace MO {
namespace MATH { class TimelineNd; template <typename T> class ArithmeticArray; }
namespace PYTHON34 {

    /** Adds the Timeline objects to the module. */
    void initTimeline(PyObject* module);

    bool isTimeline(PyObject* obj);

    PyObject* buildTimeline(MATH::TimelineNd*);
    PyObject* buildList(const MATH::ArithmeticArray<double>&);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PYTHON_TIMELINE_H

#endif // MO_ENABLE_PYTHON34
