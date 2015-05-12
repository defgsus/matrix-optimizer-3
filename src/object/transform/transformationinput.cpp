/** @file transformationinput.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#include "transformationinput.h"
#include "object/param/parameters.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertransformation.h"
#include "io/datastream.h"


namespace MO {

MO_REGISTER_OBJECT(TransformationInput)

TransformationInput::TransformationInput(QObject *parent) :
    Transformation(parent)
{
    setName("Input");
}

void TransformationInput::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("transin", 1);
}

void TransformationInput::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("transin", 1);
}


void TransformationInput::createParameters()
{
    Transformation::createParameters();

    params()->beginParameterGroup("trans", tr("transformation"));
    initParameterGroupExpanded("trans");

        p_trans_ = params()->createTransformationParameter(
                    "trans", "input", tr("Connect another object here to get it's transformation"));

        p_apply_ = params()->createSelectParameter(
                    "apply", tr("application"),
                    tr("Determines whether the incoming matrix replaces or transforms the current matrix"),
                    { "r", "lm", "rm" },
                    { tr("replace"), tr("left multiply"), tr("right multiply") },
                    { tr("The incoming matrix replaces the current matrix"),
                      tr("The current matrix is multiplied with the incoming matrix"),
                      tr("The current matrix is multiplied with the incoming matrix") },
                    { 0, 1, 2 },
                    0,
                    true, false);

    params()->endParameterGroup();
}

void TransformationInput::applyTransformation(Mat4 &matrix, Double time, uint thread) const
{
    const Mat4 m = p_trans_->value(time, thread);

    switch (p_apply_->baseValue())
    {
        default:
        case 0: matrix = m; break;
        case 1: matrix = m * matrix; break;
        case 2: matrix = matrix * m; break;
    }
}


} // namespace MO
