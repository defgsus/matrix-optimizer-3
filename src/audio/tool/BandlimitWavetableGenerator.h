/** @file bandlimitwavetablegenerator.h

    @brief Generator for wavetables with bandlimited common waveforms

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.10.2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_BANDLIMITWAVETABLEGENERATOR_H
#define MOSRC_AUDIO_TOOL_BANDLIMITWAVETABLEGENERATOR_H

#include "Wavetable.h"
#include "Waveform.h"
#include "types/float.h"
#include "types/int.h"

namespace MO {
namespace AUDIO {

/** Generator for wavetables with bandlimited common waveforms */
class BandlimitWavetableGenerator
{
public:

    enum Mode
    {
        M_WAVEFORM,
        M_EQUATION
    };

    BandlimitWavetableGenerator();
    ~BandlimitWavetableGenerator();

    // ------------- getter ----------------

    uint tableSize() const;
    uint oversampling() const;

    // ------------- setter ----------------

    /** Sets the size of the tables.
        Will be rounded to the next power of two */
    void setTableSize(uint size);

    /** Sets the amount of oversampling */
    void setOversampling(uint over);

    /** Sets the mode of the generator, either oscillator waveform or equation */
    void setMode(Mode mode);

    /** Sets the type of waveform to render */
    void setWaveform(Waveform::Type);

    /** Sets the pulsewidth for Waveform types that support it. */
    void setPulseWidth(Double pw);

    /** Sets the equation to use in M_EQUATION mode.
        Variables are 'x' [0,1] and 'xr' [0,TWO_PI] */
    void setEquation(const QString&);

    // ------------ wavetables -------------

    /** Generates the wavetable with the given waveform of size tableSize() */
    template <typename F>
    void createWavetable(Wavetable<F> & wt);

private:

    void p_update_();

    Double * p_table_();

    class Private;
    Private * p_;
};



// ___________________________ template IMPL ___________________________

template <typename F>
void BandlimitWavetableGenerator::createWavetable(Wavetable<F> &wt)
{
    // call implementation code (if necessary)
    p_update_();

    // downsample into wavetable

    wt.setSize(tableSize());

    Double * in = p_table_();
    F      * out = wt.data();

    for (uint i=0; i<tableSize(); ++i, ++out)
    {
        Double sum = 0.0;
        for (uint j=0; j<oversampling(); ++j, ++in)
            sum += *in;

        *out = sum / oversampling();
    }
}




} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_BANDLIMITWAVETABLEGENERATOR_H
