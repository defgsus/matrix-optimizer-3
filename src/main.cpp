/** @file

    @brief main.cpp

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#include <qglobal.h>
#include <QDesktopWidget>

#include <iostream>

#include "io/init.h"
#include "io/error.h"
#include "io/application.h"
#include "gui/mainwindow.h"

//#include "tests/testtimeline.h"
//#include "tests/testxmlstream.h"

//#include "tool/stringmanip.h"
//#include "io/streamoperators_qt.h"

int main(int argc, char *argv[])
{
    MO::startOfProgram();

    //MO::TestTimeline t; return t.run();
    //TestXmlStream t; return t.run();
    /*QString id("model399844975d99");
    MO::increase_id_number(id, 1);
    std::cout << id << std::endl;
    MO::increase_id_number(id, 1);
    std::cout << id << std::endl;
    return 0;
    */
    MO::application = new MO::Application(argc, argv);

    auto mainwin = new MO::GUI::MainWindow;
    mainwin->show();

    // center mainwindow
    QRect r = MO::application->desktop()->screenGeometry(mainwin->pos());
    mainwin->setGeometry(
                (r.width()-mainwin->width())/2,
                (r.height()-mainwin->height())/2,
                mainwin->width(), mainwin->height());

    //MO::GUI::QObjectInspector oi(&w);
    //oi.show();

    int ret;
    try
    {
        //throw MO::Exception() << "hallo";
        //throw MO::IoException() << "hallo";
        //MO_IO_ERROR(VERSION_MISMATCH, "win " << mainwin);
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
