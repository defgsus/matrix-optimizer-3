/** @file bandlimitwavetablegenerator.cpp

    @brief Generator for wavetables with bandlimited common waveforms

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.10.2014</p>
*/

/* Note:
 *
 * http://musicdsp.org/showArchiveComment.php?ArchiveID=134
 * states that he made good experience with 32x oversampling and
 * a 511-tap FIR (blackman-harris) 12 khz cutoff.
 * maybe try that once...
 *
 */

#include <vector>

#include "bandlimitwavetablegenerator.h"
#include "fixedfilter.h"
#include "math/constants.h"
#include "math/funcparser/parser.h"
//#include "math/fft.h"
//#include <iostream>

namespace MO {
namespace AUDIO {


class BandlimitWavetableGenerator::Private
{
public:

    Private()
        : mode              (M_WAVEFORM),
          tableSize         (nextPowerOfTwo(32000)),
          oversampling      (4),
          waveType          (Waveform::T_SQUARE),
          pulseWidth        (0.5),
          needRecalc        (true),
          needWaveCalc      (true)
    { }

    void updateTable();
    void filterTable();
    void generateWaveformTable();
    void generateEquationTable();

    Mode mode;

    uint tableSize, oversampling;
    std::vector<Double> table, ftable;

    Waveform::Type waveType;
    Double pulseWidth;
    QString equation;

    bool needRecalc,
         needWaveCalc;

    FixedFilter filter;
};



BandlimitWavetableGenerator::BandlimitWavetableGenerator()
    : p_        (new Private())
{
}

BandlimitWavetableGenerator::~BandlimitWavetableGenerator()
{
    delete p_;
}

uint BandlimitWavetableGenerator::tableSize() const { return p_->tableSize; }
uint BandlimitWavetableGenerator::oversampling() const { return p_->oversampling; }
Double * BandlimitWavetableGenerator::p_table_() { return &p_->table[0]; }

void BandlimitWavetableGenerator::setTableSize(uint size)
{
    p_->tableSize = nextPowerOfTwo(size);
    p_->needRecalc = true;
}

void BandlimitWavetableGenerator::setOversampling(uint over)
{
    p_->oversampling = std::max(2u, over);
    p_->needRecalc = true;
}

void BandlimitWavetableGenerator::setWaveform(Waveform::Type type)
{
    p_->waveType = type;
    p_->needWaveCalc = true;
}

void BandlimitWavetableGenerator::setPulseWidth(Double pw)
{
    p_->pulseWidth = Waveform::limitPulseWidth(pw);
    p_->needWaveCalc = true;
}

void BandlimitWavetableGenerator::setMode(Mode mode)
{
    p_->mode = mode;
    p_->needWaveCalc = true;
}

void BandlimitWavetableGenerator::setEquation(const QString & e)
{
    p_->equation = e;
    p_->needWaveCalc = true;
}


/** Create table, setup filter, render table and filter it,
    when dirty-flags are set */
void BandlimitWavetableGenerator::p_update_()
{
    if (p_->needRecalc)
    {
        p_->updateTable();
        p_->needRecalc = false;
        p_->needWaveCalc = true;
    }

    if (p_->needWaveCalc)
    {
        p_->needWaveCalc = false;

        switch (p_->mode)
        {
            case M_WAVEFORM: p_->generateWaveformTable(); break;
            case M_EQUATION: p_->generateEquationTable(); break;
        }

        p_->filterTable();
    }
}


void BandlimitWavetableGenerator::Private::updateTable()
{
    // suppose tableSize_ * oversampling_ is exactly one second/period
    const uint hz = tableSize * oversampling;

    table.resize(hz);
    ftable.resize(hz);
}

void BandlimitWavetableGenerator::Private::filterTable()
{
    // setup filter
    filter.setType(FixedFilter::FT_BUTTERWORTH);
    filter.setBandType(FixedFilter::BT_LOWPASS);
    filter.setOrder(6);
    // some experimental values for samplerate and frequency
    // (suppose that the entire wavetable is a waveform at 1 hz)
    filter.setSampleRate(tableSize * 2);
    filter.setFrequency(tableSize / 60);
    filter.updateCoefficients();

    const uint hz = tableSize * oversampling;

    // filter the result
    filter.reset();
    // one pass to fill filter history
    filter.process(&table[0], &ftable[0], hz);
    // one real pass
    filter.process(&table[0], &ftable[0], hz);
}


void BandlimitWavetableGenerator::Private::generateWaveformTable()
{
    const uint hz = tableSize * oversampling;

    // generate it
    for (uint i=0; i<hz; ++i)
    {
        table[i] = Waveform::waveform(Double(i) / hz, waveType, pulseWidth);
    }
}

void BandlimitWavetableGenerator::Private::generateEquationTable()
{
    const uint hz = tableSize * oversampling;

    // prepare an equation
    PPP_NAMESPACE::Parser equ;

    PPP_NAMESPACE::Float vx, vxr;
    equ.variables().add("x", &vx, "");
    equ.variables().add("xr", &vxr, "");

    equ.parse(equation.toStdString());

    // generate it
    for (uint i=0; i<hz; ++i)
    {
        vx = Double(i) / hz;
        vxr = vx * TWO_PI;
        table[i] = equ.eval();
    }

}


} // namespace AUDIO
} // namespace MO
