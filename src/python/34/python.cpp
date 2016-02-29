/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include <python3.4/Python.h>

#include "python.h"
#include "python_funcs.h"
#include "python_geometry.h"
#include "python_output.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {

namespace
{
    const char* moDocString()
    {
        return "This module holds all classes and functions for interacting "
               "with the software.";
    }

    PyModuleDef* moModuleDef()
    {
        static PyModuleDef module =
        {
            PyModuleDef_HEAD_INIT,
            "matrixoptimizer",
            moDocString(),
            -1, /* m_size */
            reinterpret_cast<PyMethodDef*>(pythonFuncs()), /* m_methods */
            nullptr, /* m_reload */
            nullptr, /* m_traverse */
            nullptr, /* m_clear */
            nullptr, /* m_free */
        };
        return &module;
    }

    PyMODINIT_FUNC moCreateModule()
    {
        auto module = PyModule_Create(moModuleDef());
        if (!module)
            return nullptr;
        initGeometry(module);
        return module;
    }
}


void initPython()
{
    if (!Py_IsInitialized())
    {
        PyImport_AppendInittab("matrixoptimizer", moCreateModule);
        //PyEval_InitThreads();
        Py_Initialize();
    }
}

/** @todo Py_Finalize() crashes for Python 3.4
    when using Py_NewInstance() / Py_EndInstance() */
void finalizePython()
{
    //if (Py_IsInitialized())
    //    Py_Finalize();
}


struct PythonInterpreter::Private
{
    Private(PythonInterpreter* p)
        : p             (p)
        , threadState   (0)
        , module        (0)
        , curGeom       (0)
    { }

    void setup();

    PythonInterpreter* p;
    PyThreadState* threadState;
    PyObject* module;

    QString output, errorOutput;

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

const QString& PythonInterpreter::output() const { return p_->output; }
const QString& PythonInterpreter::errorOutput() const { return p_->errorOutput; }

void PythonInterpreter::clear()
{
    if (p_->threadState)
    {
        Py_EndInterpreter(p_->threadState);
        p_->threadState = 0;
    }
    p_->output.clear();
    p_->errorOutput.clear();
}

PythonInterpreter* PythonInterpreter::current()
{
    if (auto dict = PyThreadState_GetDict())
    {
        //MO_PRINT("Got thread state dict");
        auto caps = PyDict_GetItemString(dict,
                            "matrixoptimizer-interpreter");
        if (caps && PyCapsule_CheckExact(caps))
        {
            //MO_PRINT("got capsule");
            void* ctx = PyCapsule_GetPointer(caps, "interpreter");
            if (ctx)
            {
                //MO_PRINT("got interpreter");
                auto inter = reinterpret_cast<PythonInterpreter*>(ctx);
                return inter;
            }
        }
    }
    return nullptr;
}

void PythonInterpreter::setGeometry(GEOM::Geometry* geom) { p_->curGeom = geom; }
MO::GEOM::Geometry* PythonInterpreter::geometry() const { return p_->curGeom; }

void PythonInterpreter::Private::setup()
{
    initPython();

    if (!threadState)
    {
        try
        {
            threadState = Py_NewInterpreter();
            if (!threadState)
                MO_ERROR("Could not create Python 2.7 interpreter");

            // -- store interpreter instance in threadstate dict --

            auto dict = PyThreadState_GetDict();
            if (!dict)
                MO_ERROR("Can't access Python thread state");

            // wrap pointer to this class in PyCapsule
            auto caps = PyCapsule_New(p, "interpreter", NULL);
            if (!caps)
                MO_ERROR("Failed to init Python interpreter capsule");
            PyDict_SetItemString(dict, "matrixoptimizer-interpreter", caps);

            // redirect output
            for (int i=0; i<2; ++i)
            {
                auto outp = reinterpret_cast<PyObject*>(createOutputObject(p, i==0));
                if (!outp)
                    MO_ERROR("Failed to create output redirection object for Python");
                if (0 != PySys_SetObject(i==0? "stderr" : "stdout", outp))
                    MO_ERROR("Failed to install output redirection for Python");
            }
        }
        catch (...)
        {
            p->clear();
            throw;
        }
    }
}

void PythonInterpreter::execute(const QString& str)
{
    auto utf8 = str.toUtf8();
    const char * src = utf8.constData();
    execute(src);
}

void PythonInterpreter::execute(const char* utf8)
{
    p_->setup();

    // --- execute ---

    PyCompilerFlags flags;
    flags.cf_flags = PyCF_SOURCE_IS_UTF8;

    PyRun_SimpleStringFlags(utf8, &flags);
}

void PythonInterpreter::write(const char *utf8, bool error)
{
    if (error)
        p_->errorOutput += QString::fromUtf8(utf8);
    else
        p_->output += QString::fromUtf8(utf8);
}


} // namespace PYTHON34
} // namespace MO

#endif // #ifdef MO_ENABLE_PYTHON34

