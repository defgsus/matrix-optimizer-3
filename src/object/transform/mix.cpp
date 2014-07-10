/** @file mix.cpp

    @brief mixer for contained transformations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/11/2014</p>
*/

#include "mix.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(Mix)

Mix::Mix(QObject *parent) :
    Transformation(parent)
{
    setName("Mix");
}

void Mix::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("mix", 1);
}

void Mix::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("mix", 1);
}


void Mix::createParameters()
{
    m_ = createFloatParameter("mix", "mix", 0);
}

void Mix::childrenChanged()
{
    transformations_ = findChildObjects<Transformation>();
}

void Mix::applyTransformation(Mat4 &matrix, Double time) const
{
    const Double
            m = std::min((Double)1, std::max((Double)0, m_->value(time) )),
            m1 = 1.0 - m;

    Mat4 trans(matrix);

    // apply child transforms
    for (auto t : transformations_)
        t->applyTransformation(trans, time);

    // mix resulting matrix
    matrix = m1 * matrix + m * trans;
}


} // namespace MO
