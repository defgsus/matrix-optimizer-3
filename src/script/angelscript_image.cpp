/** @file angelscript_image.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.01.2015</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>
#include <cstring> // strstr
#include <sstream>
#include <memory>

#include <QImage>
#include <QPainter>

#include "angelscript_image.h"
#include "angelscript.h"
#include "types/refcounted.h"
#include "types/vector.h"
#include "io/error.h"
#include "io/log.h"

#if 1
#   define MO_DEBUG_IAS(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_IAS(unused__)
#endif


namespace MO {

//------------------------------
// AngelScript functions wrapper
//------------------------------


class ImageAS : public RefCounted
{
    QImage * img;
    QPainter * p_;
    bool own;
public:
    ImageAS * self;

    const uint max_size = 1<<30;

    ImageAS(quint32 width, quint32 height, QImage::Format format)
        : img   (new QImage(std::max(1u, width), std::max(1u, height), format)),
          p_    (0),
          own   (true),
          self  (this)
    {
        MO_DEBUG_IAS("ImageAS::ImageAS(" << img->width() << ", " << img->height() << ")");
    }

    ImageAS(QImage * img)
        : img   (img),
          p_    (0),
          own   (false),
          self  (this)
    {
        MO_DEBUG_IAS("ImageAS::ImageAS(" << img << ")");
        MO_ASSERT(img, "Can't create wrapper around null image");
        if (img->width() < 1 || img->height() < 1)
            resize(std::max(1, img->width()), std::max(1, img->height()), img->format());
    }

    ~ImageAS()
    {
        MO_DEBUG_IAS("ImageAS::~ImageAS(" << img->width() << ", " << img->height() << ")");
        delete p_;
        if (own)
            delete img;
    }

    // -------- helper ---------------------------

    static ImageAS * factory(QImage * img) { return new ImageAS(img); }
    static ImageAS * factoryNew(quint32 w, quint32 h, QImage::Format f) { return new ImageAS(w, h, f); }

    static void factoryGeneric(asIScriptGeneric * gen)
    {
        // create wrapper
        uint w = gen->GetArgDWord(0),
             h = gen->GetArgDWord(1),
             f = gen->GetArgDWord(2);
        ImageAS * img = factoryNew(w, h, QImage::Format(f));

        // set return value
        *(ImageAS**)gen->GetAddressOfReturnLocation() = img;

        // notify observer
        /*if (asIScriptEngine * engine = gen->GetEngine())
        {
            void * p = engine->GetUserData(MO_AS_OBSERVER); ...
        }*/
    }

    // float[0,1] to int[0,255] with clamp
    static int ftoi(float x) { return (std::max(0, int(x)) * 255) & 0xff; }

    // float to QRgb
    static QRgb ftoc(float r, float g, float b, float a) { return qRgba(ftoi(r), ftoi(g), ftoi(b), ftoi(a)); }
    static QRgb ftoc(float r, float g, float b) { return qRgb(ftoi(r), ftoi(g), ftoi(b)); }

    // safe coordinate wrapper
    QPoint coord(int x, int y) const { return QPoint(glm::clamp(x, 0, img->width()), glm::clamp(y, 0, img->height())); }
    QPoint coord(const Vec2& p) const { return coord(int(p.x), int(p.y)); }

    // lazy getter for qpainter
    QPainter& painter() { if (!p_) p_ = new QPainter(img); return *p_; }

    // --------------- interface -----------------


    // ------ getter ------

    StringAS toString() const
    {
        std::stringstream s;
        s << "Image(" << img->width() << "x" << img->height() << ")";
        return s.str();
    }

    quint32 width() const { return img->width(); }
    quint32 height() const { return img->height(); }
    quint32 format() const { return img->format(); }

    // ------ setter ------

    void resize(uint w, uint h, QImage::Format f)
    {
        QImage tmp(std::max(1u, std::min(max_size, w)),
                   std::max(1u, std::min(max_size, h)),
                   f);
        img->swap(tmp);
    }

    void fill_4f(float r, float g, float b, float a) { img->fill(ftoc(r, g, b, a)); }
    void setPixel_4f(int x, int y, float r, float g, float b, float a) { img->setPixel(coord(x,y), ftoc(r, g, b, a)); }

    void line_2i(int x, int y, int x1, int y1) { painter().drawLine(coord(x,y), coord(x1,y1)); }
    void line_v(const Vec2& a, const Vec2& b) { painter().drawLine(coord(a), coord(b)); }
};


