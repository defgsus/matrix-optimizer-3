/** @file midievent.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/7/2014</p>
*/

#include "MidiEvent.h"

namespace MO {
namespace AUDIO {

MidiEvent::MidiEvent(uint32_t m)
    : m_  (m)
{
    // translate zero velocity note-on to note-off
    if (command() == C_NOTE_ON && !velocity())
        setCommand(C_NOTE_OFF);
}

QString MidiEvent::toString() const
{
    if (!isValid())
        return "none";

    switch (command())
    {
        case C_NOTE_OFF: return QString("note-off %1 %2 (%3)")
                    .arg(key()).arg(velocity()).arg(channel());
        case C_NOTE_ON: return QString("note-on %1 %2 (%3)")
                    .arg(key()).arg(velocity()).arg(channel());
        case C_KEY_PRESSURE: return QString("key-press %1 %2 (%3)")
                    .arg(key()).arg(velocity()).arg(channel());
        case C_CONTROL_CHANGE: return QString("cc %1 %2 (%3)")
                    .arg(controller()).arg(value()).arg(channel());
        case C_PROGRAM_CHANGE: return QString("program %1 (%2)")
                    .arg(program()).arg(channel());
        case C_CHANNEL_PRESSURE: return QString("channel-press %1 (%2)")
                    .arg(channelPressure()).arg(channel());
        case C_PITCH_BEND: return QString("pitch-bend %1 (%2)")
                    .arg(pitchBendValue()).arg(channel());
    }

    return QString("unhandled %1 %2 %3")
            .arg(data1(),1,16,QChar('0'))
            .arg(data2(),1,16,QChar('0'))
            .arg(data3(),1,16,QChar('0'));
}

} // namespace AUDIO
} // namespace MO
