/** @file moapplication.cpp

    @brief QApplication wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include <iostream>

#include "io/application.h"
#include "io/error.h"

namespace MO {

Application * application;

Application::Application(int& argc, char** args)
    :   QApplication(argc, args)
{ }

bool Application::notify(QObject * o, QEvent * e)
{
    try
    {
        return QApplication::notify(o, e);
    }
    catch (MO::Exception& e)
    {
        std::cout << "Exception in notify [" << e.what() << "]" << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "std::exception in notify [" << e.what() << "]" << std::endl;
    }
    catch (...)
    {
        std::cout << "unrecognized exception in notify" << std::endl;
    }
    return false;
}


} // namespace MO