namespace {



//--------------------------------
// Registration
//-------------------------------------

namespace native {

#define MO__REG_METHOD(decl__, func__) \
    r = engine->RegisterObjectMethod(typ, decl__, asMETHOD(ImageAS, func__), asCALL_THISCALL); assert( r >= 0 );
#define MO__REG_METHOD_F(decl__, func__) \
    r = engine->RegisterObjectMethod(typ, decl__, asFUNCTION(func__), asCALL_CDECL); assert( r >= 0 );
#define MO__REG_FUNC(decl__, func__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(func__), asCALL_CDECL); assert( r >= 0 );


// enums only
void register_image_enums(asIScriptEngine * engine)
{
    const char * enumType = "ImageFormat";
    int r; Q_UNUSED(r);
    r = engine->RegisterEnum(enumType); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType, "IMG_RGB",      QImage::Format_RGB32); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType, "IMG_RGBA",     QImage::Format_ARGB32_Premultiplied); assert( r >= 0 );
    r = engine->RegisterEnumValue(enumType, "IMG_MONO",      QImage::Format_Mono); assert( r >= 0 );
}


// --- base object interface ---
static void register_image_type(asIScriptEngine *engine, const char * typ)
{
    int r; Q_UNUSED(r);

    // -------------------- types ------------------------------

    r = engine->RegisterObjectType(typ, 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------

//    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
//        "Image@ f(uint width = 1, uint height = 1, ImageFormat format = IMG_RGBA)", asFUNCTION(ImageAS::factoryNew), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
        "Image@ f(uint width = 1, uint height = 1, ImageFormat format = IMG_RGBA)", asFUNCTION(ImageAS::factoryGeneric), asCALL_GENERIC); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_ADDREF,
        "void f()", asMETHOD(ImageAS,addRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_RELEASE,
        "void f()", asMETHOD(ImageAS,releaseRef), asCALL_THISCALL); assert( r >= 0 );

    // --------------- the object methods ----------------------

    // strings
    MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("string toString() const", toString);

    // getter
    MO__REG_METHOD("uint width() const", width);
    MO__REG_METHOD("uint height() const", height);
    MO__REG_METHOD("ImageFormat format() const", format);

    // setter
    MO__REG_METHOD("void resize(uint width, uint height, ImageFormat format = IMG_RGBA)", resize);
    MO__REG_METHOD("void setPixel(int x, int y, float r, float g, float b, float a = 1.f)", setPixel_4f);

    MO__REG_METHOD("void fill(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.f)", fill_4f);
    MO__REG_METHOD("void drawLine(int x0, int y0, int x1, int y1)", line_2i);
    MO__REG_METHOD("void drawLine(const vec2 &in from, const vec2 &in to)", line_v);

    // ------------ non-member object functions ----------------

}


#undef MO__REG_FUNC
#undef MO__REG_METHOD
#undef MO__REG_METHOD_F

} // namespace native

} // namespace



// ---------------------------- public interface -------------------------

void registerAngelScript_image(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"image type for Angelscript not supported on this platform");
    }
    else
    {
        native::register_image_enums(engine);
        native::register_image_type(engine, "Image");
    }
}



// ---------------------------- instantiations ---------------------------------

