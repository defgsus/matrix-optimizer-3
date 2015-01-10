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
    QImage img;
    QPainter * p_;
public:

    ImageAS(uint width, uint height, QImage::Format format)
        : img   (std::max(1u, width), std::max(1u, height), format),
          p_    (0)
    {
        MO_DEBUG_IAS("ImageAS::ImageAS(" << img.width() << ", " << img.height() << ")");
    }

    ~ImageAS()
    {
        MO_DEBUG_IAS("ImageAS::~ImageAS(" << img.width() << ", " << img.height() << ")");
        delete p_;
    }

    // -------- helper ---------------------------

    static ImageAS * factoryNew(uint w, uint h, QImage::Format f) { return new ImageAS(w, h, f); }

    // float[0,1] to int[0,255] with clamp
    static int ftoi(float x) { return (std::max(0, int(x)) * 255) & 0xff; }

    // float to QRgb
    static QRgb ftoc(float r, float g, float b, float a) { return qRgba(ftoi(r), ftoi(g), ftoi(b), ftoi(a)); }
    static QRgb ftoc(float r, float g, float b) { return qRgb(ftoi(r), ftoi(g), ftoi(b)); }

    // safe coordinate wrapper
    QPoint coord(int x, int y) const { return QPoint(glm::clamp(x, 0, img.width()), glm::clamp(y, 0, img.height())); }

    // lazy getter for qpainter
    QPainter& painter() { if (!p_) p_ = new QPainter(&img); return *p_; }

    // --------------- interface -----------------


    // ------ getter ------

    StringAS toString() const
    {
        std::stringstream s;
        s << "Image(" << img.width() << "x" << img.height() << ")";
        return s.str();
    }

    // ------ setter ------

    void fill_4f(float r, float g, float b, float a) { img.fill(ftoc(r, g, b, a)); }
    void setPixel_4f(int x, int y, float r, float g, float b, float a) { img.setPixel(coord(x,y), ftoc(r, g, b, a)); }
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

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_FACTORY,
        "Image@ f(uint width = 1, uint height = 1, ImageFormat format = IMG_RGBA)", asFUNCTION(ImageAS::factoryNew), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_ADDREF,
        "void f()", asMETHOD(ImageAS,addRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_RELEASE,
        "void f()", asMETHOD(ImageAS,releaseRef), asCALL_THISCALL); assert( r >= 0 );

    // --------------- the object methods ----------------------

    // strings
    MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("string toString() const", toString);

    // getter
    //MO__REG_METHOD("bool isOpen() const", isOpen);

    // setter
    MO__REG_METHOD("void setPixel(int x, int y, float r, float g, float b, float a = 1.f)", setPixel_4f);
    MO__REG_METHOD("void fill(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.f)", fill_4f);

    // ------------ non-member object functions ----------------

}


#undef MO__REG_FUNC
#undef MO__REG_METHOD
#undef MO__REG_METHOD_F

} // namespace native

} // namespace ----------------------------------------------


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
