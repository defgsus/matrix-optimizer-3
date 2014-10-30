/** @file oscillatorobject.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.10.2014</p>
*/

#ifndef MOSRC_OBJECT_OSCILLATOROBJECT_H
#define MOSRC_OBJECT_OSCILLATOROBJECT_H

#include "object.h"

namespace MO {

#ifdef XXX_NEEDS_RETHINKING
class OscillatorObject : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(OscillatorObject);

    virtual Type type() const Q_DECL_OVERRIDE { return T_OSCILLATOR; }

    virtual void createParameters() Q_DECL_OVERRIDE;


    // ---------------- output --------------

//    void advance()

//    Double value(Double time, uint thread);

    //virtual void performAudioBlock(SamplePos pos, uint thread) Q_DECL_OVERRIDE;
signals:

public slots:

private:

};
#endif

} // namespace MO

#endif // MOSRC_OBJECT_OSCILLATOROBJECT_H
