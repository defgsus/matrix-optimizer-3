/** @file geometryangelscript.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <angelscript.h>

#include <QString>

#include "geometryangelscript.h"
#include "geometry.h"
#include "io/error.h"

namespace MO {
namespace GEOM {


class GeometryAngelScript::Private
{
public:

    Private(Geometry * g)
        : g     (g),
          engine(0)
    { };

    void createEngine();
    void messageCallback(const asSMessageInfo *msg);

    void line(float x, float y, float z, float x1, float y1, float z1)
    {
        Geometry::IndexType
                    v1 = g->addVertex(x,y,z),
                    v2 = g->addVertex(x1,y1,z1);
        g->addLine(v1, v2);
    }

    Geometry * g;
    asIScriptEngine * engine;
    QString errors;
};


GeometryAngelScript::GeometryAngelScript(Geometry * g)
    : p_    (new Private(g))
{
}

GeometryAngelScript::~GeometryAngelScript()
{
    if (p_->engine)
        p_->engine->Release();
    delete p_;
}

asIScriptEngine * GeometryAngelScript::scriptEngine()
{
    if (!p_->engine)
        p_->createEngine();
    return p_->engine;
}

asIScriptEngine * GeometryAngelScript::createNullEngine()
{
    GeometryAngelScript g(0);
    auto engine = g.scriptEngine();
    g.p_->engine = 0;
    return engine;
}


void GeometryAngelScript::Private::createEngine()
{
    int r;

    engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    MO_ASSERT(engine, "");

    r = engine->RegisterGlobalFunction("void line(float x, float y, float z, float x2, float y2, float z2)",
                                        asMETHOD(Private, line),
                                        asCALL_THISCALL_ASGLOBAL, this);
    MO_ASSERT(r >= 0, "registerGlobalFunction");

}

void GeometryAngelScript::Private::messageCallback(const asSMessageInfo *msg)
{
    errors += QString("\n%1:%2 %3").arg(msg->row).arg(msg->col).arg(msg->message);
}

void GeometryAngelScript::execute(const QString &qscript)
{
    class Deleter
    {
    public:
        Deleter(asIScriptEngine * e, asIScriptModule*m) : e(e), m(m) { }
        ~Deleter() { e->DiscardModule(m->GetName()); }
        asIScriptEngine * e;
        asIScriptModule * m;
    };


    // --- create a module ---

    auto module = scriptEngine()->GetModule("_geom_module", asGM_ALWAYS_CREATE);
    if (!module)
        MO_ERROR("Could not create script module");

    Deleter deleter_(module->GetEngine(), module);

    QByteArray script = qscript.toUtf8();
    module->AddScriptSection("script", script.data(), script.size());

    p_->errors.clear();
    p_->engine->SetMessageCallback(asMETHOD(Private, messageCallback), this, asCALL_THISCALL);

    // compile
    int r = module->Build();

    if (r < 0)
        MO_ERROR(QObject::tr("Error parsing script") + ":" + p_->errors);

    // --- get main function ---

    asIScriptFunction *func = module->GetFunctionByDecl("void main()");
    if( func == 0 )
        MO_ERROR("The script must have the function 'void main()'\n");

    // Create our context, prepare it, and then execute
    asIScriptContext *ctx = scriptEngine()->CreateContext();
    if (!ctx)
        MO_ERROR("Could not create script context");

    ctx->Prepare(func);
    r = ctx->Execute();

    if( r == asEXECUTION_EXCEPTION )
        MO_ERROR("An exception occured in the script: " << ctx->GetExceptionString());

    if( r != asEXECUTION_FINISHED )
        MO_ERROR("The script ended prematurely");
}






} // namespace GEOM
} // namespace MO




#endif // MO_DISABLE_ANGELSCRIPT
