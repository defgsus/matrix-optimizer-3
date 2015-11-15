/** @file testxmlstream.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/26/2014</p>
*/

#include <iostream>

#include "testxmlstream.h"
#include "types/properties.h"
#include "io/xmlstream.h"
#include "io/log.h"

TestXmlStream::TestXmlStream()
{
}


int TestXmlStream::run()
{
    return test1();
}


int TestXmlStream::test0()
{
    try
    {
        using namespace MO::IO;

        XmlStream io;

        io.startWriting();

        io.createSection("patch");
            io.write("version", "1");
            io.createSection("module");
                io.write("class", "Math");
                io.write("id", "Math1");
                io.write("name", "Mathematik");
                io.createSection("param");
                    io.write("id", "num_in");
                    io.write("v", 2);
                io.endSection();
            io.endSection();
            io.createSection("connections");
                io.createSection("c");
                io.write("fm", "Math1");
                io.write("fc", "out1");
                io.write("tm", "Math1");
                io.write("tc", "in1");
                io.endSection();
            io.endSection();
            io.createSection("pair-test");
                io << Pair("string", "A") << Pair("int", 23);
            io.endSection();
        io.endSection();

        io.stopWriting();

        qDebug("-------------");

        io.startReading();

        io.nextSubSection();
        if (io.section() != "patch") { std::cout << "expected patch\n"; exit(-1); }

        std::cout << "version " << io.readInt("version") << std::endl;



        io.stopReading();

    }
    catch (...)
    {
        std::cout << "error";
        throw;
    }

    return 0;
}

int TestXmlStream::test1()
{
    try
    {
        MO::Properties::NamedValues nv;
        nv.set("id0", "null", "", 0);
        nv.set("id1", "eins", "", 1);
        nv.set("id2", "zwei", "", 2);

        MO::Properties prop;
        prop.set("type", QObject::tr("type"),
                         QObject::tr("The type of geometry, source or file"),
                         nv, 1);

        prop.set("filename-obj", QObject::tr("obj filename"),
                         QObject::tr("The Wavefront object file to load"),
                         QString());
        prop.setSubType("filename-obj", MO::Properties::ST_FILENAME);

        prop.set("asTriangles", QObject::tr("create triangles"),
                         QObject::tr("Selects lines or triangles"),
                         true);
        prop.set("shared", QObject::tr("shared vertices"),
                         QObject::tr("Minimizes the amount of vertices by "
                                     "reusing the same for adjacent primitives"),
                         true);
        prop.set("radius", QObject::tr("radius"),
                         QObject::tr("The radius of the object to create"),
                         0.1f, 0.1f);
        prop.set("closed", QObject::tr("closed"),
                         QObject::tr("Enables closing the geometry with end caps"),
                         true);

        prop.set("segments", QObject::tr("number segments"),
                         QObject::tr("Number of segments on x, y, z"),
                         QVector<uint>() << 10 << 10 << 1);

        prop.set("color", QObject::tr("ambient color"),
                         QObject::tr("The base color of the geometry"),
                         QVector<float>() << .5f << .5f << .5f << 1.f,
                         QVector<float>() << 0.f << 0.f << 0.f << 0.f,
                         QVector<float>() << 1.f << 1.f << 1.f << 1.f,
                         QVector<float>() << .1f << .1f << .1f << .1f);

        MO::IO::XmlStream xml;
        xml.startWriting();
        prop.serialize(xml);
        xml.stopWriting();
        MO_PRINT(xml.data());

        MO::Properties prop2;
        prop2.setNamedValues("type", nv);
        xml.startReading();
        xml.nextSubSection();
        prop2.deserialize(xml);
        xml.stopReading();
        MO_PRINT(prop2.toString());
    }
    catch (...)
    {
        std::cout << "error";
        throw;
    }

    return 0;
}
