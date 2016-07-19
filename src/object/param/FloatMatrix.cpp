/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/19/2016</p>
*/

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


} // namespace MO
