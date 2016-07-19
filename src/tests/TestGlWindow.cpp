/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/21/2016</p>
*/

#include "TestGlWindow.h"
#include "gl/GlWindow.h"
#include "gl/GlContext.h"
#include "io/error.h"
#include "io/log.h"

using namespace MO;

struct TestGlWindow::Private
{
    Private(TestGlWindow*p)
        : p         (p)
        , window    (nullptr)
        , context   (nullptr)
    {

    }

    void run();

    TestGlWindow* p;

    GL::GlWindow* window;
    GL::GlContext* context;
};

TestGlWindow::TestGlWindow()
    : p_            (new Private(this))
{

}

TestGlWindow::~TestGlWindow()
{
    delete p_;
}

int TestGlWindow::run()
{
    try
    {
        p_->run();
    }
    catch (const Exception& e)
    {
        MO_WARNING(e.what());
        delete p_->context;
        delete p_->window;
        return -1;
    }
    return 0;
}

void TestGlWindow::Private::run()
{
    window = new GL::GlWindow(512, 512);
    context = new GL::GlContext(window);


    while (window->update())
    {

        window->swapBuffers();
    }

    delete context;
    delete window;
}
