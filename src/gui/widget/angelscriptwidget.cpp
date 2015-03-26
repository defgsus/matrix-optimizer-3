/** @file angelscriptwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include "angelscriptwidget.h"
#include "script/angelscript.h"
#include "tool/syntaxhighlighter.h"
#include "io/error.h"
#include "io/log.h"


namespace MO {
namespace GUI {


class AngelScriptWidget::Private
{
public:
    Private(AngelScriptWidget * widget)
        : widget    (widget),
          engine    (0),
          syn       (0)
    {

    }

    ~Private()
    {
        if (engine)
            engine->Release();
        //delete engine;
    }

    void createObjects();

    bool compile();

    void messageCallback(const asSMessageInfo *msg);

    void execute();

    AngelScriptWidget * widget;

    asIScriptEngine * engine;
    asIScriptModule * module;
    asIScriptContext * context;

    SyntaxHighlighter * syn;
};



AngelScriptWidget::AngelScriptWidget(QWidget *parent)
    : AbstractScriptWidget  (parent),
      p_                    (new Private(this))
{
    p_->createObjects();
}

AngelScriptWidget::~AngelScriptWidget()
{
    delete p_;
}

asIScriptEngine * AngelScriptWidget::scriptEngine() const
{
    return p_->engine;
}

asIScriptModule * AngelScriptWidget::scriptModule() const
{
    return p_->module;
}

void AngelScriptWidget::setScriptEngine(asIScriptEngine *eng) const
{
    MO_ASSERT(eng, "NULL engine");

    if (p_->engine)
        p_->engine->Release();

    p_->engine = eng;

    p_->createObjects();
}

void AngelScriptWidget::Private::createObjects()
{
    if (!engine)
        engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    MO_ASSERT(engine, "");

    engine->SetMessageCallback(asMETHOD(Private, messageCallback), this, asCALL_THISCALL);

    module = engine->GetModule("_editor_module", asGM_ALWAYS_CREATE);
    MO_ASSERT(module, "");

    // get a context to execute the prog
    context = engine->CreateContext();
    if (!context)
        MO_ERROR("Could not create script context");

    if (!syn)
        syn = new SyntaxHighlighter(widget);
    syn->initForAngelScript(module);

    widget->setSyntaxHighlighter(syn);
}

void AngelScriptWidget::Private::messageCallback(const asSMessageInfo *msg)
{
    if (msg->type == asMSGTYPE_ERROR)
        widget->addCompileMessage(msg->row-1, M_ERROR, msg->message);
    if (msg->type == asMSGTYPE_WARNING)
        widget->addCompileMessage(msg->row-1, M_WARNING, msg->message);
    if (msg->type == asMSGTYPE_INFORMATION)
        widget->addCompileMessage(msg->row-1, M_INFO, msg->message);
    /*
    MO_DEBUG("angelscript: " << msg->row << ":" << msg->col
             << " " << msg->message);
    */
}

QString AngelScriptWidget::getHelpUrl(const QString& token) const
{
    QString url = "angelscript.html#" + token.toHtmlEscaped();
    return url;
}

bool AngelScriptWidget::Private::compile()
{
    const auto script = widget->scriptText().toStdString();

    //module->BindAllImportedFunctions();

    module->AddScriptSection("script",
                             script.c_str(), script.size());

    bool ret = (module->Build() >= 0);

    if (ret)
        widget->updateSyntaxHighlighter();

    return ret;
}

void AngelScriptWidget::updateSyntaxHighlighter()
{
    p_->syn->initForAngelScript(p_->module);
    setSyntaxHighlighter(p_->syn);
}

bool AngelScriptWidget::compile()
{
    return p_->compile();
}

void AngelScriptWidget::executeScript()
{
    p_->execute();
}

void AngelScriptWidget::Private::execute()
{
    // --- create a module ---

    auto module = widget->scriptEngine()->GetModule("_test_module", asGM_ALWAYS_CREATE);
    if (!module)
        MO_ERROR("Could not create script module");

//    AngelScriptAutoPtr deleter_(module->GetEngine(), module);

    QByteArray script = widget->scriptText().toUtf8();
    module->AddScriptSection("script", script.data(), script.size());

    //p_->errors.clear();
    //p_->engine->SetMessageCallback(asMETHOD(Private, messageCallback), this, asCALL_THISCALL);

    // compile
    int r = module->Build();

    if (r < 0)
        MO_ERROR(QObject::tr("Error parsing script")/* + ":" + p_->errors*/);

    // --- get main function ---

    asIScriptFunction *func = module->GetFunctionByDecl("void main()");
    if( func == 0 )
        MO_ERROR("The script must have the function 'void main()'\n");

//    AngelScriptAutoPtr deleter2_(ctx);

    context->Unprepare();
    context->Prepare(func);
    r = context->Execute();

    if( r == asEXECUTION_EXCEPTION )
        MO_ERROR("An exception occured in the script: " << context->GetExceptionString());

    if( r != asEXECUTION_FINISHED )
        MO_ERROR("The script ended prematurely");
}

} // namespace GUI
} // namespace MO


#endif // MO_DISABLE_ANGELSCRIPT
