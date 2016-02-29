/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON27

#include <python2.7/Python.h>

#include "python.h"
#include "python_funcs.h"
#include "python_geometry.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace PYTHON27 {

namespace
{
    const char* moDocString()
    {
        return "This module holds all classes and functions for interacting "
               "with the software.";
    }

}


void initPython()
{
    if (!Py_IsInitialized())
    {
        PyEval_InitThreads();
        Py_Initialize();
    }
}

void finalizePython()
{
    if (Py_IsInitialized())
        Py_Finalize();
}


struct PythonInterpreter::Private
{
    Private(PythonInterpreter* p)
        : p             (p)
        , threadState   (0)
        , module        (0)
        , curGeom       (0)
    { }

    PythonInterpreter* p;
    PyThreadState* threadState;
    PyObject* module;

    GEOM::Geometry* curGeom;
};

PythonInterpreter::PythonInterpreter()
    : p_        (new Private(this))
{
}

PythonInterpreter::~PythonInterpreter()
{
    clear();
    delete p_;
}

void PythonInterpreter::clear()
{
    if (p_->threadState)
    {
        Py_EndInterpreter(p_->threadState);
        p_->threadState = 0;
    }
}

void PythonInterpreter::setGeometry(GEOM::Geometry* geom) { p_->curGeom = geom; }
MO::GEOM::Geometry* PythonInterpreter::geometry() const { return p_->curGeom; }

void PythonInterpreter::execute(const QString& str)
{
    auto utf8 = str.toUtf8();
    const char * src = utf8.constData();
    execute(src);
}

void PythonInterpreter::execute(const char* utf8)
{
    initPython();

    if (!p_->threadState)
    {
        p_->threadState = Py_NewInterpreter();
        if (!p_->threadState)
            MO_ERROR("Could not create Python 2.7 interpreter");

        // init module and methods
        p_->module = Py_InitModule4(
                            "matrixoptimizer",
                            reinterpret_cast<PyMethodDef*>(pythonFuncs()),
                            const_cast<char*>(moDocString()),
                            NULL,//reinterpret_cast<PyObject*>(this),
                            PYTHON_API_VERSION
                    );
        if (p_->module == NULL)
        {
            clear();
            MO_ERROR("Could not initialize 'matrixoptimizer' module for Python 2.7");
        }

        initGeometry(p_->module);
    }

    // --- execute ---

    PyCompilerFlags flags;
    flags.cf_flags = PyCF_SOURCE_IS_UTF8;

    PyRun_SimpleStringFlags(utf8, &flags);

    std::cout.flush();
    std::cerr.flush();
}

} // namespace PYTHON27
} // namespace MO

#endif // #ifdef MO_ENABLE_PYTHON27

