/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/6/2016</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_BEATDETECTORAO_H
#define MOSRC_OBJECT_AUDIO_BEATDETECTORAO_H


#include "object/audioobject.h"
#include "object/interface/valuefloatinterface.h"
#include "object/interface/valuefloatmatrixinterface.h"

namespace MO {

/** 'Classic' multi-band beat detection */
class BeatDetectorAO : public AudioObject
            , public ValueFloatInterface
            , public ValueFloatMatrixInterface
{
public:

    MO_OBJECT_CONSTRUCTOR(BeatDetectorAO)

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint count) Q_DECL_OVERRIDE;

    virtual QString getOutputName(SignalType, uint channel) const override;
    virtual QString infoString() const override;

    virtual Double valueFloat(uint channel, const RenderTime &time) const override;
    virtual FloatMatrix valueFloatMatrix(
            uint channel, const RenderTime &time) const override;

protected:

    virtual void processAudio(const RenderTime&) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO


#endif // MOSRC_OBJECT_AUDIO_BEATDETECTORAO_H
