/** @file

    @brief main.cpp

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/

#include <iostream>

#include <QApplication>

#include "io/error.h"
#include "gui/mainwindow.h"
#include "gui/qobjectinspector.h"

//#include "tests/testtimeline.h"
//#include "tests/testio.h"

class MoApplication : public QApplication
{
public:
    MoApplication(int& argc, char** args)
        :   QApplication(argc, args)
    { }

    virtual bool notify(QObject * o, QEvent * e)
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
};

int main(int argc, char *argv[])
{
    //MO::TestTimeline t; return t.run();
    //TestIo t; return t.run();

    MoApplication a(argc, argv);

    MO::GUI::MainWindow w;
    w.show();

    //MO::GUI::QObjectInspector oi(&w);
    //oi.show();

    try
    {
        //throw MO::BasicException() << "hallo";
        //throw MO::IoException() << "hallo";
        //MO_IO_ERROR(VERSION_MISMATCH, "w " << &w);
        return a.exec();
    }
    /*catch (MO::IoException& e)
    {
        std::cout << "IoException '" << e.what() << "'" << std::endl;
        throw;
    }*/
    catch (MO::Exception& e)
    {
        std::cout << "Exception '" << e.what() << "'" << std::endl;
        throw;
    }
    catch (std::exception& e)
    {
        std::cout << "std::exception '" << e.what() << "'" << std::endl;
        throw;
    }

}
