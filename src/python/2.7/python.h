/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON27

#ifndef MOSRC_PYTHON_27_PYTHON_H
#define MOSRC_PYTHON_27_PYTHON_H

namespace MO {

    void initPython27();
    void finalizePython27();

    void executePython27(const char* source);

    /** Returns the 'matrixoptimizer' module. */
    void* getPython27Module();

} // namespace MO

#endif // MOSRC_PYTHON_27_PYTHON_H

#endif // #ifdef MO_ENABLE_PYTHON27
