/** @file dumbfile.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.12.2014</p>
*/

#ifndef MO_DISABLE_DUMB

#ifndef MOSRC_AUDIO_TOOL_DUMBFILE_H
#define MOSRC_AUDIO_TOOL_DUMBFILE_H

#include <QString>
#include <QList>

#include "types/float.h"
#include "audio/configuration.h"

namespace MO {
namespace AUDIO {

class AudioBuffer;

/** Tracker file loader/player using libdumb */
class DumbFile
{
    DumbFile(const DumbFile&);
    void operator = (const DumbFile&);

public:
    DumbFile();
    DumbFile(const AUDIO::Configuration&);

    ~DumbFile();

    // ----------------- getter -----------------

    bool isOpen() const;
    bool isReady() const;

    const QString& filename() const;

    long position() const;

    const AUDIO::Configuration& config() const;

    // ------------------------------------------

    /** Throws Exception on errors */
    void open(const QString& filename);

    void close();

    /** Call before processing */
    void setConfig(const AUDIO::Configuration&);

    void process(const QList<AudioBuffer*>& outputs, F32 amp = 1.f);

    void setPosition(long pos);
    void setPositionThreadsafe(long pos);

private:

    class Private;
    Private * p_;
};


} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_DUMBFILE_H

#endif // #ifndef MO_DISABLE_DUMB
