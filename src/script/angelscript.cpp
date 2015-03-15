/** @file angelscript.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>

#include <QString>
#include <QFile>
#include <QTextStream>

#include "angelscript.h"
#include "angelscript_vector.h"
#include "angelscript_math.h"
#include "angelscript_object.h"
#include "angelscript_geometry.h"
#include "angelscript_timeline.h"
#include "angelscript_network.h"
#include "angelscript_image.h"
#include "3rd/angelscript/scriptmath/scriptmathcomplex.h" // XXX deprecated: will be replaced
#include "3rd/angelscript/scriptarray/scriptarray.h"
#include "3rd/angelscript/scriptstdstring/scriptstdstring.h"
#include "io/log.h"
#include "io/error.h"
#include "io/xmlstream.h"

namespace MO {



QString toString(const StringAS& as)
{
    return QString::fromUtf8(as.c_str());
}

StringAS toStringAS(const QString& qs)
{
    return StringAS( qs.toUtf8().constData() );
}








namespace {

    void angelPrint(const StringAS& s)
    {
        MO_DEBUG("angelscript: " << toString(s));
        Q_UNUSED(s)
    }

} // namespace

void registerDefaultAngelScript(asIScriptEngine * engine)
{
    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterScriptMathComplex(engine);

    //engine->SetDefaultNamespace("MO");
    registerAngelScript_vector(engine);
    registerAngelScript_math(engine);
    registerAngelScript_image(engine);
    registerAngelScript_timeline(engine);
    registerAngelScript_geometry(engine);
    registerAngelScript_object(engine);
    registerAngelScript_network(engine);


    //if (object)
    //    registerAngelScript_rootObject(engine, object);

    int r = engine->RegisterGlobalFunction("void print(const string & in)", asFUNCTION(angelPrint), asCALL_CDECL); assert( r >= 0 );
    Q_UNUSED(r)
}




// -------------------------------- namespace export -----------------------

namespace {

    void exportFunc(IO::XmlStream & xml, asIScriptFunction * func)
    {
        const QString
                name = QString::fromUtf8(func->GetName()),
                decl = QString::fromUtf8(func->GetDeclaration(true, false, true)),
                doc = "";

        xml.newSection("function");
        xml.write("name", name);
        xml.write("decl", decl);
        xml.write("doc", doc);
        xml.endSection();
    }

}

void exportAngelScriptFunctions(const QString & filename)
{
    IO::XmlStream xml;
    /*
    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
    {
        MO_IO_ERROR(WRITE, "Can't open for writing '" << filename << "'\n" << file.errorString());
    }

    QTextStream stream(&file);
    */

    xml.startWriting("mo-angelscript-doc");

    asIScriptEngine * engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    AngelScriptAutoPtr deleter_(engine);

    registerDefaultAngelScript(engine);

    //auto module = engine->GetModule();

    // global functions
    for (asUINT i=0; i<engine->GetGlobalFunctionCount(); ++i)
    {
        asIScriptFunction * func = engine->GetGlobalFunctionByIndex(i);
        MO_ASSERT(func, "function " << i << " not found");
        exportFunc(xml, func);
    }

    // object types
    for (asUINT i=0; i<engine->GetObjectTypeCount(); ++i)
    {
        asIObjectType* obj = engine->GetObjectTypeByIndex(i);
        const QString
                name = QString::fromUtf8(obj->GetName()),
                doc = "";

        xml.newSection("object");
        xml.write("name", name);
        xml.write("doc", doc);

        // object properties
        for (asUINT j=0; j<obj->GetPropertyCount(); ++j)
        {
            const char * name, * decl;
            obj->GetProperty(j, &name);
            decl = obj->GetPropertyDeclaration(j);
            const QString doc = "";

            xml.newSection("property");
            xml.write("name", QString::fromUtf8(name));
            xml.write("decl", QString::fromUtf8(decl));
            xml.write("doc", doc);
            xml.endSection();
        }

        // object behaviours (c/dtors)
        for (asUINT j=0; j<obj->GetBehaviourCount(); ++j)
        {
            asIScriptFunction * func = obj->GetBehaviourByIndex(j, 0);
            MO_ASSERT(func, "behaviour " << i << " not found");
            exportFunc(xml, func);
        }

        // object methods
        for (asUINT j=0; j<obj->GetMethodCount(); ++j)
        {
            asIScriptFunction * func = obj->GetMethodByIndex(j);
            MO_ASSERT(func, "function " << i << " not found");
            exportFunc(xml, func);
        }


        xml.endSection();
    }

    xml.stopWriting();
    xml.save(filename);
}

namespace {

    void exportFunc(QTextStream& html, asIScriptFunction * func)
    {
        const QString
                //name = QString::fromUtf8(func->GetName()),
                decl = QString::fromUtf8(func->GetDeclaration(true, false, true));

        html << "<p><b>" << decl.toHtmlEscaped() << "</b></p>";
    }
} // namespace

