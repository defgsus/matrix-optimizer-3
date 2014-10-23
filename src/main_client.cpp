/** @file main_client.cpp

    @brief Matrix Optimizer III Client

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <iostream>

#include "io/init.h"
#include "io/application.h"
#include "io/settings.h"
#include "io/currentthread.h"
#include "engine/clientengine.h"

int main(int argc, char *argv[])
{
    MO::setCurrentThreadName("GUI");

    MO::startOfProgram();

    // application and settings
    MO::application = new MO::Application(argc, argv);
    MO::settings = new MO::Settings(MO::application);

    // stats
    MO::settings->setValue("Stats/clientRuns",
                           MO::settings->value("Stats/clientRuns").toInt() + 1);

    int ret = MO::clientEngine().run(argc, argv);

    delete MO::application;

    //std::cout << "bis später" << std::endl;

    return ret;
}

