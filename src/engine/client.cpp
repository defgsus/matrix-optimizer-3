/** @file client.cpp

    @brief Client runtime process

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include "client.h"
#include "io/log.h"
#include "io/application.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "object/scene.h"

namespace MO {

Client::Client(QObject *parent) :
    QObject(parent)
{
}

int Client::run()
{
    MO_PRINT(tr("Matrix Optimizer Client"));

    createObjects_();

    int ret = application->exec();

    delete glManager_;

    return ret;
}


void Client::createObjects_()
{
    glManager_ = new GL::Manager(this);
    glWindow_ = glManager_->createGlWindow(MO_GFX_THREAD);

    glWindow_->show();//FullScreen();
}

} // namespace MO
