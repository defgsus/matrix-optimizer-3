/** @file angelscript_network.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.01.2015</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>
#include <cstring> // strstr
#include <sstream>
#include <memory>

#include "angelscript_network.h"
#include "angelscript.h"
#include "network/udpconnection.h"
#include "io/log.h"
#include "io/error.h"

namespace MO {

//------------------------------
// AngelScript functions wrapper
//------------------------------


class UdpConnectionAS
{
public:

    struct Callback
    {
        Callback(asIScriptFunction * f = 0)
            : function  (f),
              context   (0)
        {
            MO_DEBUG("UdpCallback::UdpCallback(" << f << ")");

            if (f)
            {
                context = f->GetEngine()->CreateContext();
                function->AddRef();
            }
        }
        ~Callback()
        {
            MO_DEBUG("UdpCallback::~UdpCallback(" << function << ")");

            if (function)
                function->Release();
            if (context)
                context->Release();
        }

        asIScriptFunction * function;
        asIScriptContext * context;
    };

    /** Create a non-owning wrapper */
    UdpConnectionAS(UdpConnection * con)
        : con       (con),
          ref       (1)
    {
        con->addRef();
        MO_ASSERT(con, "Can't create wrapper for NULL Connection");
    }

    UdpConnectionAS() : con(new UdpConnection), ref(1)
    {
    }

    ~UdpConnectionAS()
    {
        callbacks.clear();
        con->releaseRef();
    }

    static UdpConnectionAS * factoryNew() { return new UdpConnectionAS(); }
    static UdpConnectionAS * factory(UdpConnection * con) { return new UdpConnectionAS(con); }

    void addRef() { ++ref; }
    void releaseRef() { if (--ref == 0) delete this; }

    void connectListen_();

    // --------------- interface -----------------


    // ------ getter ------

    StringAS toString() const
    {
        std::stringstream s;
        s << "Udp(" << con->address().toString() << ":" << con->port() << ")";
        return s.str();
    }

    bool isOpen() const { return con->isOpen(); }

    // ------ setter ------

    bool open(const StringAS& addr, uint16_t port) { return con->open(QHostAddress(MO::toString(addr)), port); }
    void close() { con->close(); }

    void writeString(const StringAS& s) { con->sendDatagram(s.c_str(), s.size()); }

    // ----- callbacks -----

    void removeStringCallback(asIScriptFunction * f) { removeCallback_(f); }

    void addStringCallback(asIScriptFunction * f) { addCallback_(f); }

    // --- helper ---

    void addCallback_(asIScriptFunction * f)
    {
        // no duplicates
        for (int i=0; i<callbacks.size(); )
            if (callbacks[i].get()->function == f)
                return;

        auto cb = new Callback(f);
        callbacks.append(std::shared_ptr<Callback>(cb));
    }

    void removeCallback_(asIScriptFunction * f)
    {
        for (int i=0; i<callbacks.size(); )
        {
            if (callbacks[i].get()->function == f)
                callbacks.removeAt(i);
            else
                ++i;
        }
    }



private:

    UdpConnection * con;
    int ref;
    QVector<std::shared_ptr<Callback>> callbacks;

};

void UdpConnectionAS::connectListen_()
{
    UdpConnection::connect(con, &UdpConnection::dataReady, [=]()
    {
        for (auto & i : callbacks)
        {
            Callback * cb = i.get();

            cb->context->Prepare(cb->function);

            // construct data type
            StringAS s = con->readData().constData();
            cb->context->SetArgAddress(0, &s);

            // run
            int r = cb->context->Execute();
            if( r == asEXECUTION_EXCEPTION )
                MO_WARNING("An exception occured in the udp callback function: " << cb->context->GetExceptionString());
            if( r != asEXECUTION_FINISHED )
                MO_WARNING("Execution of udp callback angelscript function failed");

        }
    });
}


namespace {



//--------------------------------
// Registration
//-------------------------------------

namespace native {

#define MO__REG_METHOD(decl__, func__) \
    r = engine->RegisterObjectMethod(typ, decl__, asMETHOD(UdpConnectionAS, func__), asCALL_THISCALL); assert( r >= 0 );
#define MO__REG_METHOD_F(decl__, func__) \
    r = engine->RegisterObjectMethod(typ, decl__, asFUNCTION(func__), asCALL_CDECL); assert( r >= 0 );
#define MO__REG_FUNC(decl__, func__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(func__), asCALL_CDECL); assert( r >= 0 );



// --- base object interface ---
static void register_connection(asIScriptEngine *engine, const char * typ)
{
    int r;

    // -------------------- types ------------------------------

    r = engine->RegisterObjectType(typ, 0, asOBJ_REF); assert( r >= 0 );
    r = engine->RegisterFuncdef("void UdpStringCallback(const string &in data)"); assert( r >= 0 );


    // ----------------- constructor ---------------------------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
        "UdpConnection@ f()", asFUNCTION(UdpConnectionAS::factoryNew), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_ADDREF,
        "void f()", asMETHOD(UdpConnectionAS,addRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_RELEASE,
        "void f()", asMETHOD(UdpConnectionAS,releaseRef), asCALL_THISCALL); assert( r >= 0 );

    // --------------- the object methods ----------------------

    // strings
    MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("string toString() const", toString);

    // getter
    MO__REG_METHOD("bool isOpen() const", isOpen);

    // setter
    MO__REG_METHOD("bool open(const string &in addr, uint16 port)", open);
    MO__REG_METHOD("void close()", close);

    MO__REG_METHOD("void addCallback(UdpStringCallback@ cb)", addStringCallback);
    MO__REG_METHOD("void removeCallback(UdpStringCallback@ cb)", removeStringCallback);

    MO__REG_METHOD("bool write(const string &in)", writeString);



    // ------------ non-member object functions ----------------

//    MO__REG_FUNC("", );

}


#undef MO__REG_FUNC
#undef MO__REG_METHOD
#undef MO__REG_METHOD_F

} // namespace native

} // namespace ----------------------------------------------


void registerAngelScript_network(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"network type for Angelscript not supported on this platform");
    }
    else
    {
        // base object type for each class
        native::register_connection(engine, "UdpConnection");
    }
}



// ---------------------------- instantiations ---------------------------------

/*
void registerAngelScript_rootObject(asIScriptEngine *engine, Scene* root, bool writeable)
{
    int r;
    ObjectAS * as = ObjectAS::factory(root);
    if (writeable)
    {
        r = engine->RegisterGlobalProperty("Object@ scene", &as->self); assert( r >= 0 );
    }
    else
    {
        r = engine->RegisterGlobalProperty("const Object@ scene", &as->self); assert( r >= 0 );
    }
}

void registerAngelScript_object(asIScriptEngine *engine, Object * obj, bool writeable, bool withRoot)
{
    int r;
    ObjectAS * as = ObjectAS::factory(obj);
    if (writeable)
    {
        r = engine->RegisterGlobalProperty("Object@ object", &as->self); assert( r >= 0 );
    }
    else
    {
        r = engine->RegisterGlobalProperty("const Object@ object", &as->self); assert( r >= 0 );
    }

    if (withRoot)
    {
        if (!obj || !obj->sceneObject())
            registerAngelScript_rootObject(engine, 0, writeable);
        else
            registerAngelScript_rootObject(engine, obj->sceneObject(), writeable);
    }
}
*/



} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
