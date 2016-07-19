/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include <python3.4/Python.h>

#include <atomic>

#include <QSet>

#include "python.h"
#include "python_funcs.h"
#include "python_object.h"
#include "python_vector.h"
#include "python_geometry.h"
#include "python_timeline.h"
#include "python_output.h"
#include "tool/stringmanip.h"
#include "io/ApplicationTime.h"
#include "io/error.h"
#include "io/log.h"

#if 0
#   define MO_PY_DEBUG(arg__) \
        MO_PRINT("PythonInterpreter(): " << arg__);
#else
#   define MO_PY_DEBUG(unused__)
#endif

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

        PyObject* submod = vecCreateModule();
        if (!submod)
            return NULL;
        Py_INCREF(submod);
        PyModule_AddObject(module, "vecfunc", submod);

        // add the classes
        MO_PY_DEBUG("init object"); initObject(module);
        MO_PY_DEBUG("init geometry"); initGeometry(module);
        MO_PY_DEBUG("init vector"); initVector(module);
        MO_PY_DEBUG("init timeline"); initTimeline(module);

        return module;
    }
}


void initPython()
{
    static bool is_init = false;
    if (!is_init)//Py_IsInitialized())
    {
        is_init = true;

        PyImport_AppendInittab("matrixoptimizer", moCreateModule);
        PyImport_AppendInittab("matrixoptimizer.vecfunc", vecCreateModule);

        Py_Initialize();

        PyEval_InitThreads();
        PyEval_SaveThread();
    }
}

/** @todo Py_Finalize() crashes - prob some refcount */
void finalizePython()
{
    //if (Py_IsInitialized())
    //    Py_Finalize();
}

void runConsole(int argc, char **args)
{
    initPython();

    std::vector<std::wstring> swargs;
    wchar_t* wargs[argc];
    for (int i=0; i<argc; ++i)
    {
        std::wstring str;
        str.resize(4096);
        swprintf(&str[0], 4095, L"%hs", args[i]);
        swargs.push_back(str);
        wargs[i] = &swargs.back()[0];
    }

    Py_Main(argc, wargs);
}


struct PythonInterpreter::Private
{
    Private(PythonInterpreter* p)
        : p             (p)
        , main_module   (0)
        , byteCode      (0)
        , curObject     (0)
        , curGeom       (0)
    { }

    void setup();
    void backup_main();
    void restore_main();
    void destroy();
    void compile(const char* utf8);
    PyObject* create_namespace(const char* filename);

    /** high-level */
    void execute(const char* utf8);

    PythonInterpreter* p;

    PyObject* main_module,
            * byteCode;

    QString output, errorOutput;

    Object* curObject;
    GEOM::Geometry* curGeom;
};

namespace { std::atomic_long instanceCount_(0); }


PythonInterpreter::PythonInterpreter()
    : p_        (new Private(this))
{
    MO_PY_DEBUG("PythonInterpreter()");
}

PythonInterpreter::~PythonInterpreter()
{
    MO_PY_DEBUG("~PythonInterpreter()");

    p_->destroy();
    //clear();
    delete p_;

    MO_PY_DEBUG("~PythonInterpreter() ended");
}

const QString& PythonInterpreter::output() const { return p_->output; }
const QString& PythonInterpreter::errorOutput() const { return p_->errorOutput; }
long PythonInterpreter::instanceCount() { return instanceCount_; }

void PythonInterpreter::setGeometry(GEOM::Geometry* geom) { p_->curGeom = geom; }
GEOM::Geometry* PythonInterpreter::getGeometry() const { return p_->curGeom; }

void PythonInterpreter::setObject(Object*o) { p_->curObject = o; }
Object* PythonInterpreter::getObject() const { return p_->curObject; }


void PythonInterpreter::execute(const QString& str)
{
    auto utf8 = str.toUtf8();
    const char * src = utf8.constData();
    p_->execute(src);
}

void PythonInterpreter::execute(const char* utf8)
{
    p_->execute(utf8);
}

/*
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
*/

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

void PythonInterpreter::Private::backup_main()
{
    MO_ASSERT(!main_module, "duplicate backup");

    MO_PY_DEBUG("backup __main__");
    PyInterpreterState *interp = PyThreadState_GET()->interp;
    main_module = PyDict_GetItemString(interp->modules, "__main__");
    Py_XINCREF(main_module);
}

void PythonInterpreter::Private::restore_main()
{
    MO_ASSERT(main_module, "restore without backup");

    MO_PY_DEBUG("restore __main__");
    PyInterpreterState *interp = PyThreadState_GET()->interp;
    PyDict_SetItemString(interp->modules, "__main__", main_module);
    Py_XDECREF(main_module);
    main_module = nullptr;
}

void PythonInterpreter::Private::destroy()
{
    MO_PY_DEBUG("destroy()");

    Py_XDECREF(byteCode);
    byteCode = nullptr;
    if (main_module)
        restore_main();
}

PyObject* PythonInterpreter::Private::create_namespace(const char *filename)
{
    MO_PY_DEBUG("create_namespace(" << filename << ")");

    PyInterpreterState *interp = PyThreadState_GET()->interp;

    // create fresh __main__
    PyObject *mod_main = PyModule_New("__main__");
    PyDict_SetItemString(interp->modules, "__main__", mod_main);
    Py_DECREF(mod_main); /* sys.modules owns now */
    PyModule_AddStringConstant(mod_main, "__name__", "__main__");
    if (filename)
    {
        /* __file__ mainly for nice UI'ness
         * note: this wont map to a real file when executing text-blocks and buttons. */
//		PyModule_AddObject(mod_main, "__file__", PyC_UnicodeFromByte(filename));
    }
    PyModule_AddObject(mod_main, "__builtins__", interp->builtins);
    Py_INCREF(interp->builtins); /* AddObject steals a reference */

    auto dict = PyModule_GetDict(mod_main);

    return dict;
}

