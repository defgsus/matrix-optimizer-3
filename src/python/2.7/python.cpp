/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON27

#ifndef MOSRC_PYTHON_27_PYTHON_H
#define MOSRC_PYTHON_27_PYTHON_H

#include <python2.7/Python.h>

#include "python.h"
#include "geometrymodule.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

namespace { static PyObject* moModule = 0; }


void initPython27()
{
    if (!Py_IsInitialized())
    {
        Py_Initialize();
        if (!moModule)
        {
            // init module and methods
            moModule = Py_InitModule("matrixoptimizer", NULL);
            if (moModule == NULL)
                MO_ERROR("Could not initialize 'matrixoptimizer' module for Python 2.7");
        }

        python27_initGeometryModule();
    }
}

void finalizePython27()
{
    if (Py_IsInitialized())
        Py_Finalize();
}

void* getPython27Module()
{

    if (!Py_IsInitialized() || !moModule)
        initPython27();
    return moModule;
}

void executePython27(const char* src)
{
    initPython27();

    PyCompilerFlags flags;
    flags.cf_flags = PyCF_SOURCE_IS_UTF8;

    //MO_PRINT("Running python2.7\n" << src);
    PyRun_SimpleStringFlags(src, &flags);

    std::cout.flush();
    std::cerr.flush();
}


} // namespace MO

#endif // MOSRC_PYTHON_27_PYTHON_H

#endif // #ifdef MO_ENABLE_PYTHON27

