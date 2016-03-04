/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/3/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34_

#ifndef MOSRC_PYTHON_34_PYTHON_TIMELINE_H
#define MOSRC_PYTHON_34_PYTHON_TIMELINE_H

namespace MO {
namespace MATH { class Timeline1d; }
namespace PYTHON34 {

    /** Adds the Timeline objects to the module.
        @p module is PyObject* */
    void initTimeline(void* module);

    bool isTimeline(void* pyObject);

    void* buildTimeline(MATH::Timeline1d*);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PYTHON_TIMELINE_H

#endif // MO_ENABLE_PYTHON34
