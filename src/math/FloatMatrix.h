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

//#include "types/float.h"
#include "io/error.h"
#include "io/DataStream.h"
#include "io/JsonInterface.h"

namespace MO {

class ProgressInfo;

/** @brief Multi-dimensional float data container

    @todo
    Currently this is passed around by value between objects
    to make it threadsafe. Not ideal, but a copy-on-write would
    require lot's of lockings...

*/
template <typename F>
class FloatMatrixT
        : public JsonInterface
{
public:
    typedef std::vector<F> Vector;

    FloatMatrixT() { }
    explicit FloatMatrixT(const std::vector<size_t>& dimensions)
        { setDimensions(dimensions); }

    // ---- io ----

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // ---- getter ----

    std::string layoutString() const;
    std::string rangeString() const;

    bool isEmpty() const { return p_dims_.empty(); }

    const std::vector<size_t>& dimensions() const { return p_dims_; }

    /** Number of dimensions */
    size_t numDimensions() const { return p_dims_.size(); }
    /** Size of all data */
    size_t size() const { return p_data_.size(); }
    /** Size of one dimension */
    size_t size(size_t dimension) const
        { MO_ASSERT(dimension < p_dims_.size(),
                    "dimension="<<dimension<<", p_dims_="<<p_dims_.size());
          return p_dims_[dimension]; }

    bool hasDimensions(const std::vector<size_t>& dims)
    {
        return dims == p_dims_;
    }

    // --- read access ---

    /** Convert to float data array */
    template <typename T>
    FloatMatrixT<T> toType() const;

    /** Read access to all data */
    const F* data() const
        { MO_ASSERT(!p_data_.empty(), "");
          return &p_data_[0]; }

    const F* data(size_t i0) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(i0 < p_data_.size(), "i0="<<i0<< ", layout=" << layoutString());
          return &p_data_[i0]; }

