/** @file clipcontainer.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include "clipcontainer.h"
#include "io/datastream.h"
#include "io/error.h"
#include "clip.h"

namespace MO {

MO_REGISTER_OBJECT(ClipContainer)

ClipContainer::ClipContainer(QObject *parent) :
    Object          (parent),
    rows_           (0),
    cols_           (0)
{
    setName("ClipContainer");
}

void ClipContainer::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("clipcon", 1);

    io << rows_ << cols_;
}

void ClipContainer::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("clipcon", 1);

    io >> rows_ >> cols_;
}

void ClipContainer::createParameters()
{
    Object::createParameters();
}


void ClipContainer::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}

void ClipContainer::childrenChanged()
{
    setNumber(cols_, rows_);
}

QString ClipContainer::columnName(uint index) const
{
    auto i = columnNames_.find(index);
    if (i != columnNames_.end())
        return i.value();
    else
        return QString::number(index);
}

QString ClipContainer::rowName(uint index) const
{
    auto i = rowNames_.find(index);
    if (i != rowNames_.end())
        return i.value();
    else
        return QString::number(index);
}

void ClipContainer::setNumber(uint cols, uint rows)
{
    // get all clips
    auto clips = findChildObjects<Clip>();

    // find maximum rows and columns of clips
    maxRow_ = 0;
    maxCol_ = 0;
    for (auto c : clips)
    {
        maxCol_ = std::max(maxCol_, c->column());
        maxRow_ = std::max(maxRow_, c->row());
    }

    // resize if necessary
    if (maxRow_ > rows_ || maxCol_ > cols_)
        setNumber(cols_, rows_);

    rows_ = std::max(maxRow_, rows);
    cols_ = std::max(maxCol_, cols);

    // resize grid vector
    clips_.resize(cols_ * rows_);
    for (auto & c : clips_)
        c = 0;

    // put into grid vector
    for (auto c : clips)
    {
        const uint i = c->row() * cols_ + c->column();
        if (clips_[i])
            MO_WARNING("Clip '" << c->idName() << "' on same position "
                       "(" << c->column() << ", " << c->row() << ") in ClipContainer as "
                       "'" << clips_[i]->idName() << "'");
        clips_[i] = c;
    }
}

Clip * ClipContainer::clip(uint column, uint row) const
{
    if (column >= cols_ || row >= rows_)
        return 0;

    return clips_[row * cols_ + column];
}

} // namespace MO
