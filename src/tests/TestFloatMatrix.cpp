/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/8/2016</p>
*/

#include <thread>

#include "TestFloatMatrix.h"
#include "math/FloatMatrix.h"
#include "io/log.h"
#include "io/error_index.h"

namespace MO {


struct TestFloatMatrix::Private
{
    Private(TestFloatMatrix* p)
        : p         (p)
    { }

    bool testMapping2();
    bool testMapping3();

    TestFloatMatrix* p;
};

TestFloatMatrix::TestFloatMatrix()
    : p_        (new Private(this))
{

}

TestFloatMatrix::~TestFloatMatrix()
{
    delete p_;
}

int TestFloatMatrix::run()
{
    p_->testMapping2();
    p_->testMapping3();
    return 0;
}

#define ASSERT(cond_) \
    if (!(cond_)) { MO_PRINT("FAILED: " << #cond_); return false; }

bool TestFloatMatrix::Private::testMapping2()
{
    FloatMatrix m({3,2});
    ASSERT(m.width() == 2);
    ASSERT(m.height() == 3);

    m(0 * m.width() + 0) = 1.;
    m(0 * m.width() + 1) = 2.;
    m(1 * m.width() + 0) = 3.;
    m(1 * m.width() + 1) = 4.;
    m(2 * m.width() + 0) = 5.;
    m(2 * m.width() + 1) = 6.;

    MO_PRINT( m.toJsonString() );

    ASSERT(m(0 * m.width() + 0) == m(0,0));
    ASSERT(m(0 * m.width() + 1) == m(0,1));
    ASSERT(m(1 * m.width() + 0) == m(1,0));
    ASSERT(m(1 * m.width() + 1) == m(1,1));
    ASSERT(m(2 * m.width() + 0) == m(2,0));
    ASSERT(m(2 * m.width() + 1) == m(2,1));

    return true;
}

bool TestFloatMatrix::Private::testMapping3()
{
    FloatMatrix m({4,3,2});
    ASSERT(m.width() == 2);
    ASSERT(m.height() == 3);
    ASSERT(m.depth() == 4);

    int k=0;
    for (size_t z=0; z<m.depth(); ++z)
    for (size_t y=0; y<m.height(); ++y)
    for (size_t x=0; x<m.width(); ++x)
        m((z * m.height() + y) * m.width() + x) = ++k;

    for (size_t z=0; z<m.depth(); ++z)
    for (size_t y=0; y<m.height(); ++y)
    for (size_t x=0; x<m.width(); ++x)
        ASSERT(m((z * m.height() + y) * m.width() + x) == m(z,y,x));

    return true;
}

} // namespace MO
