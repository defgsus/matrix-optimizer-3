/** @file filterao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_FILTERAO_H
#define MOSRC_OBJECT_AUDIO_FILTERAO_H

#include "object/AudioObject.h"

namespace MO {

class FilterAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(FilterAO)

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter*) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint count) Q_DECL_OVERRIDE;

protected:

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_FILTERAO_H
