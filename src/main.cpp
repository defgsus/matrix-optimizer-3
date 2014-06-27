/** @file

    @brief main.cpp

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/

#include <iostream>

#include "io/error.h"
#include "io/application.h"
#include "gui/mainwindow.h"
#include "gui/qobjectinspector.h"

//#include "tests/testtimeline.h"
//#include "tests/testio.h"


int main(int argc, char *argv[])
{
    //MO::TestTimeline t; return t.run();
    //TestIo t; return t.run();

    MO::application = new MO::Application(argc, argv);

    MO::GUI::MainWindow w;
    w.show();

    //MO::GUI::QObjectInspector oi(&w);
    //oi.show();

    try
    {
        //throw MO::BasicException() << "hallo";
        //throw MO::IoException() << "hallo";
        //MO_IO_ERROR(VERSION_MISMATCH, "w " << &w);
        return MO::application->exec();
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

    std::cerr << "good bye." << std::endl;
    delete MO::application;
}