    const F* data(size_t i0, size_t i1) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(i1*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", layout="<<layoutString());
          return &p_data_[i1*p_dims_[0] + i0]; }

    const F* data(size_t i0, size_t i1, size_t i2) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT((i2*p_dims_[1] + i1)*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", layout="<<layoutString());
          return &p_data_[(i2*p_dims_[1] + i1)*p_dims_[0] + i0]; }

    const F* data(size_t i0, size_t i1, size_t i2, size_t i3) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(((i3*p_dims_[2]+i2)*p_dims_[1]+i1)*p_dims_[0]+i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", i3="
                   <<i3<<", layout="<<layoutString());
          return &p_data_[((i3*p_dims_[2] + i2)*p_dims_[1] + i1)*p_dims_[0] + i0]; }

    /** Read single values */
    const F& operator()(size_t i0) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(i0 < p_data_.size(), "i0="<<i0<< ", layout=" << layoutString());
          return p_data_[i0]; }

    const F& operator()(size_t i0, size_t i1) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(i1*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", layout="<<layoutString());
          return p_data_[i1*p_dims_[0] + i0]; }

    const F& operator()(size_t i0, size_t i1, size_t i2) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT((i2*p_dims_[1] + i1)*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", layout="<<layoutString());
          return p_data_[(i2*p_dims_[1] + i1)*p_dims_[0] + i0]; }

    const F& operator()(size_t i0, size_t i1, size_t i2, size_t i3) const
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(((i3*p_dims_[2]+i2)*p_dims_[1]+i1)*p_dims_[0]+i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", i3="
                   <<i3<<", layout="<<layoutString());
          return p_data_[((i3*p_dims_[2] + i2)*p_dims_[1] + i1)*p_dims_[0] + i0]; }
    // --- std container ---

    typename Vector::const_iterator begin() const { return p_data_.begin(); }
    typename Vector::const_iterator end() const { return p_data_.end(); }
    typename Vector::iterator begin() { return p_data_.begin(); }
    typename Vector::iterator end() { return p_data_.end(); }

    // --- compare ---

    bool operator == (const FloatMatrixT<F>& o) const { return !(*this != o); }
    bool operator != (const FloatMatrixT<F>& o) const
    {
        if (o.numDimensions() != numDimensions())
            return true;
        for (size_t i=0; i<numDimensions(); ++i)
            if (o.size(i) != size(i))
                return true;
        for (size_t i=0; i<size(); ++i)
            if (o.data()[i] != data()[i])
                return true;
        return false;
    }

    // --- setter ---

    /** Reserve space for each dimension */
    void setDimensions(const std::vector<size_t>& dimensions);

    template <typename T>
    void setDimensionsFrom(const FloatMatrixT<T>& other)
    {
        setDimensions(other.dimensions());
    }

    void clear()
    {
        p_dims_.clear();
        p_data_.clear();
        p_offs_.clear();
    }

    /** Write access to all data */
    F* data()
        { MO_ASSERT(!p_data_.empty(), "");
          return &p_data_[0]; }
    F* data(size_t i0)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(i0 < p_data_.size(), "i0="<<i0<< ", layout=" << layoutString());
          return &p_data_[i0]; }

    F* data(size_t i0, size_t i1)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(i1*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", layout="<<layoutString());
          return &p_data_[i1*p_dims_[0] + i0]; }

    F* data(size_t i0, size_t i1, size_t i2)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT((i2*p_dims_[1] + i1)*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", layout="<<layoutString());
          return &p_data_[(i2*p_dims_[1] + i1)*p_dims_[0] + i0]; }

    F* data(size_t i0, size_t i1, size_t i2, size_t i3)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(((i3*p_dims_[2]+i2)*p_dims_[1]+i1)*p_dims_[0]+i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", i3="
                   <<i3<<", layout="<<layoutString());
          return &p_data_[((i3*p_dims_[2] + i2)*p_dims_[1] + i1)*p_dims_[0] + i0]; }

    /** Write access to single value */
    F& operator()(size_t i0)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(i0 < p_data_.size(), "i0="<<i0<< ", layout=" << layoutString());
          return p_data_[i0]; }

    F& operator()(size_t i0, size_t i1)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(i1*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", layout="<<layoutString());
          return p_data_[i1*p_dims_[0] + i0]; }

    F& operator()(size_t i0, size_t i1, size_t i2)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT((i2*p_dims_[1] + i1)*p_dims_[0] + i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", layout="<<layoutString());
          return p_data_[(i2*p_dims_[1] + i1)*p_dims_[0] + i0]; }

    F& operator()(size_t i0, size_t i1, size_t i2, size_t i3)
        { MO_ASSERT(!p_data_.empty(), "");
          MO_ASSERT(p_dims_.size() >= 2, "layout="<<layoutString());
          MO_ASSERT(((i3*p_dims_[2]+i2)*p_dims_[1]+i1)*p_dims_[0]+i0 < p_data_.size(),
                   "i0="<<i0<<", i1="<<i1<<", i2="<<i2<<", i3="
                   <<i3<<", layout="<<layoutString());
          return p_data_[((i3*p_dims_[2] + i2)*p_dims_[1] + i1)*p_dims_[0] + i0]; }

    // ----- signed distance field -----

    enum DFMode { DF_EXACT, DF_FAST };

    bool calcDistanceField(
            const FloatMatrixT<F>& binarySource, DFMode mode,
            ProgressInfo* pinfo = nullptr);



    // -------- io ----------

    friend IO::DataStream& operator << (
            IO::DataStream& io, const FloatMatrixT<F>& m)
    {
        io.writeHeader("fm", 1);
        io << quint64(m.numDimensions());
        for (size_t i=0; i<m.numDimensions(); ++i)
            io << quint64(m.size(i));
        for (size_t i=0; i<m.size(); ++i)
            io << m.data()[i];
        return io;
    }

    friend IO::DataStream& operator >> (
            IO::DataStream& io, FloatMatrixT<F>& m)
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


#ifndef MO_DEFAULT_FLOAT_MATRIX_DEFINED
#   define MO_DEFAULT_FLOAT_MATRIX_DEFINED
    /** default type */
    typedef FloatMatrixT<double> FloatMatrix;
#endif

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_FLOATMATRIX_H

