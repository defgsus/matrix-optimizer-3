/** @file clip.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_OBJECT_CLIP_H
#define MOSRC_OBJECT_CLIP_H

#include "object.h"

namespace MO {

class Clip : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Clip)

    bool isClip() const Q_DECL_OVERRIDE { return true; }

    Type type() const Q_DECL_OVERRIDE { return T_CLIP; }

    //virtual QString infoName() const;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    /** Collects all sequences */
    virtual void childrenChanged() Q_DECL_OVERRIDE;

    // -------------- tracks -------------------

    /** The ClipContainer, this Clip is on (actually the parent) */
    ClipContainer * clipContainer() const;

    // -------------- getter -------------------

    // -------------- setter -------------------


    // -------------- values -------------------

    //Double value(Double time, uint thread) const;

signals:

public slots:

private:

    QList<Sequence*> sequences_;

};



} // namespace MO

#endif // MOSRC_OBJECT_CLIP_H
