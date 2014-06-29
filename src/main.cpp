/** @file

    @brief main.cpp

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#include <qglobal.h>

#include <iostream>

#include "io/init.h"
#include "io/error.h"
#include "io/application.h"
#include "gui/mainwindow.h"

//#include "tests/testtimeline.h"
//#include "tests/testxmlstream.h"


int main(int argc, char *argv[])
{
    MO::startOfProgram();

    //MO::TestTimeline t; return t.run();
    //TestXmlStream t; return t.run();


    MO::application = new MO::Application(argc, argv);

    MO::GUI::MainWindow w;
    w.show();

    //MO::GUI::QObjectInspector oi(&w);
    //oi.show();

    int ret;
    try
    {
        //throw MO::BasicException() << "hallo";
        //throw MO::IoException() << "hallo";
        //MO_IO_ERROR(VERSION_MISMATCH, "w " << &w);
        ret = MO::application->exec();
    }
    /*catch (MO::IoException& e)
    {
        std::cout << "IoException '" << e.what() << "'" << std::endl;
        throw;
    }*/
    catch (MO::Exception& e)
    {
        std::cout << "Exception '" << e.what() << "'" << std::endl;
        throw;
    }
    catch (std::exception& e)
    {
        std::cout << "std::exception '" << e.what() << "'" << std::endl;
        throw;
    }

    delete MO::application;

    std::cout << "good bye" << std::endl;

    return ret;
}
