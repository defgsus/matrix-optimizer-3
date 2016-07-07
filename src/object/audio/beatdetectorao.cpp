/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/6/2016</p>
*/

#include <QMutex>
#include <QMutexLocker>

#include "beatdetectorao.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/beatdetector.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterfloatmatrix.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/floatmatrix.h"
#include "object/util/objecteditor.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(BeatDetectorAO)

class BeatDetectorAO::Private
{
    public:

    Private()
        : initFftSize           (1024)
        , initNumBins           (16)
    { }

    ParameterSelect
        * paramFft,
        * paramHist,
        * paramBinary;
    ParameterInt
        * paramNumBins;
    ParameterFloat
        * paramFreqLow,
        * paramFreqHigh,
        * paramAverage,
        * paramThresh;

    std::vector<AUDIO::BeatDetector> beats;
    FloatMatrix
            matrixBeat, matrixHistory,
            matrixConv, matrixSpeed, matrixCandis;

    size_t initFftSize, initNumBins;
    QMutex matrixMutex;
};

BeatDetectorAO::BeatDetectorAO()
    : AudioObject   (),
      p_            (new Private())
{
    setName("BeatDetect");
    setNumberAudioInputsOutputs(1, 0);
    //setNumberOutputs(ST_FLOAT, 1);
    setNumberOutputs(ST_FLOAT_MATRIX, 5);
}

BeatDetectorAO::~BeatDetectorAO()
{
    delete p_;
}

QString BeatDetectorAO::getOutputName(SignalType st, uint channel) const
{
    if (st == ST_FLOAT && channel == 0)
        return tr("");
    if (st == ST_FLOAT_MATRIX)
    {
        if (channel == 0)
            return tr("beat");
        if (channel == 1)
            return tr("history");
        if (channel == 2)
            return tr("conv");
        if (channel == 3)
            return tr("speed");
        if (channel == 4)
            return tr("candis");

    }
    return AudioObject::getOutputName(st, channel);
}

Double BeatDetectorAO::valueFloat(uint , const RenderTime& ) const
{
    return 0.;
}

FloatMatrix BeatDetectorAO::valueFloatMatrix(uint chan, const RenderTime& ) const
{
    QMutexLocker lock(&p_->matrixMutex);
    switch (chan)
    {
        default:
        case 0: return p_->matrixBeat;
        case 1: return p_->matrixHistory;
        case 2: return p_->matrixConv;
        case 3: return p_->matrixSpeed;
        case 4: return p_->matrixCandis;
    }
}

QString BeatDetectorAO::infoString() const
{
    return QString("fft-size=%1, bins=%2")//, history=%3")
            .arg(p_->paramFft ? p_->paramFft->baseValue() : p_->initFftSize)
            .arg(p_->paramNumBins ? p_->paramNumBins->baseValue() : p_->initNumBins)
            //.arg(p_->)
            ;
}

void BeatDetectorAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aobeatdet", 2);
}

void BeatDetectorAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aobeatdet", 2);
}

void BeatDetectorAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("fft", tr("fourier transform"));
    initParameterGroupExpanded("fft");

        p_->paramFft = params()->createSelectParameter(
                    "fft_size", tr("size"),
            tr("The size of the fourier window in samples"),
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "", "", "", "", "", "", "", "", "", "", "", "", "" },
            { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
            p_->initFftSize, true, false);

        p_->paramNumBins = params()->createIntParameter(
                    "num_bins", tr("bin count"),
                    tr("The number of frequency bands analyzed"),
                    p_->initNumBins, true, true);
        p_->paramNumBins->setMinValue(1);

        p_->paramFreqLow = params()->createFloatParameter(
                    "freq_low", tr("low frequency"),
                    tr("The lowest frequency to consider"),
                    0., 10.);
        p_->paramFreqLow->setMinValue(0.);

        p_->paramFreqHigh = params()->createFloatParameter(
                    "freq_high", tr("high frequency"),
                    tr("The highest frequency to consider"),
                    20000., 100.);
        p_->paramFreqHigh->setMinValue(0.);

        p_->paramHist = params()->createSelectParameter(
                    "history_size", tr("history size"),
            tr("The number of chunks in beat history"),
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
            { "", "", "", "", "", "", "", "", "", "", "", "", "" },
            { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
            256, true, false);

        p_->paramAverage = params()->createFloatParameter(
                    "average_time", tr("average time"),
                    tr("The time in seconds to gather the average amplitude "
                       "used for thresholding"),
                    1.0, 0.05);
        p_->paramAverage->setMinValue(0.0);

        p_->paramThresh = params()->createFloatParameter(
                    "threshold", tr("threshold"),
                    tr("Multiplier for comparing the current signal versus "
                       "the average signal"),
                    1.3, 0.05);

        p_->paramBinary = params()->createBooleanParameter(
                    "binary_beat", tr("binary"),
                    tr(""),
                    tr("Off"), tr("On"),
                    false, true, true);

    params()->endParameterGroup();
}

void BeatDetectorAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);
}

void BeatDetectorAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();
}

void BeatDetectorAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->beats.resize(count);
}


void BeatDetectorAO::processAudio(const RenderTime& time)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs = audioInputs(time.thread());

    if (inputs.size() < 1 || inputs[0] == nullptr)
        return;

    const size_t
            fftSize = nextPowerOfTwo((uint)p_->paramFft->value(time)),
            numBins = p_->paramNumBins->value(time),
            numHist = p_->paramHist->value(time);

    AUDIO::BeatDetector& beat = p_->beats[time.thread()];
    // changes to beatdetector that cause a reset
    if (beat.fftSize() != fftSize
     || beat.numBins() != numBins
     || beat.numHistory() != numHist
     || uint(beat.sampleRate()) != time.sampleRate())
        beat.setSize(fftSize, numBins, numHist, time.sampleRate());
    // changes that can be set just so
    beat.setAverageTime(p_->paramAverage->value(time));
    beat.setThreshold(p_->paramThresh->value(time));
    beat.setBeatsBinary(p_->paramBinary->value(time));
    // This will resize bins on change
    beat.setFrequencyRange(p_->paramFreqLow->value(time),
                           p_->paramFreqHigh->value(time));

    // perform
    beat.push(inputs[0]->readPointer(), inputs[0]->blockSize());


    // copy to matrix output
    {
        QMutexLocker lock(&p_->matrixMutex);

        switch (1)
        {
            case 0:
                if (!p_->matrixBeat.isEmpty())
                    p_->matrixBeat.clear();
            break;

            case 1:
            {
                std::vector<size_t> dim = { beat.numBins(), 3 };
                if (!p_->matrixBeat.hasDimensions(dim))
                    p_->matrixBeat.setDimensions(dim);
                for (size_t i=0; i<beat.numBins(); ++i)
                {
                    *p_->matrixBeat.data(i, 0) = beat.beat()[i];
                    *p_->matrixBeat.data(i, 1) = beat.currentLevel()[i];
                    *p_->matrixBeat.data(i, 2) = beat.averageLevel()[i];
                }
            }
            break;
        }

        switch (1)
        {
            case 0:
                if (!p_->matrixHistory.isEmpty())
                    p_->matrixHistory.clear();
            break;

            case 1:
            {
                std::vector<size_t> dim = { beat.numBins(), beat.numHistory() };
                if (!p_->matrixHistory.hasDimensions(dim))
                    p_->matrixHistory.setDimensions(dim);
                for (size_t i=0; i<beat.numBins(); ++i)
                    for (size_t j=0; j<beat.numHistory(); ++j)
                        *p_->matrixHistory.data(i, j) = beat.beatHistory(i)[j];
            }
            break;
        }

        switch (1)
        {
            case 0:
                if (!p_->matrixConv.isEmpty())
                    p_->matrixConv.clear();
            break;

            case 1:
            {
                std::vector<size_t> dim = { beat.numBins(), beat.numHistory() };
                if (!p_->matrixConv.hasDimensions(dim))
                    p_->matrixConv.setDimensions(dim);
                for (size_t i=0; i<beat.numBins(); ++i)
                    for (size_t j=0; j<beat.numHistory(); ++j)
                        *p_->matrixConv.data(i, j) = beat.convolution(i)[j];
            }
            break;
        }

        switch (1)
        {
            case 0:
                if (!p_->matrixSpeed.isEmpty())
                    p_->matrixSpeed.clear();
            break;

            case 1:
            {
                std::vector<size_t> dim = { beat.numBins(), 3 };
                if (!p_->matrixSpeed.hasDimensions(dim))
                    p_->matrixSpeed.setDimensions(dim);
                for (size_t i=0; i<beat.numBins(); ++i)
                {
                    *p_->matrixSpeed.data(i, 0) = beat.beatsPerSecond(i);
                    *p_->matrixSpeed.data(i, 1) = beat.lengthNormalized(i);
                    *p_->matrixSpeed.data(i, 2) = beat.bestMatch(i);
                }
            }
            break;
        }

        switch (1)
        {
            case 0:
                if (!p_->matrixCandis.isEmpty())
                    p_->matrixCandis.clear();
            break;

            case 1:
            {
                std::vector<size_t> dim = { beat.numBins(), beat.numHistory() };
                if (!p_->matrixCandis.hasDimensions(dim))
                    p_->matrixCandis.setDimensions(dim);
                for (size_t i=0; i<beat.numBins(); ++i)
                    for (size_t j=0; j<beat.numHistory(); ++j)
                        *p_->matrixCandis.data(i, j) = beat.candidates(i)[j];
            }
            break;
        }
    }
}


} // namespace MO

