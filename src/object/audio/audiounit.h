/** @file audiounit.h

    @brief Abstract audio processing object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_AUDIOUNIT_H
#define MOSRC_OBJECT_AUDIO_AUDIOUNIT_H

#include <QStringList>

#include "object/object.h"

namespace MO {

class AudioUnit : public Object
{
    Q_OBJECT
public:

    enum ProcessMode
    {
        PM_ON,
        PM_OFF,
        PM_BYPASS
    };

    const static QStringList processModeIds;


    MO_ABSTRACT_OBJECT_CONSTRUCTOR(AudioUnit);

    bool isAudioUnit() const { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    //virtual void performAudioBlock(SamplePos pos, uint thread) Q_DECL_OVERRIDE;

    // -------------- getter -----------------

    /** Returns the currently set process mode */
    ProcessMode processMode() const;

private:

    ParameterSelect * audioProcessing_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_AUDIOUNIT_H
