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

namespace {
    const uint minimumRows_ = 4;
    const uint minimumColumns_ = 4;
}

ClipContainer::ClipContainer(QObject *parent) :
    Object          (parent),
    rows_           (minimumRows_),
    cols_           (minimumColumns_)
{
    setName("ClipContainer");

    // init grid vector
    clips_.resize(cols_ * rows_);
    for (auto & c : clips_)
        c = 0;
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
    // return set name
    auto i = columnNames_.find(index);
    if (i != columnNames_.end())
        return i.value();

    // return letter combi
    QString ret;
    do
    {
        ret.prepend(QChar((index%26) + 'A'));
        index /= 26;
    } while (index);
    return ret;
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

    rows_ = std::max(minimumRows_, std::max(maxRow_, rows));
    cols_ = std::max(minimumColumns_, std::max(maxCol_, cols));

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

    const int i = row * cols_ + column;
    if (i >= clips_.size())
    {
        MO_WARNING("ClipContainer::clip(" << column << ", " << row << ") "
                   "out of range for clips_ array " << i << "/" << clips_.size());
        return 0;
    }
    return clips_[i];
}

bool ClipContainer::findNextFreeSlot(uint &column, uint &row, bool resizeIfNecessary)
{
    column = std::min(column, cols_-1);
    row = std::min(row, rows_-1);

    // If clips_ array has not been initialized yet
    // it's probably XXX save to use the given position
    if (clips_.isEmpty())
        return true;

    int ocolumn = column,
        ocolumn1 = column,
        orow = row;

    // search forward
    while (column < cols_)
    {
        if (clips_[row * cols_ + column] == 0)
            return true;

        row++;
        if (row >= rows_)
        {
            row = 0;
            column++;
        }
    }

    // search backwards
    while (ocolumn >= 0)
    {
        if (clips_[orow * cols_ + ocolumn] == 0)
        {
            column = ocolumn;
            row = orow;
            return true;
        }

        orow--;
        if (orow < 0)
        {
            orow = rows_ - 1;
            ocolumn--;
        }
    }

    // still not found
    if (!resizeIfNecessary)
        return false;

    // resize and place in new row
    setNumberRows(rows_ + 1);
    column = ocolumn1;
    row = rows_ - 1;

    return true;
}

} // namespace MO
