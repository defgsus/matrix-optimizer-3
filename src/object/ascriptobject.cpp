/** @file ascriptobject.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/1/2015</p>
*/

#include "ascriptobject.h"
#include "io/datastream.h"
#include "param/parameters.h"
#include "param/parametertext.h"
#include "script/angelscript.h"
#include "script/angelscript_object.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(AScriptObject)

class AScriptObject::Private
{
    public:
    Private(AScriptObject * o)
        : obj(o), isCompiled(false), engine(0), module(0), context(0), mainFunc(0), ok(false) { }

    ~Private()
    {
        if (engine)
            engine->Release();
    }

    void compile();
    void run();

    void messageCallback(const asSMessageInfo * msg);

    AScriptObject * obj;
    ParameterText * scriptText;
    bool isCompiled;

    asIScriptEngine * engine;
    asIScriptModule * module;
    asIScriptContext * context;
    asIScriptFunction * mainFunc;
    bool ok;
};


AScriptObject::AScriptObject(QObject *parent)
    : Object    (parent),
      p_        (new Private(this))
{
    setName("AngelScript");
}

AScriptObject::~AScriptObject()
{
    delete p_;
}

void AScriptObject::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("ascript", 1);
}

void AScriptObject::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("ascript", 1);
}

void AScriptObject::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("_ascript", tr("script"));
    initParameterGroupExpanded("_ascript");

        p_->scriptText = params()->createTextParameter("_ascript", tr("sourcecode"),
                                                    tr("The source code of the AngelScript program"),
                                                    TT_ANGELSCRIPT,
                                                    "// " + tr("Press F1 for help") + "\n"
                                                    "void main()\n{\n\t\n}\n",
                                                    true, false);

    params()->endParameterGroup();
}

void AScriptObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);

    if (p == p_->scriptText)
    {
        p_->compile();
        p_->run();
    }
}



void AScriptObject::Private::compile()
{
    ok = false;
    isCompiled = false;

    // get engine
    if (!engine)
    {
        engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
        if (!engine)
            MO_ERROR(tr("The script engine could not be created"));

        engine->SetMessageCallback(asMETHOD(Private, messageCallback), this, asCALL_THISCALL);

        // install namespace
        registerDefaultAngelScript(engine);
        registerAngelScript_object(engine, obj, true, true);
    }

    // get module
    if (!module)
        module = engine->GetModule(("_exec_" + obj->idName()).toUtf8().constData(), asGM_CREATE_IF_NOT_EXISTS);
    if (!module)
        MO_ERROR(tr("The script module could not be created"));

    // compile
    std::string script(scriptText->value().toUtf8().constData());
    module->AddScriptSection("script", script.c_str(), script.size());

    if (module->Build() < 0)
        MO_ERROR(tr("The script could not be compiled"));

    // get main func
    mainFunc = module->GetFunctionByDecl("void main()");
    if( mainFunc == 0 )
        MO_ERROR(tr("The script must have the function 'void main()'\n"));

    // get a context to execute the prog
    if (!context)
        context = engine->CreateContext();
    if (!context)
        MO_ERROR(tr("The script context could not be created"));

    ok = true;
    isCompiled = true;
}

void AScriptObject::Private::run()
{
    if (!ok)
        return;

    MO_ASSERT(context, "");

    // init context
    if (context->Prepare(mainFunc) < 0)
        MO_ERROR(tr("The script context could not be initialized"));

    int r = context->Execute();

    if( r == asEXECUTION_EXCEPTION )
        MO_ERROR("An exception occured in the script: " << context->GetExceptionString());

    if( r != asEXECUTION_FINISHED )
        MO_ERROR("The script ended prematurely");
}

void AScriptObject::Private::messageCallback(const asSMessageInfo * msg)
{
    MO_WARNING("angelscript('" << obj->name() << "'): " << msg->message);
}

void AScriptObject::runScript()
{
    if (!p_->ok || !p_->isCompiled)
        p_->compile();

    p_->run();
}

} // namespace MO
