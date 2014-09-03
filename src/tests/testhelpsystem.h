/** @file testhelpsystem.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/3/2014</p>
*/

#ifndef MOSRC_TESTS_TESTHELPSYSTEM_H
#define MOSRC_TESTS_TESTHELPSYSTEM_H

namespace MO {
class HelpSystem;

class TestHelpSystem
{
public:
    TestHelpSystem();

    int run();

    HelpSystem * help;
};

} // namespace MO

#endif // MOSRC_TESTS_TESTHELPSYSTEM_H
