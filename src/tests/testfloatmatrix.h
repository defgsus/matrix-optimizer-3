/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/8/2016</p>
*/

#ifndef MOSRC_TESTS_TESTFLOATMATRIX_H
#define MOSRC_TESTS_TESTFLOATMATRIX_H

namespace MO {

class TestFloatMatrix
{
public:
    TestFloatMatrix();
    ~TestFloatMatrix();

    int run();

private:
    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_TESTS_TESTFLOATMATRIX_H
