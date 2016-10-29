/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/19/2016</p>
*/

#include <sstream>
#include <cmath>

#include <QJsonObject>
#include <QJsonArray>

#include "FloatMatrix.h"
#include "tool/ProgressInfo.h"
#include "io/log.h"

namespace MO {

template <typename F>
QJsonObject FloatMatrixT<F>::toJson() const
{
    QJsonObject main;

    // dimensions
    main.insert("dims", JSON::toArray(p_dims_));
    // data
    main.insert("data", JSON::toArray(p_data_));

    return main;
}

template <typename F>
void FloatMatrixT<F>::fromJson(const QJsonObject& main)
{
    std::vector<size_t> dims;
    std::vector<F> data;

    JSON::fromArray(dims, JSON::expectChild(main, "dims"));
    JSON::fromArray(data, JSON::expectChild(main, "data"));

    size_t sum = 1;
    for (auto s : dims)
        sum *= s;
    if (data.size() != sum)
        MO_IO_ERROR(PARSE, "Illegal data in json matrix, expected "
                    << sum << " data points, got " << data.size());

    setDimensions(dims);
    p_data_ = data;
}

template <typename F>
std::string FloatMatrixT<F>::layoutString() const
{
    if (p_dims_.empty())
        return "empty";
    std::stringstream s; s << "(";
    for (size_t i=0; i<p_dims_.size(); ++i)
    {
        if (i > 0)
            s << "x";
        s << p_dims_[i];
    }
    s << ")";
    if (p_dims_.size() > 1)
        s << "=" << p_data_.size();
    return s.str();
}

template <typename F>
std::string FloatMatrixT<F>::rangeString() const
{
    if (p_data_.empty())
        return "0";
    F mi = p_data_.front(), ma = mi;
    for (const auto& f : p_data_)
        mi = std::min(mi, f), ma = std::max(ma, f);
    std::stringstream s;
    s << mi << " - " << ma;
    return s.str();
}

template <typename F>
size_t FloatMatrixT<F>::width() const
{
    return isEmpty() ? 0
                     : size(numDimensions()-1);
}

template <typename F>
size_t FloatMatrixT<F>::height() const
{
    return numDimensions() < 2 ? 1
                               : size(numDimensions()-2);
}

template <typename F>
size_t FloatMatrixT<F>::depth() const
{
    return numDimensions() < 3 ? 1
                               : size(numDimensions()-3);
}

template <typename F>
template <typename T>
FloatMatrixT<T> FloatMatrixT<F>::toType() const
{
    FloatMatrixT<T> m(dimensions());
    m.p_data_.clear();
    for (const auto& f : p_data_)
        m.p_data_.push_back(f);
    return m;
}

template <typename F>
void FloatMatrixT<F>::setDimensions(const std::vector<size_t>& dimensions)
{
    p_dims_ = dimensions;
    p_offs_.clear();
    if (p_dims_.empty())
    {
        p_data_.clear();
        return;
    }
    // clamp size to 1
    for (auto& s : p_dims_)
        s = std::max(size_t(1), s);
    // calculate space required
    size_t num = 1;
    p_offs_.push_back(0);
    for (auto d : p_dims_)
    {
        num *= d;
        p_offs_.push_back(num);
    }
    p_data_.resize(num);
}

template <typename F>
void FloatMatrixT<F>::minimizeDimensions()
{
    if (numDimensions() < 2)
        return;

    while (p_dims_.size() > 1 && p_dims_.front() <= 1)
        p_dims_.erase(p_dims_.begin());

    setDimensions(p_dims_);
}


template <typename F>
FloatMatrixT<F> FloatMatrixT<F>::transposedXY() const
{
    if (numDimensions() == 0)
        return *this;
    if (numDimensions() == 1)
    {
        FloatMatrixT<F> m({ size(0), 1 });
        for (size_t i=0; i<size(0); ++i)
            m(i, 0) = (*this)(i);
        return m;
    }
    if (numDimensions() == 2)
    {
        FloatMatrixT<F> m({ size(1), size(0) });
        for (size_t y=0; y<size(0); ++y)
        for (size_t x=0; x<size(1); ++x)
            m(x, y) = (*this)(y, x);
        m.minimizeDimensions();
        return m;
    }
    if (numDimensions() == 3)
    {
        FloatMatrixT<F> m({ size(0), size(2), size(1) });
        for (size_t z=0; z<size(0); ++z)
        for (size_t y=0; y<size(1); ++y)
        for (size_t x=0; x<size(2); ++x)
            m(z, x, y) = (*this)(z, y, x);
        m.minimizeDimensions();
        return m;
    }
    MO_ASSERT(false, "transposedXY not implemented for matrix "
              << layoutString());
    return *this;
}

template <typename F>
FloatMatrixT<F> FloatMatrixT<F>::rotatedRight() const
{
    if (numDimensions() == 0)
        return *this;
    if (numDimensions() == 1)
    {
        FloatMatrixT<F> m({ size(0), 1 });
        for (size_t i=0; i<size(0); ++i)
            m(i, 0) = (*this)(i);
        return m;
    }
    if (numDimensions() == 2)
    {
        FloatMatrixT<F> m({ size(1), size(0) });
        for (size_t y=0; y<size(0); ++y)
        for (size_t x=0; x<size(1); ++x)
            m(x, y) = (*this)(size(0)-1-y, x);
        m.minimizeDimensions();
        return m;
    }
    if (numDimensions() == 3)
    {
        FloatMatrixT<F> m({ size(0), size(2), size(1) });
        for (size_t z=0; z<size(0); ++z)
        for (size_t y=0; y<size(1); ++y)
        for (size_t x=0; x<size(2); ++x)
            m(z, x, y) = (*this)(z, size(1)-1-y, x);
        m.minimizeDimensions();
        return m;
    }
    MO_ASSERT(false, "rotateRight not implemented for matrix "
              << layoutString());
    return *this;
}



// ###################### distance field ##############################

namespace {

template <typename F>
struct SdfIndex
{
    int x,y,z;
    F d;
    bool operator<(const SdfIndex& o) const { return d < o.d; }
};

/** Creates lists of SdfIndex (for one quadrant/octant)
    sorted by distance to origin,
    separate for each distance >=1 step. */
template <typename F>
void calcSdfIndex(std::vector<std::vector<SdfIndex<F>>>& indices,
                  const FloatMatrixT<F>& src)
{
    indices.clear();

    // generate distance-to-point list
    std::vector<SdfIndex<F>> all;
    switch (src.numDimensions())
    {
        case 2:
        {
            const int W = src.size(1);
            const int H = src.size(0);

            for (int y=0; y<H; ++y)
            for (int x=0; x<W; ++x)
            if (!(x==0 && y==0))
            {
                SdfIndex<F> idx;
                idx.d = std::sqrt(F(x*x+y*y));
                idx.x = x; idx.y = y; idx.z = 0;
                all.push_back(idx);
            }
        }
        break;

        case 3:
        {
            const int W = src.size(2);
            const int H = src.size(1);
            const int D = src.size(0);

            for (int z=0; z<D; ++z)
            for (int y=0; y<H; ++y)
            for (int x=0; x<W; ++x)
            if (!(x==0 && y==0 && z==0))
            {
                SdfIndex<F> idx;
                idx.d = std::sqrt(F(x*x+y*y+z*z));
                idx.x = x; idx.y = y; idx.z = z;
                all.push_back(idx);
            }
        }
        break;
    }

    if (all.empty())
        return;

    std::sort(all.begin(), all.end());

    // split 'all' into separate lists
    // delimited by a distance >= 1.
    std::vector<SdfIndex<F>> idxs;
    F curD = 0.;
    for (const SdfIndex<F>& i : all)
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

/** Exact (slow) one-dimensional signed distance field */
template <typename F>
void calcSdfExact1(FloatMatrixT<F>& dst, const FloatMatrixT<F>& src,
                   ProgressInfo* progress)
{
    dst.setDimensions(src.dimensions());

    const int W = src.size(0);
    if (progress)
        progress->setNumItems(W);

    for (int x=0; x<W; ++x)
    {
        if (progress && (x % 100 == 0))
        {
            progress->setProgress(x);
            progress->send();
        }
        int d = W;
        for (int x1=0; x1<W; ++x1)
            if (*src.data(x1) > 0)
                d = std::min(d, std::abs(x-x1));

        *dst.data(x) = d;
    }
}

/** Exact (slow) two-dimensional signed distance field */
template <typename F>
void calcSdfExact2(FloatMatrixT<F>& dst, const FloatMatrixT<F>& src,
                   ProgressInfo* progress)
{
    dst.setDimensions(src.dimensions());

    const int W = src.size(1);
    const int H = src.size(0);
    const F maxD = std::sqrt(F(W*W+H*H));

    if (progress)
        progress->setNumItems(W*H);

    std::vector<std::vector<SdfIndex<F>>> indices;
    calcSdfIndex(indices, src);

    auto ii = indices.cbegin();

    typedef typename std::vector<std::vector<SdfIndex<F>>>::const_iterator Iter;
    std::vector<Iter> iiY;
    for (int i=0; i<H; ++i)
        iiY.push_back(ii);

    for (int y1=0; y1<H; ++y1)
    {
        if (progress)
        {
            progress->setProgress(y1*W);
            progress->send();
        }

        ii = iiY[y1];
        for (int x1=0; x1<W; ++x1)
        {
            bool inside = *src.data(y1, x1) > 0;
            F d = maxD;

            if (ii != indices.cbegin())
                --ii;

            for (; ii != indices.cend(); ++ii)
            {
            #define MO__TEST(op1__, op2__) \
                for (const SdfIndex<F>& i : *ii) \
                { \
                    int x = x1 op1__ i.x, \
                        y = y1 op2__ i.y; \
                    if (x >= 0 && y >= 0 && x < W && y < H) \
                    if ((*src.data(y, x) > 0) != inside) \
                    { \
                        d = inside ? (-i.d+1.) : i.d; \
                        goto index_found; \
                    } \
                }
                MO__TEST(-, -);
                MO__TEST(-, +);
                MO__TEST(+, -);
                MO__TEST(+, +);
            #undef MO__TEST
            }
        index_found:
            *dst.data(y1, x1) = d;
            iiY[y1] = ii;
        }
    }
}

/** Exact (slow) three-dimensional signed distance field */
template <typename F>
void calcSdfExact3(FloatMatrixT<F>& dst, const FloatMatrixT<F>& src,
                   ProgressInfo* progress)
{
    dst.setDimensions(src.dimensions());

    const int W = src.size(2);
    const int H = src.size(1);
    const int D = src.size(0);
    const F maxD = std::sqrt(F(W*W+H*H+D*D));

    if (progress)
        progress->setNumItems(W*H*D);

    std::vector<std::vector<SdfIndex<F>>> indices;
    calcSdfIndex(indices, src);

    auto ii = indices.cbegin();

    typedef typename std::vector<std::vector<SdfIndex<F>>>::const_iterator Iter;
    std::vector<Iter> iiY;
    for (int i=0; i<W*H; ++i)
        iiY.push_back(ii);

    for (int z1=0; z1<D; ++z1)
    {
        if (progress)
        {
            progress->setProgress(z1*H*W);
            progress->send();
        }

        for (int y1=0; y1<H; ++y1)
        {
            ii = iiY[z1*H+y1];
            for (int x1=0; x1<W; ++x1)
            {
                bool inside = *src.data(z1, y1, x1) > 0;
                F d = maxD;

                if (ii != indices.cbegin())
                    --ii;
                for (; ii != indices.cend(); ++ii)
                {
                #define MO__TEST(op1__, op2__, op3__) \
                    for (const SdfIndex<F>& i : *ii) \
                    { \
                        int x = x1 op1__ i.x, \
                            y = y1 op2__ i.y, \
                            z = z1 op3__ i.z; \
                        if (x >= 0 && y >= 0 && z >= 0 && \
                            x < W && y < H && z < D) \
                        if ((*src.data(z, y, x) > 0) != inside) \
                        { \
                            d = inside ? (-i.d+1.) : i.d; \
                            goto index_found; \
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
            index_found:
                *dst.data(z1, y1, x1) = d;
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

/** Approximated two-dimensional signed distance field
    using "Dead Reckoning" */
template <typename F>
void calcSdfDr2(FloatMatrixT<F>& dst, const FloatMatrixT<F>& src,
                ProgressInfo* progress)
{
    dst.setDimensions(src.dimensions());

    const int W = src.size(1);
    const int H = src.size(0);
    const F maxD = std::sqrt(F(W*W+H*H));

    struct Coord
    {
        Coord() : x(0), y(0) { }
        Coord(int x, int y) : x(x), y(y) { }
        F dist(int x1, int y1)
            { x1-=x; y1-=y; return std::sqrt(F(x1*x1+y1*y1)); }
        int x, y;
    };
    std::vector<Coord> coords(src.size());

    // init
    for (int y=0; y<H; ++y)
    for (int x=0; x<W; ++x)
    {
        dst(y, x) = maxD;

        // detect border
        F v = src(y, x);
        //if (v > 0.)
        if ( (x>0 && src(y,x-1) != v)
          || (y>0 && src(y-1,x) != v)
          || (x+1<W && src(y,x+1) != v)
          || (y+1<H && src(y+1,x) != v)
             )
        {
            dst(y, x) = F(0);
            coords[y*W+x] = Coord(x,y);
        }
    }

    const F d1 = F(1), d2 = std::sqrt(F(2));

    int numIter = 10;

    if (progress)
        progress->setNumItems(numIter);

    int prevNumChanged2=-1, prevNumChanged1=-1, numChanged;
    for (int iter=0; iter<numIter; ++iter)
    {
        if (progress)
        {
            progress->setProgress(iter);
            progress->send();
        }

        numChanged = 0;

    #define MO__TEST(x__, y__, di__) \
        if (dst((y__), (x__)) + di__ < d) \
        { \
            coords[ofs] = coords[(y__)*W + (x__)]; \
            dst(y, x) = coords[ofs].dist(x, y); \
            ++numChanged; \
        }

        // forward pass
        for (int y=0; y<H; ++y)
        for (int x=0; x<W; ++x)
        {
            size_t ofs = y*W+x;
            const F d = dst(y, x);

            if (y > 0)
            {
                if (x > 0)
                {
                    MO__TEST(x-1, y-1, d2);
                }
                MO__TEST(x, y-1, d1);
                if (x < W-1)
                {
                    MO__TEST(x+1, y-1, d2);
                }
            }
            if (x > 0)
            {
                MO__TEST(x-1, y, d1);
            }
        }

        // backward pass
        for (int y=H-1; y>=0; --y)
        for (int x=W-1; x>=0; --x)
        {
            size_t ofs = y*W+x;
            const F d = dst(y, x);

            if (x < W-1)
            {
                MO__TEST(x+1, y, d1);
            }
            if (y < H-1)
            {
                if (x > 0)
                {
                    MO__TEST(x-1, y+1, d2);
                }
                MO__TEST(x, y+1, d1);
                if (x < W-1)
                {
                    MO__TEST(x+1, y+1, d2);
                }
            }
        }
    #undef MO__TEST

        // quit if no significant change
        if (numChanged == prevNumChanged2)
            break;
        prevNumChanged2 = prevNumChanged1;
        prevNumChanged1 = numChanged;

        if (iter > numIter*3/4)
        {
            numIter += 10;
            if (progress)
                progress->setNumItems(numIter);
        }
    }

    // set sign
    for (int y=0; y<H; ++y)
    for (int x=0; x<W; ++x)
    {
        if (src(y,x) > 0.)
            dst(y,x) *= F(-1);
    }
}


/** Approximated three-dimensional signed distance field
    using "Dead Reckoning" */
template <typename F>
void calcSdfDr3(FloatMatrixT<F>& dst, const FloatMatrixT<F>& src,
                ProgressInfo* progress)
{
    dst.setDimensions(src.dimensions());

    const int W = src.size(2);
    const int H = src.size(1);
    const int D = src.size(0);
    const F maxD = std::sqrt(F(W*W+H*H+D*D));

    struct Coord
    {
        Coord() : x(0), y(0), z(0) { }
        Coord(int x, int y, int z) : x(x), y(y), z(z) { }
        F dist(int x1, int y1, int z1)
            { x1-=x; y1-=y; z1-=z; return std::sqrt(F(x1*x1+y1*y1+z1*z1)); }
        int x, y, z;
    };
    std::vector<Coord> coords(src.size());
    std::vector<size_t> prevOfs(dst.size());

    // init
    for (int z=0; z<D; ++z)
    for (int y=0; y<H; ++y)
    for (int x=0; x<W; ++x)
    {
        dst(z, y, x) = maxD;

        // detect border
        const F diff = 0.01;
        F v = src(z, y, x);
        //if (v > 0.)
        if ( (x>0   && std::abs(src(z,  y,  x-1) - v) > diff)
          || (y>0   && std::abs(src(z,  y-1,x  ) - v) > diff)
          || (z>0   && std::abs(src(z-1,y,  x  ) - v) > diff)
          || (x+1<W && std::abs(src(z,  y,  x+1) - v) > diff)
          || (y+1<H && std::abs(src(z,  y+1,x  ) - v) > diff)
          || (z+1<D && std::abs(src(z+1,y,  x  ) - v) > diff)
             )
        {
            dst(z, y, x) = F(0);
            coords[(z*H+y)*W+x] = Coord(x,y,z);
        }
    }

    //const F d1 = F(1), d2 = std::sqrt(F(2)), d3 = std::sqrt(F(3));

    int numIter = 10;

    if (progress)
        progress->setNumItems(numIter);

    int prevNumChanged2=-1, prevNumChanged1=-1, numChanged;
    for (int iter=0; iter<numIter; ++iter)
    {
        if (progress)
        {
            progress->setProgress(iter);
            progress->send();
        }

        numChanged = 0;

    #define MO__TEST(x__, y__, z__) \
        if ((x+(x__))>0 && (x+(x__))<W && (y+(y__))>0 && (y+(y__))<H \
            && (z+(z__))>0 && (z+(z__))<D) \
        if (dst(z+(z__), y+(y__), x+(x__)) \
            + std::sqrt(F((x__)*(x__)+(y__)*(y__)+(z__)*(z__))) < d) \
        { \
            ++numChanged; \
            size_t ofs2 = ((z+(z__))*H + (y+(y__)))*W + (x+(x__)); \
            coords[ofs] = coords[ofs2]; \
            dst(z, y, x) = coords[ofs].dist(x, y, z); \
        }

        // forward pass
        for (int z=0; z<D; ++z)
        for (int y=0; y<H; ++y)
        for (int x=0; x<W; ++x)
        {
            size_t ofs = (z*H+y)*W+x;
            const F d = dst(z, y, x);

            MO__TEST(-1, -1, -1);
            MO__TEST( 0, -1, -1);
            MO__TEST(+1, -1, -1);
            MO__TEST(-1,  0, -1);
            MO__TEST(+1,  0, -1);
            MO__TEST(-1, +1, -1);
            MO__TEST( 0, +1, -1);
            MO__TEST(+1, +1, -1);

            MO__TEST(-1, -1,  0);
            MO__TEST( 0, -1,  0);
            MO__TEST(+1, -1,  0);
            MO__TEST(-1,  0,  0);

            MO__TEST(-1, -1, +1);
        }

        // backward pass
        for (int z=D-1; z>=0; --z)
        for (int y=H-1; y>=0; --y)
        for (int x=W-1; x>=0; --x)
        {
            size_t ofs = (z*H+y)*W+x;
            const F d = dst(z, y, x);

            MO__TEST(+1, +1, -1);

            MO__TEST(+1,  0,  0);
            MO__TEST(-1, +1,  0);
            MO__TEST( 0, +1,  0);
            MO__TEST(+1, +1,  0);

            MO__TEST(-1, -1, +1);
            MO__TEST( 0, -1, +1);
            MO__TEST(+1, -1, +1);
            MO__TEST(-1,  0, +1);
            MO__TEST(+1,  0, +1);
            MO__TEST(-1, +1, +1);
            MO__TEST( 0, +1, +1);
            MO__TEST(+1, +1, +1);
        }
#undef MO__TEST

        // quit if no significant change
        if (numChanged == prevNumChanged2)
            break;
        prevNumChanged2 = prevNumChanged1;
        prevNumChanged1 = numChanged;

        if (iter > numIter*3/4)
        {
            numIter += 10;
            if (progress)
                progress->setNumItems(numIter);
        }

    }

    // set sign
    for (int z=0; z<D; ++z)
    for (int y=0; y<H; ++y)
    for (int x=0; x<W; ++x)
    {
        if (src(z,y,x) > 0.)
            dst(z,y,x) *= F(-1);
    }
}



} // namespace {

template <typename F>
bool FloatMatrixT<F>::calcDistanceField(
        const FloatMatrixT<F>& src, DFMode mode, ProgressInfo* pinfo)
{
    bool handled = false;
    switch (mode)
    {
        case DF_EXACT:
            handled = true;
            switch (src.numDimensions())
            {
                case 1: calcSdfExact1(*this, src, pinfo); break;
                case 2: calcSdfExact2(*this, src, pinfo); break;
                case 3: calcSdfExact3(*this, src, pinfo); break;
                default: MO_WARNING("Exact distance field for matrix "
                                    << src.layoutString() << " not supported");
                    handled = false;
                break;
            }
        break;

        case DF_FAST:
            handled = true;
            switch (src.numDimensions())
            {
                //case 1: calcSdfDr1(*this, src, pinfo); break;
                case 2: calcSdfDr2(*this, src, pinfo); break;
                case 3: calcSdfDr3(*this, src, pinfo); break;
                default: MO_WARNING("Fast distance field for matrix "
                                    << src.layoutString() << " not supported");
                    handled = false;
                break;
            }
        break;
    }

    if (pinfo)
    {
        pinfo->setFinished();
        pinfo->send();
    }

    return handled;
}







template class FloatMatrixT<float>;
template class FloatMatrixT<double>;


} // namespace MO