void PythonInterpreter::Private::compile(const char *utf8)
{
    Py_XDECREF(byteCode);
    byteCode = 0;

    MO_PY_DEBUG("compiling");

    PyObject* fn_dummy = Py_BuildValue("s", "matrixoptimizer");
    if (!fn_dummy)
        MO_ERROR("failed to initialize compiling python");
    PyCompilerFlags flags;
    flags.cf_flags = PyCF_SOURCE_IS_UTF8;
    byteCode = Py_CompileStringObject(utf8, fn_dummy, Py_file_input, &flags, -1);
    Py_DECREF(fn_dummy);

    if (PyErr_Occurred() || !byteCode)
    {
        MO_PY_DEBUG("compile error");
        /** XXX @todo catch compile errors */
        PyErr_Print();
        PyErr_Clear();
        Py_XDECREF(byteCode);
        byteCode = 0;
    }

    if (!byteCode)
        MO_ERROR("failed to compile python");
}


void PythonInterpreter::Private::execute(const char* utf8)
{
    initPython();

    MO_PY_DEBUG("get GIL");
    PyGILState_STATE gil_state = PyGILState_Ensure();

    try
    {
        backup_main();

        // redirect output
        MO_PY_DEBUG("redirect stdout/err");
        for (int i=0; i<2; ++i)
        {
            auto outp = reinterpret_cast<PyObject*>(createOutputObject(p, i==0));
            if (!outp)
                MO_ERROR("Failed to create output redirection object for Python");
            if (0 != PySys_SetObject(i==0? "stderr" : "stdout", outp))
                MO_ERROR("Failed to install output redirection for Python");
        }

        PyObject* dict = create_namespace("sub-script");

        compile(utf8);

        // -- store interpreter instance in threadstate dict --

        {
            MO_PY_DEBUG("attach interpreter class");

            auto dict = PyThreadState_GetDict();
            if (!dict)
                MO_ERROR("Can't access Python thread state");

            // wrap pointer to this class in PyCapsule
            auto caps = PyCapsule_New(p, "interpreter", NULL);
            if (!caps)
                MO_ERROR("Failed to init Python interpreter capsule");
            PyDict_SetItemString(dict, "matrixoptimizer-interpreter", caps);
        }

        // execute
        MO_PY_DEBUG("eval");
        ++instanceCount_;
        Py_INCREF(byteCode);
        PyEval_EvalCode(byteCode, dict, dict);

        if (PyErr_Occurred())
        {
            MO_PY_DEBUG("runtime error");
            PyErr_Print();
            PyErr_Clear();
            MO_ERROR("python runtime error");
        }

        restore_main();

        MO_PY_DEBUG("release GIL");
        PyGILState_Release(gil_state);
    }
    catch (...)
    {
        MO_PY_DEBUG("shutdown from exception");
        destroy();
        MO_PY_DEBUG("release GIL");
        PyGILState_Release(gil_state);
        throw;
    }
}



void PythonInterpreter::write(const char *utf8, bool error)
{
    //MO_PY_DEBUG("write(" << utf8 << ")");

    /** @todo segfault on python geometry script when run
        at program start from autoloaded scene and there is
        an output channeled to PythonInterpreter::write().
        Current workaround is by not printing during the first five seconds. */
    if (applicationTime() < 5.)
        return;

    if (error)
        p_->errorOutput += QString::fromUtf8(utf8);
    else
        p_->output += QString::fromUtf8(utf8);
}



QString PythonInterpreter::getHelpHtmlString()
{
    static QString helpStr;
    if (!helpStr.isEmpty())
        return helpStr;

    PythonInterpreter interp;
    interp.execute("import matrixoptimizer as mo\nhelp(mo)");
    QString help = interp.output();

    QSet<QString> anchors;

    auto lines = help.split("\n");
    help.clear();
    for (QString& line : lines)
    {
        // function
        if (line.contains("(...)"))
        {
            int idx = line.indexOf("|");
            if (idx > 0)
            {
                int idx2 = line.indexOf("(...)");
                QString funcName = line.mid(idx+1, idx2-idx-1).simplified();
                if (!anchors.contains(funcName))
                {
                    anchors.insert(funcName);
                    line = QString("</pre><a name=\"%1\"></a><pre>\n%2")
                            .arg(funcName).arg(line);
                }
            }
        }
        // class
        else if (line.simplified().startsWith("class"))
        {
            int idx = line.indexOf("class");
            int idx2 = line.indexOf("(");
            if (idx2 > idx)
            {
                QString className = line.mid(idx+5, idx2-idx-5).simplified();
                if (!anchors.contains(className))
                {
                    anchors.insert(className);
                    line = QString("</pre><a name=\"%1\"></a><pre>\n%2")
                            .arg(className).arg(line);
                };
            }
        }

        help.append(line + "\n");
    }

    return helpStr = "<pre>" + help + "</pre>";
}


} // namespace PYTHON34
} // namespace MO

#endif // #ifdef MO_ENABLE_PYTHON34

