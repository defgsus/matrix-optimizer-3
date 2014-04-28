/** @file

    @brief main.cpp

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/

#include <QApplication>

#include "gui/mainwindow.h"
#include "tests/testtimeline.h"

int main(int argc, char *argv[])
{
    MO::TestTimeline t; return t.run();

    QApplication a(argc, argv);
    MO::GUI::MainWindow w;
    w.show();

    return a.exec();
}
