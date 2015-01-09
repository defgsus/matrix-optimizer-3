/** @file test_angelscript.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.01.2015</p>
*/

#include <QByteArray>

#include "test_angelscript.h"
#include "script/angelscript.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {


class TestAngelScript::Private
{
public:

    Private(TestAngelScript * p)
        : p         (p),
          engine    (0),
          context   (0)
    { }

    ~Private()
    {
        if (engine)
            engine->Release();
    }

    int createEngine();

    int run();
    int test_vector();

    int compile(const QString& script);

    TestAngelScript * p;
    asIScriptEngine * engine;
    asIScriptContext * context;
    asIScriptModule * module;
};




TestAngelScript::TestAngelScript()
    : p_    (new Private(this))
{

}

TestAngelScript::~TestAngelScript()
{
    delete p_;
}

int TestAngelScript::run()
{
    return p_->run();
}

int TestAngelScript::Private::createEngine()
{
    engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    if (!engine)
    {
        MO_WARNING("Could not create AngelScript engine");
        return 1;
    }

    module = engine->GetModule("_test_module", asGM_ALWAYS_CREATE);
    if (!module)
    {
        MO_WARNING("Could not create AngelScript module");
        return 1;
    }

    return 0;
}

int TestAngelScript::Private::compile(const QString &scriptt)
{
    QByteArray script = scriptt.toUtf8();
    int r = module->AddScriptSection("script", script.data(), script.size());
    if (r < 0)
    {
        MO_WARNING("Could not add AngelScript section ('" << scriptt << "')");
        return 1;
    }

    return 0;
}

int TestAngelScript::Private::run()
{
    int e = 0;
    e = createEngine();
    if (e)
        return e;

    e += test_vector();

    return e;
}

int TestAngelScript::Private::test_vector()
{
    return 0;
}



} // namespace MO
