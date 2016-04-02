/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/15/2016</p>
*/

#include "pythonobject.h"
#include "io/datastream.h"
#include "param/parameters.h"
#include "param/parametertext.h"
#include "python/34/python.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(PythonObject)

class PythonObject::Private
{
    public:
    Private(PythonObject * o)
        : obj(o)
    { }

    ~Private()
    {

    }

    void run();

    PythonObject * obj;
    ParameterText * scriptText;
};


PythonObject::PythonObject()
    : Object    (),
      p_        (new Private(this))
{
    setName("Python");
}

PythonObject::~PythonObject()
{
    delete p_;
}

void PythonObject::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("pyobj", 1);
}

void PythonObject::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("pyobj", 1);
}

void PythonObject::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("_python", tr("script"));
    initParameterGroupExpanded("_python");

        p_->scriptText = params()->createTextParameter(
                    "python_script", tr("python script"),
                    tr("The source code of the python script to execute"),
                    TT_PYTHON34,
                    "// " + tr("Press F1 for help") + "\n"
                    "import matrixoptimizer as mo\n",
                    true, false);

    params()->endParameterGroup();
}

void PythonObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);

    if (p == p_->scriptText)
    {
        p_->run();
    }
}


void PythonObject::Private::run()
{
#ifdef MO_ENABLE_PYTHON34
    PYTHON34::PythonInterpreter interp;
    interp.setObject(obj);
    interp.execute(scriptText->baseValue());
#else
    obj->setErrorMessage(QObject::tr("Python support is not enabled in this binary"));
#endif
}

void PythonObject::runScript()
{
    p_->run();
}

} // namespace MO
