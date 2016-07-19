/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/8/2016</p>
*/

#include <thread>

#include "TestFloatMatrix.h"
#include "object/param/FloatMatrix.h"

namespace MO {


struct TestFloatMatrix::Private
{
    Private(TestFloatMatrix* p)
        : p         (p)
    { }

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

    return 0;
}

} // namespace MO
