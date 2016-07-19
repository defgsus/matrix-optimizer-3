/** @file testequation.h

    @brief Math equation tester

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#ifndef MOSRC_TESTS_TESTEQUATION_H
#define MOSRC_TESTS_TESTEQUATION_H

namespace MO {

class TestEquation
{
public:
    TestEquation();

    int run();

private:

    int compareTests_();
};

} // namespace MO

#endif // MOSRC_TESTS_TESTEQUATION_H
