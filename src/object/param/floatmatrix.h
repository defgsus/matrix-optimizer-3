/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#ifndef MOSRC_OBJECT_PARAM_FLOATMATRIX_H
#define MOSRC_OBJECT_PARAM_FLOATMATRIX_H

#include <vector>
#include <string>
#include <sstream>

#include "types/float.h"
#include "io/error.h"
#include "io/datastream.h"

namespace MO {

/** @brief Multi-dimensional float data container



*/
class FloatMatrix
{
public:
    typedef std::vector<Double> Vector;

    FloatMatrix() { }
    FloatMatrix(const std::vector<size_t>& dimensions) { setDimensions(dimensions); }

    // ---- getter ----

    std::string layoutString() const
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

    /** Number of dimensions */
    size_t dimensions() const { return p_dims_.size(); }
    /** Size of all data */
    size_t size() const { return p_data_.size(); }
    /** Size of one dimension */
    size_t size(size_t dimension) const
        { MO_ASSERT(dimension < p_dims_.size(),
                    "dimension="<<dimension<<", p_dims_="<<p_dims_.size());
          return p_dims_[dimension]; }

    // --- read access ---

    /** Read access to all data */
    const Double* data() const
        { MO_ASSERT(!p_data_.empty(), "");
          return &p_data_[0]; }
    const Double* data(size_t i0) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(i0 < p_data_.size(), "i0="<<i0<< ", layout=" << layoutString());
          return &p_data_[i0]; }
    const Double* data(size_t i0, size_t i1) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(i1*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", layout="<<layoutString());
          return &p_data_[i1*p_dims_[0] + i0]; }
    const Double* data(size_t i0, size_t i1, size_t i2) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT((i2*p_dims_[1] + i1)*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", layout="<<layoutString());
          return &p_data_[(i2*p_dims_[1] + i1)*p_dims_[0] + i0]; }
    const Double* data(size_t i0, size_t i1, size_t i2, size_t i3) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(((i3*p_dims_[2]+i2)*p_dims_[1]+i1)*p_dims_[0]+i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", i3="
                   <<i3<<", layout="<<layoutString());
          return &p_data_[((i3*p_dims_[2] + i2)*p_dims_[1] + i1)*p_dims_[0] + i0]; }

    // --- std container ---

    Vector::const_iterator begin() const { return p_data_.begin(); }
    Vector::const_iterator end() const { return p_data_.end(); }
    Vector::iterator begin() { return p_data_.begin(); }
    Vector::iterator end() { return p_data_.end(); }

    // --- compare ---

    bool operator == (const FloatMatrix& o) const { return !(*this != o); }
    bool operator != (const FloatMatrix& o) const
    {
        if (o.dimensions() != dimensions())
            return true;
        for (size_t i=0; i<dimensions(); ++i)
            if (o.size(i) != size(i))
                return true;
        for (size_t i=0; i<size(); ++i)
            if (o.data()[i] != data()[i])
                return true;
        return false;
    }

    // --- setter ---

    /** Reserve space for each dimension */
    void setDimensions(const std::vector<size_t>& dimensions)
    {
        p_dims_ = dimensions;
        p_offs_.clear();
        if (p_dims_.empty())
        {
            p_data_.clear();
            return;
        }
        size_t num = 1;
        p_offs_.push_back(0);
        for (auto d : p_dims_)
        {
            num *= d;
            p_offs_.push_back(num);
        }
        p_data_.resize(num);
    }

    /** Write access to all data */
    Double* data()
        { MO_ASSERT(!p_data_.empty(), "");
          return &p_data_[0]; }
    Double* data(size_t i0)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(i0 < p_data_.size(), "i0="<<i0<< ", layout=" << layoutString());
          return &p_data_[i0]; }
    Double* data(size_t i0, size_t i1)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(i1*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", layout="<<layoutString());
          return &p_data_[i1*p_dims_[0] + i0]; }
    Double* data(size_t i0, size_t i1, size_t i2)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT((i2*p_dims_[1] + i1)*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", layout="<<layoutString());
          return &p_data_[(i2*p_dims_[1] + i1)*p_dims_[0] + i0]; }
    Double* data(size_t i0, size_t i1, size_t i2, size_t i3)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(((i3*p_dims_[2]+i2)*p_dims_[1]+i1)*p_dims_[0]+i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", i3="
                   <<i3<<", layout="<<layoutString());
          return &p_data_[((i3*p_dims_[2] + i2)*p_dims_[1] + i1)*p_dims_[0] + i0]; }


    // -------- io ----------

    friend IO::DataStream& operator << (IO::DataStream& io, const FloatMatrix& m)
    {
        io.writeHeader("fm", 1);
        io << quint64(m.dimensions());
        for (size_t i=0; i<m.dimensions(); ++i)
            io << quint64(m.size(i));
        for (size_t i=0; i<m.size(); ++i)
            io << m.data()[i];
        return io;
    }

    friend IO::DataStream& operator >> (IO::DataStream& io, FloatMatrix& m)
    {
        io.readHeader("fm", 1);
        quint64 dim;
        std::vector<size_t> dims;
        io >> dim;
        for (size_t i=0; i<dim; ++i)
        {
            quint64 s;
            io >> s;
            dims.push_back(s);
        }
        m.setDimensions(dims);
        for (size_t i=0; i<m.size(); ++i)
            io >> m.data()[i];

        return io;
    }
private:

    Vector p_data_;
    std::vector<size_t> p_dims_, p_offs_;
};


} // namespace MO

#endif // MOSRC_OBJECT_PARAM_FLOATMATRIX_H