QString getAngelScriptFunctionsHtml()
{
    QString retstr;
    QTextStream html(&retstr);

    asIScriptEngine * engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    AngelScriptAutoPtr deleter_(engine);

    registerDefaultAngelScript(engine);

    // global functions
    html << "<h2>global functions</h2>\n";

    for (asUINT i=0; i<engine->GetGlobalFunctionCount(); ++i)
    {
        asIScriptFunction * func = engine->GetGlobalFunctionByIndex(i);
        MO_ASSERT(func, "function " << i << " not found");
        exportFunc(html, func);
        html << "\n";
    }

    // object types
    html << "<h2>classes</h2><ul>\n";
    for (asUINT i=0; i<engine->GetObjectTypeCount(); ++i)
    {
        html << "<li>";

        asIObjectType* obj = engine->GetObjectTypeByIndex(i);
        const QString
                name = QString::fromUtf8(obj->GetName());

        html << "<h3>" << name << "</h3>\n";

        // object properties
        html << "<p>properties</p><ul>";
        for (asUINT j=0; j<obj->GetPropertyCount(); ++j)
        {
            const char * name, * decl;
            obj->GetProperty(j, &name);
            decl = obj->GetPropertyDeclaration(j);

            html << "<li>" << QString::fromUtf8(decl).toHtmlEscaped() << "</li>";
        }
        html << "</ul>";

        // object behaviours (c/dtors)
        html << "<p>behaviours</p><ul>";
        for (asUINT j=0; j<obj->GetBehaviourCount(); ++j)
        {
            asIScriptFunction * func = obj->GetBehaviourByIndex(j, 0);
            MO_ASSERT(func, "behaviour " << i << " not found");
            html << "<li>";
            exportFunc(html, func);
            html << "</li>";
        }
        html << "</ul>";

        // object methods
        html << "<p>methods</p><ul>";
        for (asUINT j=0; j<obj->GetMethodCount(); ++j)
        {
            asIScriptFunction * func = obj->GetMethodByIndex(j);
            MO_ASSERT(func, "function " << i << " not found");
            html << "<li>";
            exportFunc(html, func);
            html << "</li>";
        }
        html << "</ul>";

        html << "</li>\n";
    }
    html << "</ul>";

    return retstr;
}



QString exampleAngelScript()
{
    QFile f(":/help/example_angelscript.txt");
    if (!f.open(QFile::ReadOnly | QFile::Text))
        return QString();
    QTextStream s(&f);
    return s.readAll();
}




} // namespace MO




#endif // #ifndef MO_DISABLE_ANGELSCRIPT




/**************************************************************************

    // --- some working examples ---


void dumpTree(Object@ o, string pre = "")
{
    print(pre + o.name());
    pre += " ";
    for (int i=0; i<o.childrenCount(); ++i)
        dumpTree(o.children(i), pre);
}

void testObject()
{
    Object@ o = Root;
    print(o.name() + " " + o + " " + o.childrenCount());
    if (o.childrenCount() >= 1)
        print(o.children(0));

    dumpTree(Root);
}


void printArray(const array<int> &in a)
{
    string s;
    for (uint i = 0; i < a.length(); ++i)
        s += a[i] + " ";
    print(s);
}

void printArray(const array<vec3> &in a)
{
    string s;
    for (uint i = 0; i < a.length(); ++i)
        s += " " + a[i];
    print(s);
}

void testArray()
{
    array<int> a = { 5, 4, 3, 2, 1 };

    print(a.toString());

    a.resize(23);
    print(a.toString());

    array<string> as = { "hallo", "welt" };
    print(as.toString());

    array<vec3> b = { vec3(1), vec3(2) };
    print(b.toString());

    array<array<int>> aa = { a, a };
    print(aa.toString());
}

void testVec()
{
    vec2 v2(1,2);
    vec3 v3(3,4,5);
    vec4 v4(6,7,8,9);
    print(""+v2+" "+v3+" "+v4);
    v2 = v2 + 1.f;
    v3 = v3 + 1.f;
    v4 = v4 + 1.f;
    print(""+v2+" "+v3+" "+v4);
    print(""+smallest(v2)+" "+smallest(v3)+" "+smallest(v4));
    print(""+largest(v2)+" "+largest(v3)+" "+largest(v4));

    print(rgb2hsv(vec3(0.5,0.2,0.2)));
    print(hsv2rgb(vec3(0.8,1,1)));
}

void testGeometry()
{
    Geometry g;
    Geometry@ g2 = g;
    print(""+g + " " + g2);
    g.addLine(vec3(0,0,0), vec3(1,1,1));
    print(""+g.vertexCount());
}

void main()
{
    print("----------------------------");

    testArray();
    //testVec();
}


*************************************************************************/
