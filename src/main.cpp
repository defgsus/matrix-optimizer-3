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
#include "io/Application.h"
#include "io/Settings.h"
#include "io/isclient.h"
#include "io/CurrentThread.h"
#include "io/log.h"
#include "io/version.h"
#include "gui/MainWindow.h"
#include "gui/SplashScreen.h"
#include "engine/ClientEngine.h"
#include "maincommandline.h"
#include "io/DiskRenderSettings.h"
#include "python/34/python.h"

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





//#include "tests/Testtimeline.h"
//#include "tests/Testxmlstream.h"
//#include "tests/Testhelpsystem.h"
//#include "tests/Testequation.h"
//#include "tests/Testtesselator.h"
//#include "tests/Testcommandlineparser.h"
//#include "tests/Testdirectedgraph.h"
//#include "tests/Testcsg.h"
//#include "tests/Testpython.h"
//#include "tests/Testglwindow.h"
#include "tests/TestFloatMatrix.h"
//#include "tests/TestFft.h"
//#include "math/arithmeticarray.h"

//#include "types/vector.h"
//#include "tool/stringmanip.h"
//#include "io/streamoperators_qt.h"
//#include "math/funcparser/parser.h"
//#include "io/log.h"

int main(int argc, char *argv[])
{
    MO::setCurrentThreadName("GUI");

    // tests without QApplication
    //MO::PYTHON34::runConsole(argc, argv); return 0;
    //TestPython t; return t.run();
    //MO::TestCsg t; return t.run();
    //TestXmlStream t; return t.run();
    //MO::TestEquation t; return t.run();
    //MO::TestTesselator t; return t.run();
    //MO::TestDirectedGraph t; return t.run();
    //TestGlWindow t; return t.run();
    //MO::TestFloatMatrix t; return t.run();
    //MO::TestFft t; return t.run();

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
        //MO::PYTHON34::runConsole(argc, argv); return 0;

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
            if (0)
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

    MO::endOfProgram();

    std::cout << std::endl
//#ifndef NDEBUG
        << "bis später"
//#endif
        << std::endl;

    return ret;
}