void registerAngelScript_image(asIScriptEngine *engine, QImage * img, bool writeable)
{
    int r;
    Q_UNUSED(r)

    std::string decl = "Image@ image";
    if (!writeable)
        decl = "const " + decl;

    if (img)
    {
        ImageAS * as = ImageAS::factory(img);

        r = engine->RegisterGlobalProperty(decl.c_str(), &as->self); assert( r >= 0 );
    }
    else
    {
        static void * null_ = 0;

        r = engine->RegisterGlobalProperty(decl.c_str(), &null_); assert( r >= 0 );
    }
}







// ---------------------- ImageEngineAS -----------------------------

class ImageEngineAS::Private
{
public:

    Private(QImage * img, Object * o = 0)
        : object    (o),
          img       (img),
          ias       (img ? ImageAS::factory(img) : 0),
          engine    (0),
          context   (0)
    { }

    ~Private()
    {
        if (context)
            context->Release();

        ias->releaseRef("ImageAS destroy");
    }

    void createEngine();
    void messageCallback(const asSMessageInfo *msg);

    // global script function
    ImageAS * getImageAS() { if (ias) ias->addRef("ImageAS::getImageAS()"); return ias; }

    Object * object;
    QImage * img;
    ImageAS * ias;
    asIScriptEngine * engine;
    asIScriptContext * context;
    QString errors;
};


ImageEngineAS::ImageEngineAS(QImage * img)
    : p_    (new Private(img))
{
}

ImageEngineAS::ImageEngineAS()
    : p_    (new Private(0))
{
}

ImageEngineAS::~ImageEngineAS()
{
    delete p_;
}

asIScriptEngine * ImageEngineAS::scriptEngine()
{
    if (!p_->engine)
        p_->createEngine();
    return p_->engine;
}


void ImageEngineAS::Private::createEngine()
{
    int r; Q_UNUSED(r);

    engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    MO_ASSERT(engine, "");

    registerDefaultAngelScript(engine);

    registerAngelScript_image(engine, img, true);

    /*if (object)
    {
        // read access to object (parent of geometry)
        registerAngelScript_object(engine, object, false);

        // read access to root object
        if (auto scene = object->sceneObject())
            registerAngelScript_rootObject(engine, scene, false);
    }*/
}

void ImageEngineAS::Private::messageCallback(const asSMessageInfo *msg)
{
    MO_WARNING(QString("\n%1:%2 %3").arg(msg->row).arg(msg->col).arg(msg->message));
    // XXX sometimes segfaults
    //errors += QString("\n%1:%2 %3").arg(msg->row).arg(msg->col).arg(msg->message);
}

void ImageEngineAS::execute(const QString &qscript)
{
    MO_DEBUG_IAS("ImageEngineAS("<<this<<")::execute()");

    MO_ASSERT(p_->img, "no image in ImageEngineAS");

    // --- create a module ---

    auto module = scriptEngine()->GetModule("_img_module", asGM_ALWAYS_CREATE);
    if (!module)
        MO_ERROR("Could not create script module");

    QByteArray script = qscript.toUtf8();
    module->AddScriptSection("script", script.data(), script.size());

    p_->errors.clear();
    p_->engine->SetMessageCallback(asMETHOD(Private, messageCallback), this, asCALL_THISCALL);

    // compile
    int r = module->Build();

    if (r < 0)
        MO_ERROR(QObject::tr("Error parsing script") + ":" + p_->errors);

    p_->engine->ClearMessageCallback();

    // --- get main function ---

    asIScriptFunction *func = module->GetFunctionByDecl("void main()");
    if( func == 0 )
        MO_ERROR("The script must have the function 'void main()'\n");

    // --- get context for execution
    if (!p_->context)
        p_->context = scriptEngine()->CreateContext();
    else
        p_->context->Unprepare();

    if (!p_->context)
        MO_ERROR("Could not create script context");

    p_->context->Prepare(func);
    r = p_->context->Execute();

    if( r == asEXECUTION_EXCEPTION )
        MO_ERROR("An exception occured in the script: " << p_->context->GetExceptionString());

    if( r != asEXECUTION_FINISHED )
        MO_ERROR("The script ended prematurely");
}








} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
