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
#include "io/application.h"
#include "io/settings.h"
#include "io/currentthread.h"
#include "gui/mainwindow.h"
#include "gui/audiolinkwindow.h"
#include "gui/splashscreen.h"

//#include "tests/testtimeline.h"
//#include "tests/testxmlstream.h"
//#include "tests/testhelpsystem.h"
//#include "tests/testequation.h"

//#include "types/vector.h"
//#include "tool/stringmanip.h"
//#include "io/streamoperators_qt.h"
//#include "math/funcparser/parser.h"
//#include "io/log.h"

int main(int argc, char *argv[])
{
    MO::setCurrentThreadName("GUI");

    //MO::TestEquation t; return t.run();

#if (0)
    using namespace MO;
    Mat4 mat(1.0);
    std::cout << mat << std::endl;
    mat = MATH::rotate(mat, 45.f, Vec3(0,0,1));
    std::cout << mat << std::endl;
    Mat4 mat2(1.0);
    std::cout << (mat + 0.5 * (mat2 - mat)) << std::endl;
    //mat = glm::scale(mat, Vec3(2, 1, 1));
    //std::cout << mat << std::endl;
    return 0;
#endif

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

    // tests
    //{ MO::TestHelpSystem test; return test.run(); }

#ifdef NDEBUG
    auto splash = new MO::GUI::SplashScreen();
    splash->show();
#endif

    auto mainwin = new MO::GUI::MainWindow;
    MO::application->setMainWindow(mainwin);

    mainwin->show();

    //auto audiowin = new MO::GUI::AudioLinkWindow;
    //audiowin->show();

#ifdef NDEBUG
    splash->raise();
#endif

    MO::application->setPaletteFor(mainwin);

    //MO::GUI::QObjectInspector oi(&w);
    //oi.show();

    int ret = MO::application->exec();

    delete MO::application;

    std::cout << std::endl
#if (0)
        << "peak memory: " << MO::Memory::peak()
        << ", lost = " << MO::Memory::lost() << "\n"
#endif
        << "bis später" << std::endl;

    return ret;
}
