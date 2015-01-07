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
#include "io/settings.h"
#include "io/applicationtime.h"
#include "io/currenttime.h"
#include "io/log.h"
#include "io/error.h"
#include "io/systeminfo.h"

#if 1
#   define MO_DEBUG_NAS(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_NAS(unused__)
#endif


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
            MO_DEBUG_NAS("UdpCallback::UdpCallback(" << f << ")");

            if (f)
            {
                context = f->GetEngine()->CreateContext();
                function->AddRef();
            }
        }
        ~Callback()
        {
            MO_DEBUG_NAS("UdpCallback::~UdpCallback(" << function << ")");

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
        MO_DEBUG_NAS("UdpConnectionAS::UdpConnectionAS(" << con << ")");
        MO_ASSERT(con, "Can't create wrapper for NULL Connection");
        con->addRef();
        connectListen_();
    }

    UdpConnectionAS() : con(new UdpConnection), ref(1)
    {
        MO_DEBUG_NAS("UdpConnectionAS::UdpConnectionAS() created " << con);
        connectListen_();
    }

    ~UdpConnectionAS()
    {
        MO_DEBUG_NAS("UdpConnectionAS::~UdpConnectionAS() con = " << con);
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
    bool openAny(uint16_t port) { return con->open(port); }
    void close() { con->close(); }

    bool sendString(const StringAS& s) { return con->sendDatagram(s.c_str(), s.size()); }
    bool sendStringAP(const StringAS& s, const StringAS& addr, uint16_t port)
        { return con->sendDatagram(s.c_str(), s.size(), QHostAddress(MO::toString(addr)), port); }

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
                callbacks.remove(i);
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
        QByteArray data = con->readData();
        if (data.isEmpty())
        {
            MO_WARNING("UdpConnectionAS received null data from UdpConnection");
            return;
        }

        for (auto & i : callbacks)
        {
            Callback * cb = i.get();

            cb->context->Prepare(cb->function);

            // construct data type
            StringAS s = data.constData();
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

struct netfuncs
{
    static bool isClient() { return MO::isClient(); }
    static bool isServer() { return MO::isServer(); }
    static int clientIndex() { return settings->clientIndex(); }
    static StringAS serverAddress() { return toStringAS(settings->serverAddress()); }
    static StringAS localAddress() { SystemInfo inf; inf.get(); return toStringAS(inf.localAddress()); }

    static Double applicationTime() { return MO::applicationTime(); }
    static Double sceneTime() { return CurrentTime::time(); }
};


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
    int r; Q_UNUSED(r);

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
    MO__REG_METHOD("bool open(uint16 port)", openAny);
    MO__REG_METHOD("void close()", close);

    MO__REG_METHOD("void addCallback(UdpStringCallback@ cb)", addStringCallback);
    MO__REG_METHOD("void removeCallback(UdpStringCallback@ cb)", removeStringCallback);

    MO__REG_METHOD("bool send(const string &in data)", sendString);
    MO__REG_METHOD("bool send(const string &in data, const string &in addr, uint16 port)", sendStringAP);

    MO__REG_METHOD("void keepAlive()", addRef);

    // ------------ non-member object functions ----------------

}

void register_network(asIScriptEngine * engine)
{
    int r; Q_UNUSED(r);

    // ------------ non-member object functions ----------------

    MO__REG_FUNC("bool isClient()", netfuncs::isClient);
    MO__REG_FUNC("bool isServer()", netfuncs::isServer);
    MO__REG_FUNC("int clientIndex()", netfuncs::clientIndex);
    MO__REG_FUNC("string localAddress()", netfuncs::localAddress);
    MO__REG_FUNC("string serverAddress()", netfuncs::serverAddress);

    MO__REG_FUNC("double applicationTime()", netfuncs::applicationTime);
    MO__REG_FUNC("double sceneTime()", netfuncs::sceneTime);

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
        native::register_network(engine);
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





/* ------  stuff that works ---------


void printNetwork()
{
    print("---- network -----");
    print("server         " + (isServer() ? "yes" : "no"));
    print("local address  " + localAddress());
    print("server address  " + serverAddress());
}

void udpReceive(const string &in msg)
{
    print("udp message " + msg.length() + " bytes "
        " [" + msg + "]");
}

void testUDPReceive()
{
    print("---- udp receive ----");

    UdpConnection udp;
    if (!udp.open(//"192.168.1.35",
             51000))
    {
        print("Can't open");
        return;
    }

    udp.addCallback(udpReceive);
    udp.keepAlive();
    print("Installed listener");
}



---------------------- */
