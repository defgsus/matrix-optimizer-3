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
#include "engine/client.h"

int main(int argc, char *argv[])
{
    MO::setCurrentThreadName("GUI");

    MO::startOfProgram();

    MO::application = new MO::Application(argc, argv);
    MO::settings = new MO::Settings(MO::application);

    MO::Client * client = new MO::Client(MO::application);

    int ret = client->run(argc, argv);

    delete MO::application;

    std::cout << "bis später" << std::endl;

    return ret;
}

