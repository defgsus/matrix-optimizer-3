/** @file soundsource.h

    @brief Basic sound Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MO_DISABLE_SPATIAL

#ifndef MOSRC_OBJECT_SOUNDSOURCE_H
#define MOSRC_OBJECT_SOUNDSOURCE_H

#include "Object.h"

namespace MO {

class SoundSource : public Object
{
public:
    MO_OBJECT_CONSTRUCTOR(SoundSource);

    /** @todo This is a stupid type */
    virtual Type type() const Q_DECL_OVERRIDE { return T_SOUNDSOURCE; }
    virtual bool isSoundSource() const Q_DECL_OVERRIDE { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void calculateSoundSourceBuffer(const QList<AUDIO::SpatialSoundSource*>,
                                            const RenderTime& time)
                                            Q_DECL_OVERRIDE;
private:

    ParameterFloat * audioTrack_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SOUNDSOURCE_H


#endif // #ifndef MO_DISABLE_SPATIAL
