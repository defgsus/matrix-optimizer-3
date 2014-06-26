/** @file testtimeline.cpp

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 4/28/2014</p>
*/

#include <iostream>
#include <iomanip>

#include <QTime>

#include "testtimeline.h"

namespace MO {


bool TestTimeline::run()
{
    for (int i=0; i<500000; ++i)
    {
        writepos.push_back( (double)rand()/RAND_MAX * 1000.);
        readpos.push_back(  (double)rand()/RAND_MAX * 1000.);
        values.push_back(   (double)rand()/RAND_MAX * 2. - 1.);
    }

    bool r = true;
    for (int i=1; i<MATH::Timeline1D::Point::MAX; ++i)
        r |= test((MATH::Timeline1D::Point::Type)i);
    return r;
}

bool TestTimeline::test(MATH::Timeline1D::Point::Type pointType)
{
    const int num = values.size();

    MATH::Timeline1D tl;

    // write

    /*std::cout << "adding " << a << " random values (type "
              << Timeline1D::Point::getName(pointType) << ")"
              << std::endl;*/
    std::cout << "num-points " << num << ", type "
              << MATH::Timeline1D::Point::getName(pointType) << std::endl;

    QTime m;
    m.start();

    for (int i=0; i<num; ++i)
    {
        tl.add(writepos[i], values[i], pointType);
    }

    int e1 = m.elapsed();

    // read

    /*std::cout << "reading " << a << " random positions (type "
              << Timeline1D::Point::getName(pointType) << ")"
              << std::endl;*/

    m.start();

    double v=0;
    for (int i=0; i<num; ++i)
    {
        v += tl.get(readpos[i]);
    }

    int e2 = m.elapsed();

    std::cout << "adding:  " << std::setw(8) << ((double)e1/1000)
              << "sec = " << std::setw(8) << (long int)((double)num/e1) << "k per sec" << std::endl
              << "reading: " << std::setw(8) << ((double)e2/1000)
              << "sec = " << std::setw(8) << (long int)((double)num/e2) << "k per sec" << std::endl;

    return true;
}



} // namespace MO
