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
#include "io/settings.h"
#include "gui/mainwindow.h"

//#include "tests/testtimeline.h"
//#include "tests/testxmlstream.h"

//#include "tool/stringmanip.h"
//#include "io/streamoperators_qt.h"
//#include "math/funcparser/parser.h"
//#include "io/log.h"
#include "types/vector.h"

int main(int argc, char *argv[])
{
    //void * ptr = new float;  MO_DEBUG(ptr);

    /*
    MO::Mat4 mat(1.0);
    std::cout << mat << std::endl;
    mat = glm::rotate(mat, 90.f, MO::Vec3(0,0,1));
    std::cout << mat << std::endl;
    return 0;
    */

    //return PPP_NAMESPACE::test_parser_();
    //MO::TestTimeline t; return t.run();
    //TestXmlStream t; return t.run();
    /*QString id("model399844975d99");
    MO::increase_id_number(id, 1);
    std::cout << id << std::endl;
    MO::increase_id_number(id, 1);
    std::cout << id << std::endl;
    return 0;
    */

    MO::startOfProgram();

    MO::application = new MO::Application(argc, argv);
    MO::settings = new MO::Settings(MO::application);

    auto mainwin = new MO::GUI::MainWindow;
    MO::application->setMainWindow(mainwin);
    mainwin->show();

    MO::application->setPaletteFor(mainwin);

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

    std::cout
#if (0)
        << "peak memory: " << MO::Memory::peak()
        << ", lost = " << MO::Memory::lost() << "\n"
#endif
        << "bis später" << std::endl;

    return ret;
}
