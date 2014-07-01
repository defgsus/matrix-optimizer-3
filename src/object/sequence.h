/** @file sequence.h

    @brief Abstract sequence class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_OBJECT_SEQUENCE_H
#define MOSRC_OBJECT_SEQUENCE_H

#include "object.h"

namespace MO {

class Sequence : public Object
{
    Q_OBJECT
public:
    explicit Sequence(QObject *parent = 0);

    bool isSequence() const { return true; }

    virtual void serialize(IO::DataStream &) const;
    virtual void deserialize(IO::DataStream &);

    // ------------- settings -------------------

    /** Start time in seconds */
    Double start() const { return start_; }

    /** End time in seconds */
    Double end() const { return start_ + length_; }

    /** Length in seconds */
    Double length() const { return length_; }

    /** Loop start time (local) in seconds */
    Double loopStart() const { return loopStart_; }

    /** Loop length in seconds */
    Double loopLength() const { return loopLength_ ; }

    /** Loop end time (local) in seconds */
    Double loopEnd() const { return loopStart_ + loopLength_ ; }

    void setStart(Double t) { start_ = t; }
    void setEnd(Double t) { length_ = t - start_; }
    void setLength(Double t) { length_ = t; }

    void setLoopStart(Double t) { loopStart_ = t; }
    void setLoopEnd(Double t) { loopLength_ = t - loopStart_; }
    void setLoopLength(Double t) { loopLength_ = t; }

signals:

public slots:

private:

    Double start_,
           length_,
           loopStart_,
           loopLength_;

};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCE_H
