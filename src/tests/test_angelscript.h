/** @file test_angelscript.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.01.2015</p>
*/

#ifndef MOSRC_TESTS_TEST_ANGELSCRIPT_H
#define MOSRC_TESTS_TEST_ANGELSCRIPT_H

namespace MO {

class TestAngelScript
{
public:

    TestAngelScript();
    ~TestAngelScript();

    int run();

private:
    class Private;
    Private * p_;
};

} // namespace MO

#endif // TEST_ANGELSCRIPT_H
