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

    struct Idx
    {
        int x,y,z;
        Double d;
        bool operator<(const Idx& o) const { return d < o.d; }
    };

    void recalcIndex();
    void recalc(const RenderTime& time);

    DistanceFieldDO* p;
    FloatMatrix dist;

    bool doRecalc;
    /** Lists of Idx (for one quadrant) sorted by distance to origin,
        separated for each distance >=1 step. */
    std::vector<std::vector<Idx>> indices;

    ParameterFloatMatrix* p_input;
    ParameterFloat* p_thresh;
    ParameterSelect* p_inside;
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

        p_->p_inside = params()->createSelectParameter(
            "inside", tr("inside"),
            tr("What values determine the inside of the implicit object"),
        { "above", "below" },
        { tr("above threshold"), tr("below threshold") },
        { tr("Values >= threshold are inside"),
          tr("Values <= threshold are inside") },
        { IN_ABOVE, IN_BELOW },
        IN_ABOVE, true, false);

        p_->p_thresh = params()->createFloatParameter(
            "threshold", tr("surface threshold"),
            tr("The value in the input below which is inside"),
            0.0, 0.1, true, false);

    params()->endParameterGroup();
}

void DistanceFieldDO::onParameterChanged(Parameter* p)
{
    Object::onParameterChanged(p);

    if (p == p_->p_input
     || p == p_->p_inside
     || p == p_->p_thresh
            )
        p_->doRecalc = true;
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


void DistanceFieldDO::Private::recalcIndex()
{
    MO_DEBUG_FM(p->idName() + "::recalcIndex()");

    indices.clear();

    // generate distance-to-point list
    std::vector<Idx> all;
    switch (dist.numDimensions())
    {
        case 2:
        {
            const int W = dist.size(1);
            const int H = dist.size(0);

            for (int y=0; y<H; ++y)
            for (int x=0; x<W; ++x)
            if (!(x==0 && y==0))
            {
                Idx idx;
                idx.d = std::sqrt(Double(x*x+y*y));
                idx.x = x; idx.y = y; idx.z = 0;
                all.push_back(idx);
            }
        }
        break;

        case 3:
        {
            const int W = dist.size(2);
            const int H = dist.size(1);
            const int D = dist.size(0);

            for (int z=0; z<D; ++z) //{ MO_DEBUG_FM(z << "/" << D);
            for (int y=0; y<H; ++y)
            for (int x=0; x<W; ++x)
            if (!(x==0 && y==0 && z==0))
            {
                Idx idx;
                idx.d = std::sqrt(Double(x*x+y*y+z*z));
                idx.x = x; idx.y = y; idx.z = z;
                all.push_back(idx);
            } //}
        }
        break;
    }

    MO_DEBUG_FM("sorting");
    std::sort(all.begin(), all.end());

    if (all.empty())
        return;

    std::vector<Idx> idxs;
    Double curD = 0.;
    for (const Idx& i : all)
    {
        if (i.d >= curD)
        {
            indices.push_back(idxs);
            idxs.clear();
            curD = i.d + 1.;
        }
        idxs.push_back(i);
    }
}

void DistanceFieldDO::Private::recalc(const RenderTime& time)
{
    p->clearError();

    auto matrix = p_input->value(time);
    MO_DEBUG_FM(p->idName() << ")::recalc(" << time << ")"
                << matrix.layoutString());

    // recalc distance indices
    if (!dist.hasDimensions(matrix.dimensions()))
    {
        dist.setDimensionsFrom(matrix);
        recalcIndex();
    }

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
    }

    MO_DEBUG_FM("calcing distance ..");

    switch (dist.numDimensions())
    {
        default:
            p->setErrorMessage(tr("%1-dimensional matrix not supported")
                               .arg(dist.numDimensions()));
            return;
        break;

        case 1:
        {
            const int W = matrix.size(0);
            for (int x=0; x<W; ++x)
            {
                int d = W;
                for (int x1=0; x1<W; ++x1)
                    if (*matrix.data(x1) > 0)
                        d = std::min(d, std::abs(x-x1));

                *dist.data(x) = d;
            }
        }
        break;

        /*
         . . . . . . . .
         . . . . . . . .
         . x x x . . . .
         . . . x . . . .
         . . . . . . . .
        case 2:
        {
            const int W = matrix.size(1);
            const int H = matrix.size(0);
            for (int y=0; y<H; ++y)
            for (int x=0; x<W; ++x)
            {
                int d = W*W+H*H;
                for (int y1=0; y1<H; ++y1)
                for (int x1=0; x1<W; ++x1)
                    if (*matrix.data(y1, x1) > 0)
                        d = std::min(d, (x-x1)*(x-x1) + (y-y1)*(y-y1));

                *dist.data(x) = std::sqrt(Double(d));
            }
        }
        break;
        */
        case 2:
        {
            const int W = matrix.size(1);
            const int H = matrix.size(0);
            const Double maxD = std::sqrt(Double(W*W+H*H));

            progress.setNumItems(W*H);

            auto ii = indices.cbegin();

            std::vector<std::vector<std::vector<Idx>>::const_iterator> iiY;
            for (int i=0; i<H; ++i)
                iiY.push_back(ii);

            for (int y1=0; y1<H; ++y1)
            {
                progress.setProgress(y1*W);
                progress.send();

                ii = iiY[y1];
                for (int x1=0; x1<W; ++x1)
                {
                    bool inside = *matrix.data(y1, x1) > 0;
                    Double d = maxD;

                    if (ii != indices.cbegin())
                        --ii;

                    for (; ii != indices.cend(); ++ii)
                    {
                    #define MO__TEST(op1__, op2__) \
                        for (const Idx& i : *ii) \
                        { \
                            int x = x1 op1__ i.x, \
                                y = y1 op2__ i.y; \
                            if (x >= 0 && y >= 0 && x < W && y < H) \
                            if ((*matrix.data(y, x) > 0) != inside) \
                            { \
                                d = inside ? -i.d : i.d; \
                                goto index_found2; \
                            } \
                        }
                        MO__TEST(-, -);
                        MO__TEST(-, +);
                        MO__TEST(+, -);
                        MO__TEST(+, +);
                    #undef MO__TEST
                    }
                index_found2:
                    *dist.data(y1, x1) = d;
                    iiY[y1] = ii;
                }
            }
        }
        break;

        case 3:
        {
            const int W = matrix.size(2);
            const int H = matrix.size(1);
            const int D = matrix.size(0);
            const Double maxD = std::sqrt(Double(W*W+H*H+D*D));

            progress.setNumItems(W*H*D);

            auto ii = indices.cbegin();
            std::vector<std::vector<std::vector<Idx>>::const_iterator> iiY;
            for (int i=0; i<W*H; ++i)
                iiY.push_back(ii);

            for (int z1=0; z1<D; ++z1)
            {
                progress.setProgress(z1*H*W);
                progress.send();

                for (int y1=0; y1<H; ++y1)
                {
                    ii = iiY[z1*H+y1];
                    for (int x1=0; x1<W; ++x1)
                    {
                        bool inside = *matrix.data(z1, y1, x1) > 0;
                        Double d = maxD;

                        if (ii != indices.cbegin())
                            --ii;
                        for (; ii != indices.cend(); ++ii)
                        {
                        #define MO__TEST(op1__, op2__, op3__) \
                            for (const Idx& i : *ii) \
                            { \
                                int x = x1 op1__ i.x, \
                                    y = y1 op2__ i.y, \
                                    z = z1 op3__ i.z; \
                                if (x >= 0 && y >= 0 && z >= 0 && \
                                    x < W && y < H && z < D) \
                                if ((*matrix.data(z, y, x) > 0) != inside) \
                                { \
                                    d = inside ? -i.d : i.d; \
                                    goto end_loop; \
                                } \
                            }
                            MO__TEST(-, -, -);
                            MO__TEST(-, -, +);
                            MO__TEST(-, +, -);
                            MO__TEST(-, +, +);
                            MO__TEST(+, -, -);
                            MO__TEST(+, +, +);
                        #undef MO__TEST
                        }
                    end_loop:
                        *dist.data(z1, y1, x1) = d;
                        /*MO_DEBUG_FM("search "
                                    << int(iiY[z1*H+y1]-indices.cbegin())
                                    << "/" << indices.size()
                                    << " - " << int(ii-indices.cbegin()));
                        */
                        iiY[z1*H+y1] = ii;
                        if (y1+1 < H)
                        {
                            iiY[z1*H+y1+1] = ii;
                            if (z1+1 < D)
                                iiY[(z1+1)*H+y1+1] = ii;
                        }
                        if (z1+1 < D)
                            iiY[(z1+1)*H+y1] = ii;
                    }
                }
            }
        }
        break;

    }

    progress.setFinished();
    progress.send();
}


} // namespace MO
