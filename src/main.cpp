/** @file main.cpp

    @brief Main duty dispatcher or whatever

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#include <memory>

#include <QFile>
#include <QTextStream>

#include "io/init.h"
#include "io/application.h"
#include "io/settings.h"
#include "io/isclient.h"
#include "io/currentthread.h"
#include "io/log.h"
#include "io/version.h"
#include "gui/mainwindow.h"
#include "gui/splashscreen.h"
#include "engine/clientengine.h"
#include "maincommandline.h"
#include "io/diskrendersettings.h"

void showHelp()
{
    QFile f(":/help/commandlinehelp.txt");
    f.open(QFile::ReadOnly);
    QTextStream s(&f);

    MO_PRINT(
           "\n"
        << MO::applicationName()
        << "\n\n"
        << s.readAll()
        << "\n" << MO::MainCommandLine().help()
        );
}


int renderToDisk()
{
    return 0;
}





//#include "tests/testtimeline.h"
//#include "tests/testxmlstream.h"
//#include "tests/testhelpsystem.h"
//#include "tests/testequation.h"
//#include "tests/testtesselator.h"
//#include "tests/testcommandlineparser.h"
//#include "tests/testdirectedgraph.h"

//#include "types/vector.h"
//#include "tool/stringmanip.h"
//#include "io/streamoperators_qt.h"
//#include "math/funcparser/parser.h"
//#include "io/log.h"

int main(int argc, char *argv[])
{
    MO::setCurrentThreadName("GUI");

    // tests without QApplication
    //MO::TestEquation t; return t.run();
    //MO::TestTesselator t; return t.run();
    //MO::TestDirectedGraph t; return t.run();

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

    // get major command switch
    QString command;
    if (argc > 1)
        command = argv[1];

    // determine what to do
    bool doRender = false;
    if (0 == command.compare("-h", Qt::CaseInsensitive)
     || command.contains("help", Qt::CaseInsensitive))
    {
        showHelp();
        return 0;
    }
    else
    if (0 == command.compare("client", Qt::CaseInsensitive))
    {
        MO::setThisApplicationToClient();
    }
    else
    if (0 == command.compare("render", Qt::CaseInsensitive))
    {
        doRender = true;
    }
    else
    if (!command.isEmpty() && !command.startsWith("-"))
    {
        showHelp();
        MO_PRINT("Unrecognized mode '" << command << "'");
        return 1;
    }

    // application scope
    int ret;
    {
        // any 3rd library code that needs global initialization
        // goes in here
        MO::startOfProgram();

        // create QApplication
        MO::createApplication(argc, argv);
        std::unique_ptr<MO::Application> app_deleter(MO::application());

        MO::application()->setAttribute(Qt::AA_X11InitThreads);

        // tests with QApplication
        //{ MO::TestHelpSystem test; return test.run(); }
        //{ MO::TestCommandLineParser test; return test.run(argc, argv, 1); }

        // ------ start program ---------

        // disk renderer
        if (doRender)
            ret = renderToDisk();

        // --- client engine ---
        else
        if (MO::isClient())
        {
            ret = MO::clientEngine().run(argc, argv, 2);
        }

        // --- server/gui engine ---
        else
        {
            // parse server/gui/desktop commandline
            MO::MainCommandLine cl;
            auto comret= cl.parse(argc, argv, 1);
            if (comret != MO::MainCommandLine::Ok)
                return comret == MO::MainCommandLine::Error ? 1 : 0;

        #ifdef NDEBUG
            MO::GUI::SplashScreen * splash = 0;
            if (1)
            {
                splash = new MO::GUI::SplashScreen();
                splash->show();
            }
        #endif

            auto mainwin = new MO::GUI::MainWindow;
            MO::application()->setMainWindow(mainwin);

            mainwin->show();

        #ifdef NDEBUG
            if (splash)
                splash->raise();
        #endif

            MO::application()->setStyleSheet(MO::settings()->styleSheet());

            //MO::GUI::QObjectInspector oi(&w);
            //oi.show();

            ret = MO::application()->exec();
        }

        // ----- end of program ------

    }

    std::cout << std::endl
#if (0)
        << "peak memory: " << MO::Memory::peak()
        << ", lost = " << MO::Memory::lost() << "\n"
#endif
#ifndef NDEBUG
        << "bis später"
#endif
        << std::endl;

    return ret;
}
