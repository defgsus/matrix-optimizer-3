/** @file main_client.cpp

    @brief Matrix Optimizer III Client

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <qglobal.h>
#include <QDesktopWidget>

#include <iostream>

#include "io/init.h"
#include "io/application.h"
#include "io/settings.h"
#include "io/currentthread.h"

int main(int argc, char *argv[])
{
    MO::setCurrentThreadName("GUI");

    MO::startOfProgram();

    MO::application = new MO::Application(argc, argv);
    MO::settings = new MO::Settings(MO::application);

/*    auto mainwin = new MO::GUI::MainWindow;
    MO::application->setMainWindow(mainwin);

    mainwin->show();
*/

    int ret = MO::application->exec();


    delete MO::application;

    std::cout << "bis später" << std::endl;

    return ret;
}

