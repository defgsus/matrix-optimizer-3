/** @file mix.cpp

    @brief mixer for contained transformations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/11/2014</p>
*/

#include "Mix.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "io/DataStream.h"

namespace MO {

MO_REGISTER_OBJECT(Mix)

Mix::Mix()
    : Transformation()
{
    setName("Mix");
}

Mix::~Mix() { }

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

    params()->beginParameterGroup("mixmix", tr("mix"));
    initParameterGroupExpanded("mixmix");

        m_ = params()->createFloatParameter("mix", "mix",
                              tr("The contained transformations will be mixed-in by this value, range 0-1")
                              , 1, 0, 1, 0.05);

    params()->endParameterGroup();
}

void Mix::childrenChanged()
{
    transformations_ = findChildObjects<Transformation>();
}

void Mix::applyTransformation(Mat4 &matrix, const RenderTime& time) const
{
    const Float m = m_->value(time);

    // don't touch matrix at all
    if (m <= 0)
        return;

    Mat4 trans(matrix);

    // apply child transforms
    for (auto t : transformations_)
        if (t->active(time))
            t->applyTransformation(trans, time);

    if (m >= 1.0)

        matrix = trans;

    else
        // mix resulting matrix
        matrix = (1 - m) * matrix + m * trans;
}


} // namespace MO
