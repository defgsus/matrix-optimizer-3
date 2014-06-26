/** @file

    @brief main.cpp

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/

#include <iostream>

#include <QApplication>

#include "io/error.h"
#include "gui/mainwindow.h"
//#include "tests/testtimeline.h"

int main(int argc, char *argv[])
{
    //MO::TestTimeline t; return t.run();

    QApplication a(argc, argv);
    MO::GUI::MainWindow w;
    w.show();

    try
    {
        //throw MO::BasicException() << "hallo";
        //throw MO::IoException() << "hallo";
        //MO_IO_ERROR(VERSION_MISMATCH, "w " << &w);
        return a.exec();
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

}
