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
        //MO_IO_ERROR("w " << &w);
        return a.exec();
    }
    //catch (std::exception& e)
    catch (std::exception& e)
    {
        std::cout << "std::exception '" << e.what() << "'" << std::endl;
        throw;
    }

}
