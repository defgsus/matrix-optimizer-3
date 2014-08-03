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
    Transformation::createParameters();

    m_ = createFloatParameter("mix", "mix",
                              tr("The contained transformations will be mixed-in by this value, range 0-1")
                              , 1, 0, 1, 0.05);
}

void Mix::childrenChanged()
{
    transformations_ = findChildObjects<Transformation>();
}

void Mix::applyTransformation(Mat4 &matrix, Double time, uint thread) const
{
    const Double m = m_->value(time, thread);

    // don't touch matrix at all
    if (m <= 0)
        return;

    Mat4 trans(matrix);

    // apply child transforms
    for (auto t : transformations_)
        if (t->active(time, thread))
            t->applyTransformation(trans, time, thread);

    if (m >= 1.0)

        matrix = trans;

    else
        // mix resulting matrix
        matrix = (1 - m) * matrix + m * trans;
}


} // namespace MO
