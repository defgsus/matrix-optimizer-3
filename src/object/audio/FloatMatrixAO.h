/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_FLOATMATRIXAO_H
#define MOSRC_OBJECT_AUDIO_FLOATMATRIXAO_H

#include "object/AudioObject.h"
#include "object/interface/ValueFloatMatrixInterface.h"

namespace MO {

/** Converts audio channels to FloatMatrix */
class FloatMatrixAO
        : public AudioObject
        , public ValueFloatMatrixInterface
{
public:

    MO_OBJECT_CONSTRUCTOR(FloatMatrixAO)

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint count) Q_DECL_OVERRIDE;

    virtual QString getOutputName(SignalType, uint channel) const override;

    virtual FloatMatrix valueFloatMatrix(
            uint channel, const RenderTime &time) const override;
    virtual bool hasFloatMatrixChanged(
            uint channel, const RenderTime &time) const override;

protected:

    /*virtual void setAudioBuffers(uint thread, uint bufferSize,
                                 const QList<AUDIO::AudioBuffer*>& inputs,
                                 const QList<AUDIO::AudioBuffer*>& outputs)
                                                            Q_DECL_OVERRIDE; */

    virtual void processAudio(const RenderTime&) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_FLOATMATRIXAO_H
