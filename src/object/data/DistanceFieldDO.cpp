/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#include <list>

#include "DistanceFieldDO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloatMatrix.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "math/interpol.h"
#include "tool/ProgressInfo.h"
#include "io/DataStream.h"
#include "io/log_FloatMatrix.h"

namespace MO {

MO_REGISTER_OBJECT(DistanceFieldDO)

struct DistanceFieldDO::Private
{
    Private(DistanceFieldDO* p)
        : p         (p)
        , doRecalc  (true)
    { }


    void recalc(const RenderTime& time);

    DistanceFieldDO* p;
    FloatMatrix dist;

    bool doRecalc;

    ParameterFloatMatrix* p_input;
    ParameterFloat* p_thresh, *p_uthresh;
    ParameterSelect* p_inside, *p_mode;
};

DistanceFieldDO::DistanceFieldDO()
    : Object    ()
    , p_        (new Private(this))
{
    setName("DistanceField");
    setNumberOutputs(ST_FLOAT_MATRIX, 1);
}

DistanceFieldDO::~DistanceFieldDO() { delete p_; }

void DistanceFieldDO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("distfdo", 1);
}

void DistanceFieldDO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("distfdo", 1);
}

void DistanceFieldDO::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("distance", tr("distance"));
    initParameterGroupExpanded("distance");

        p_->p_input = params()->createFloatMatrixParameter(
            "input", tr("input"),
            tr("The input matrix"),
            FloatMatrix(), true, true);
        p_->p_input->setVisibleGraph(true);

        p_->p_mode = params()->createSelectParameter(
            "sdf_mode", tr("mode"),
            tr("The method used for distance field estimation"),
        { "exact", "fast" },
        { tr("exact"), tr("fast") },
        { tr("Exact (slow) evaluation of distance field"),
          tr("Fast approximation using 'Dead Reckoning' method") },
        { FloatMatrix::DF_EXACT, FloatMatrix::DF_FAST },
            FloatMatrix::DF_FAST, true, false);

        p_->p_inside = params()->createSelectParameter(
            "inside", tr("inside"),
            tr("Values that determine the inside of the implicit object"),
        { "above", "below", "between" },
        { tr("above threshold"), tr("below threshold"), tr("between") },
        { tr("Values >= threshold are inside"),
          tr("Values <= threshold are inside"),
          tr("Values >= threshold and <= upper threshold are inside") },
        { IN_ABOVE, IN_BELOW, IN_BETWEEN },
        IN_ABOVE, true, false);

        p_->p_thresh = params()->createFloatParameter(
            "threshold", tr("surface threshold"),
            tr("Input values above/below are inside"),
            0.0, 0.1, true, false);

        p_->p_uthresh = params()->createFloatParameter(
            "threshold2", tr("upper surface threshold"),
            tr("The upper limit for input values which are inside"),
            1.0, 0.1, true, false);

    params()->endParameterGroup();
}

void DistanceFieldDO::onParameterChanged(Parameter* p)
{
    Object::onParameterChanged(p);

    if (p == p_->p_input
     || p == p_->p_mode
     || p == p_->p_inside
     || p == p_->p_thresh
     || p == p_->p_uthresh
           )
        p_->doRecalc = true;
}

void DistanceFieldDO::updateParameterVisibility()
{
    Object::updateParameterVisibility();
    bool between = p_->p_inside->baseValue() == IN_BETWEEN;

    p_->p_uthresh->setVisible(between);
}


FloatMatrix DistanceFieldDO::valueFloatMatrix(uint, const RenderTime& time) const
{
    if (p_->doRecalc || p_->p_input->hasChanged(time))
    {
        p_->recalc(time);
        p_->doRecalc = false;
    }
    return p_->dist;
}

bool DistanceFieldDO::hasFloatMatrixChanged(
        uint , const RenderTime& time) const
{
    return p_->doRecalc || p_->p_input->hasChanged(time);
}



void DistanceFieldDO::Private::recalc(const RenderTime& time)
{
    p->clearError();

    auto matrix = p_input->value(time);
    MO_DEBUG_FM(p->idName() << ")::recalc(" << time << ")"
                << matrix.layoutString());

    ProgressInfo progress(tr("calc distance field"), p);
    progress.send();

    // thresholding input
    const Double thresh = p_thresh->value(time);
    switch (Inside(p_inside->value(time)))
    {
        case IN_ABOVE:
            for (auto& f : matrix)
                f = f >= thresh ? 1. : 0.;
        break;
        case IN_BELOW:
            for (auto& f : matrix)
                f = f <= thresh ? 1. : 0.;
        break;
        case IN_BETWEEN:
        {
            const Double uthresh = p_uthresh->value(time);
            for (auto& f : matrix)
                f = f >= thresh && f <= uthresh ? 1. : 0.;
        }
        break;
    }

    dist.calcDistanceField(
                matrix,
                FloatMatrix::DFMode(p_mode->value(time)),
                &progress);
}


} // namespace MO
