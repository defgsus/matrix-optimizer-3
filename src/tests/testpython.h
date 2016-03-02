/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/2/2016</p>
*/

#ifndef MOSRC_TESTS_TESTPYTHON_H
#define MOSRC_TESTS_TESTPYTHON_H


class TestPython
{
public:
    TestPython();
    ~TestPython();

    int run();

private:
    struct Private;
    Private * p_;
};

#endif // TESTPYTHON_H
