/** @file clipcontroller.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include "ClipController.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"
#include "Clip.h"

namespace MO {

MO_REGISTER_OBJECT(ClipController)

namespace {
    const uint minimumRows_ = 12;
    const uint minimumColumns_ = 12;
}

ClipController::ClipController() :
    Object          (),
    rows_           (minimumRows_),
    cols_           (minimumColumns_)
{
    setName("ClipController");

    // init grid vector
    clipGrid_.resize(cols_ * rows_);
    for (auto & c : clipGrid_)
        c = 0;
}

ClipController::~ClipController() { }

void ClipController::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("clipcon", 1);

    io << rows_ << cols_;
}

void ClipController::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("clipcon", 1);

    io >> rows_ >> cols_;

    cols_ = std::max(minimumColumns_, cols_);
    rows_ = std::max(minimumRows_, rows_);
}

void ClipController::createParameters()
{
    Object::createParameters();
}


void ClipController::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}

void ClipController::childrenChanged()
{
    setNumber(cols_, rows_);
}

QString ClipController::columnName(uint index) const
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

QString ClipController::rowName(uint index) const
{
    auto i = rowNames_.find(index);
    if (i != rowNames_.end())
        return i.value();
    else
        return QString::number(index);
}

void ClipController::setNumber(uint cols, uint rows)
{
    cols_ = cols;
    rows_ = rows;
    updateClipPositions();
}


void ClipController::collectClips()
{
    Object * root = rootObject();
    clips_ = root->findChildObjects<Clip>("", true);

    for (Clip * c : clips_)
    {
        if (c->clipContainer() != this)
        {
            c->setClipContainer(this);
            uint col = c->column(),
                 row = c->row();
            if (findNextFreeSlot(col, row, true))
                c->setPosition(col, row);
        }
    }

    updateClipPositions();
}


void ClipController::updateClipPositions()
{
    // find maximum rows and columns of clips
    maxRow_ = 0;
    maxCol_ = 0;
    for (auto c : clips_)
    {
        maxCol_ = std::max(maxCol_, c->column());
        maxRow_ = std::max(maxRow_, c->row());
    }

    rows_ = std::max(minimumRows_, std::max(maxRow_, rows_));
    cols_ = std::max(minimumColumns_, std::max(maxCol_, cols_));

    // resize grid vector
    clipGrid_.resize(cols_ * rows_);
    for (auto & c : clipGrid_)
        c = 0;

    // put into grid vector
    for (auto c : clips_)
    {
        const uint i = c->row() * cols_ + c->column();
        if (clipGrid_[i])
            MO_WARNING("Clip '" << c->idName() << "' on same position "
                       "(" << c->column() << ", " << c->row() << ") in ClipContainer as "
                       "'" << clipGrid_[i]->idName() << "'");
        clipGrid_[i] = c;
    }
}

Clip * ClipController::clip(uint column, uint row) const
{
    if (column >= cols_ || row >= rows_)
        return 0;

    const int i = row * cols_ + column;
    if (i >= clipGrid_.size())
    {
        MO_WARNING("ClipContainer::clip(" << column << ", " << row << ") "
                   "out of range for clips_ array " << i << "/" << clipGrid_.size());
        return 0;
    }
    return clipGrid_[i];
}

Clip * ClipController::playingClip(uint column) const
{
    if (column >= cols_)
    {
        MO_WARNING("ClipContainer::playingClip(" << column << ") "
                   "out of range (" << cols_ << ")");
        return 0;
    }

    for (uint y = 0; y < rows_; ++y)
    {
        Clip * c = clip(column, y);
        if (c && c->isPlaying())
            return c;
    }

    return 0;
}


bool ClipController::findNextFreeSlot(uint &column, uint &row, bool resizeIfNecessary, bool* resized)
{
    if (resized)
        *resized = false;

    column = std::min(column, cols_-1);
    row = std::min(row, rows_-1);

    // If clips_ array has not been initialized yet
    // it's probably XXX save to use the given position
    if (clipGrid_.isEmpty())
        return true;

    int ocolumn = column,
        ocolumn1 = column,
        orow = row;

    // search forward
    while (column < cols_)
    {
        if (clipGrid_[row * cols_ + column] == 0)
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
        if (clipGrid_[orow * cols_ + ocolumn] == 0)
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

    if (resized)
        *resized = true;

    return true;
}


void ClipController::triggerClip(Clip *clip, Double gtime)
{
    MO_DEBUG_CLIP("ClipContainer::triggerClip(" << clip << ", " << gtime << ")");

    ///@todo newobj emit clipTriggered(clip);

    // stop other clip on this column
    if (Clip * c = playingClip(clip->column()))
        if (c != clip)
            triggerStopClip(c, gtime);

    // XXX
    clip->startClip(gtime);
    ///@todo newobj emit clipStarted(clip);
}

void ClipController::triggerStopClip(Clip *clip, Double gtime)
{
    MO_DEBUG_CLIP("ClipContainer::triggerStopClip(" << clip << ", " << gtime << ")");
    Q_UNUSED(gtime);

    ///@todo newobj emit clipStopTriggered(clip);

    // XXX
    clip->stopClip();
    ///@todo newobj emit clipStopped(clip);
}

void ClipController::triggerRow(uint index, Double gtime)
{
    MO_DEBUG_CLIP("ClipContainer::triggerRow(" << index << ", " << gtime << ")");

    MO_ASSERT(index < rows_, "ClipContainer::triggerRow(" << index << ", " << gtime << ") "
              "out of range (" << rows_ << ")");

    for (uint x = 0; x < cols_; ++x)
    {
        Clip * c = clip(x, index);
        Clip * other = playingClip(x);

        if (c)
        {
            ///@todo newobj emit clipTriggered(c);

            // XXX
            c->startClip(gtime);
            ///@todo newobj emit clipStarted(c);
        }

        // stop others on this column
        if (other && other != c)
            triggerStopClip(other, gtime);
    }
}

void ClipController::triggerStopColumn(uint index, Double gtime)
{
    MO_DEBUG("ClipContainer::triggerStopColumn(" << index << ", " << gtime << ")");

    MO_ASSERT(index < cols_, "ClipContainer::triggerStopColumn(" << index << ", " << gtime << ") "
              "out of range (" << cols_ << ")");

    if (Clip * other = playingClip(index))
        triggerStopClip(other, gtime);
}




} // namespace MO
