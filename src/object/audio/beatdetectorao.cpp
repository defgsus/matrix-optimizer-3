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
        * paramFft;
    ParameterInt
        * paramNumBins;
    ParameterFloat
        * paramAverage,
        * paramThresh;

    std::vector<AUDIO::BeatDetector> beats;
    FloatMatrix matrixBeat, matrixHistory;

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
    setNumberOutputs(ST_FLOAT_MATRIX, 2);
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
    }
    return AudioObject::getOutputName(st, channel);
}

Double BeatDetectorAO::valueFloat(uint , const RenderTime& ) const
{
    return 0.;
}

FloatMatrix BeatDetectorAO::valueFloatMatrix(uint , const RenderTime& ) const
{
    QMutexLocker lock(&p_->matrixMutex);
    return p_->matrixBeat;
}

QString BeatDetectorAO::infoString() const
{
    return QString("fft-size=%1, bins=%2")
            .arg(p_->paramFft ? p_->paramFft->baseValue() : p_->initFftSize)
            .arg(p_->paramNumBins ? p_->paramNumBins->baseValue() : p_->initNumBins)
            ;
}

void BeatDetectorAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aofft", 1);
}

void BeatDetectorAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aofft", 1);
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
            numBins = p_->paramNumBins->value(time);

    AUDIO::BeatDetector& beat = p_->beats[time.thread()];
    // changes to beatdetector that cause a reset
    if (beat.fftSize() != fftSize
     || beat.numBins() != numBins)
        beat.setSize(fftSize, numBins);
    // changes that can be set just so
    beat.setAverageTime(p_->paramAverage->value(time));
    beat.setThreshold(p_->paramThresh->value(time));
    beat.setSampleRate(time.sampleRate());

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
                    *p_->matrixBeat.data(i, 1) = beat.averageLevel()[i];
                    *p_->matrixBeat.data(i, 2) = beat.currentLevel()[i];
                }
            }
            break;
        }

    }
}


} // namespace MO

