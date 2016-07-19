/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/21/2016</p>
*/

#ifndef TESTGLWINDOW_H
#define TESTGLWINDOW_H

class TestGlWindow
{
public:
    TestGlWindow();
    ~TestGlWindow();

    int run();

private:
    struct Private;
    Private* p_;
};

#endif // TESTGLWINDOW_H
