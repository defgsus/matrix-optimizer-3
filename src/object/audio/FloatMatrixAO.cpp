/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#include <QMutex>
#include <QMutexLocker>

#include "FloatMatrixAO.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/tool/ResampleBuffer.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterFloatMatrix.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "object/param/FloatMatrix.h"
#include "object/util/ObjectEditor.h"
#include "io/DataStream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(FloatMatrixAO)

class FloatMatrixAO::Private
{
    public:

    Private()
        : hasMatrixChanged  (true)
        , matrixMutex       (QMutex::Recursive)
    { }

    ParameterInt
        * paramSize;

    std::vector<AUDIO::ResampleBuffer<F32>> rebufs;
    FloatMatrix matrix;
    bool hasMatrixChanged;
    QMutex matrixMutex;
};

FloatMatrixAO::FloatMatrixAO()
    : AudioObject   (),
      p_            (new Private())
{
    setName("FloatMatrix");
    setNumberAudioInputsOutputs(1, 0);
    setNumberOutputs(ST_FLOAT_MATRIX, 1);
}

FloatMatrixAO::~FloatMatrixAO()
{
    delete p_;
}

QString FloatMatrixAO::getOutputName(SignalType st, uint channel) const
{
    if (st == ST_FLOAT_MATRIX && channel == 0)
        return tr("matrix");
    return AudioObject::getOutputName(st, channel);
}

FloatMatrix FloatMatrixAO::valueFloatMatrix(uint , const RenderTime& ) const
{
    QMutexLocker lock(&p_->matrixMutex);
    p_->hasMatrixChanged = false;
    return p_->matrix;
}

bool FloatMatrixAO::hasFloatMatrixChanged(
        uint , const RenderTime& ) const
{
    QMutexLocker lock(&p_->matrixMutex);
    return p_->hasMatrixChanged;
}

void FloatMatrixAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aofloatm", 1);
}

void FloatMatrixAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aofloatm", 1);
}

void FloatMatrixAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("convert", tr("convert"));
    initParameterGroupExpanded("convert");

        p_->paramSize = params()->createIntParameter(
                    "matrix_width", tr("width"),
            tr("Number of horizontal samples in matrix"),
            1024,  true, false);
        p_->paramSize->setMinValue(1);

    params()->endParameterGroup();
}

void FloatMatrixAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->paramSize)
    {
        QMutexLocker lock(&p_->matrixMutex);
        p_->matrix.setDimensions({size_t(p_->paramSize->baseValue())});
        p_->hasMatrixChanged = true;
    }
}

void FloatMatrixAO::onParametersLoaded()
{
    {
        QMutexLocker lock(&p_->matrixMutex);
        p_->matrix.setDimensions({size_t(p_->paramSize->baseValue())});
        p_->hasMatrixChanged = true;
    }
}

void FloatMatrixAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->rebufs.resize(count);
}

/*
void FloatMatrixAO::setAudioBuffers(uint thread, uint bufferSize,
                            const QList<AUDIO::AudioBuffer*>&,
                            const QList<AUDIO::AudioBuffer*>&)
{

}
*/

void FloatMatrixAO::processAudio(const RenderTime& time)
{
    size_t width = p_->matrix.numDimensions() ? p_->matrix.size(0) : 0;
    if (width == 0)
        return;

    const QList<AUDIO::AudioBuffer*>&
            inputs = audioInputs(time.thread());

    AUDIO::ResampleBuffer<F32>&
            rebuf = p_->rebufs[time.thread()];

    if (rebuf.size() != width)
        rebuf.setSize(width);

    for (AUDIO::AudioBuffer* in : inputs)
    {
        if (!in)
            continue;

        rebuf.push(in->readPointer(), in->blockSize());

        if (rebuf.hasBuffer())
        {
            // copy to matrix output
            QMutexLocker lock(&p_->matrixMutex);
            const F32* src = rebuf.buffer();
            for (size_t i=0; i<width; ++i, ++src)
                *p_->matrix.data(i) = *src;
            p_->hasMatrixChanged = true;

            rebuf.pop();
        }
    }
}


} // namespace MO
