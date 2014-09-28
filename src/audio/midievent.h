/** @file midievent.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/7/2014</p>
*/

#ifndef MOSRC_AUDIO_MIDIEVENT_H
#define MOSRC_AUDIO_MIDIEVENT_H

#include <cinttypes>

#include <QString>

namespace MO {
namespace AUDIO {


/** A classic midi event
    - no sysex messages
    - currently no NRPN controllers */
class MidiEvent
{
public:

    enum Command
    {
        C_NOTE_OFF          = 0x8,
        C_NOTE_ON           = 0x9,
        C_KEY_PRESSURE      = 0xa,
        C_CONTROL_CHANGE    = 0xb,
        C_PROGRAM_CHANGE    = 0xc,
        C_CHANNEL_PRESSURE  = 0xd,
        C_PITCH_BEND        = 0xe
    };

    /** Constructs an invalid message */
    MidiEvent() : m_(0) { }

    explicit MidiEvent(uint32_t m);

    // ------------ getter ----------------

    bool isValid() const { return m_ != 0; }

    Command command() const { return (Command)(status_ >> 4); }
    uint8_t channel() const { return status_ & 0xf; }

    uint8_t key() const { return data1_ & 0x7f; }
    uint8_t velocity() const { return data2_ & 0x7f; }

    uint8_t controller() const { return data1_ & 0x7f; }
    uint8_t value() const { return data2_ & 0x7f; }

    uint8_t program() const { return data1_ & 0x7f; }

    uint8_t channelPressure() const { return data1_ & 0x7f; }

    int16_t pitchBendValue() const
        { return (((int16_t)(data2_ & 0x7f) << 7) | (data1_ & 0x7f)) - 0x2000; }

    uint8_t data1() const { return data1_; }
    uint8_t data2() const { return data2_; }
    uint8_t data3() const { return data3_; }

    QString toString() const;

    // --------------- setter -------------------

    void setCommand(Command c) { status_ = (status_ & 0xf) | (c << 4); }
    void setChannel(uint8_t c) { status_ = (status_ & 0xf0) | (c & 0xf); }
    void setKey(uint8_t k) { data1_ = k & 0x7f; }
    void setVelocity(uint8_t v) { data2_ = v & 0x7f; }

private:

    union
    {
        uint32_t m_;
        struct
        {
            uint8_t
                status_,
                data1_,
                data2_,
                data3_;
        };
    };
};


} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_MIDIEVENT_H
