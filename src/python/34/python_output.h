/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PYTHON_OUTPUT_H
#define MOSRC_PYTHON_34_PYTHON_OUTPUT_H

namespace MO {
namespace PYTHON34 {

    class PythonInterpreter;

    /** Creates a sys.stdout/stderr compatible object.
        Its output is forwarded to PythonInterpreter::write().
        @returns PyObject* */
    void* createOutputObject(PythonInterpreter*, bool error);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PYTHON_OUTPUT_H

#endif // MO_ENABLE_PYTHON34
