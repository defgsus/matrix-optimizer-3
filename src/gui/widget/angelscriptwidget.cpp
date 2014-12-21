/** @file angelscriptwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <angelscript.h>


#include "angelscriptwidget.h"
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

    AngelScriptWidget * widget;

    asIScriptEngine * engine;
    asIScriptModule * module;

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


void AngelScriptWidget::Private::createObjects()
{
    engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    MO_ASSERT(engine, "");

    engine->SetMessageCallback(asMETHOD(Private, messageCallback), this, asCALL_THISCALL);

    module = engine->GetModule("module", asGM_ALWAYS_CREATE);
    MO_ASSERT(module, "");

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


bool AngelScriptWidget::Private::compile()
{
    const auto script = widget->scriptText().toStdString();

    module->BindAllImportedFunctions();

    module->AddScriptSection("script",
                             script.c_str(), script.size());

    bool ret = (module->Build() >= 0);

    if (ret)
    {
        syn->initForAngelScript(module);
        widget->setSyntaxHighlighter(syn);
    }

    return ret;
}


bool AngelScriptWidget::compile()
{
    return p_->compile();
}



} // namespace GUI
} // namespace MO


#endif // MO_DISABLE_ANGELSCRIPT
