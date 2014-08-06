/** @file testio.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/26/2014</p>
*/

#include <iostream>

#include "testxmlstream.h"
#include "io/xmlstream.h"


TestXmlStream::TestXmlStream()
{
}


int TestXmlStream::run()
{
    try
    {
        using namespace MO::IO;

        XmlStream io;

        io.startWriting();

        io.newSection("patch");
            io.write("version", "1");
            io.newSection("module");
                io.write("class", "Math");
                io.write("id", "Math1");
                io.write("name", "Mathematik");
                io.newSection("param");
                    io.write("id", "num_in");
                    io.write("v", 2);
                io.endSection();
            io.endSection();
            io.newSection("connections");
                io.newSection("c");
                io.write("fm", "Math1");
                io.write("fc", "out1");
                io.write("tm", "Math1");
                io.write("tc", "in1");
                io.endSection();
            io.endSection();
            io.newSection("pair-test");
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
