/** @file testtimeline.h

    @brief Timeline1D testing

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 4/28/2014</p>
*/

#ifndef MOSRC_TESTS_TESTTIMELINE_H
#define MOSRC_TESTS_TESTTIMELINE_H

#include "math/timeline1d.h"

namespace MO {

class TestTimeline
{
public:

    bool run();

private:

    bool test(MATH::Timeline1d::Point::Type type);

    std::vector<double> writepos, values, readpos;
};

} // namespace MO

#endif // MOSRC_TESTS_TESTTIMELINE_H
