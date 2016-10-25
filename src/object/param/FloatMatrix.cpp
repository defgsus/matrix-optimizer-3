/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/19/2016</p>
*/

#include <sstream>

#include <QJsonObject>
#include <QJsonArray>

#include "FloatMatrix.h"

namespace MO {

QJsonObject FloatMatrix::toJson() const
{
    QJsonObject main;

    // dimensions
    main.insert("dims", JSON::toArray(p_dims_));
    // data
    main.insert("data", JSON::toArray(p_data_));

    return main;
}

void FloatMatrix::fromJson(const QJsonObject& main)
{
    std::vector<size_t> dims;
    std::vector<Double> data;

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

std::string FloatMatrix::layoutString() const
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

std::string FloatMatrix::rangeString() const
{
    if (p_data_.empty())
        return "0";
    Double mi = p_data_.front(), ma = mi;
    for (const auto& f : p_data_)
        mi = std::min(mi, f), ma = std::max(ma, f);
    std::stringstream s;
    s << mi << " - " << ma;
    return s.str();
}



std::vector<Float> FloatMatrix::toFloat() const
{
    std::vector<Float> d;
    for (const auto& f : p_data_)
        d.push_back(f);
    return d;
}

void FloatMatrix::setDimensions(const std::vector<size_t>& dimensions)
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

} // namespace MO
