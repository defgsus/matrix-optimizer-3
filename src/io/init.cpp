/** @file init.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/


#include <QtGlobal>

#include "init.h"
#include "python/34/python.h"
#include "io/error.h"

#ifdef Q_OS_LINUX
#   include <X11/Xlib.h>
#endif

namespace MO {

void startOfProgram()
{
#ifdef Q_OS_LINUX
    struct dummy_
    {
        static int errHandler(Display * , XErrorEvent * e)
        {
            MO_WARNING("Catch X11 error " << e->error_code);
            return 0;
        }
    };

    XSetErrorHandler(dummy_::errHandler);

    XInitThreads();
#endif


#ifdef MO_ENABLE_PYTHON34
    MO::PYTHON34::initPython();
#endif

}


}
