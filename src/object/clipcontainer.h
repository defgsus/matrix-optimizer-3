/** @file clipcontainer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_OBJECT_CLIPCONTAINER_H
#define MOSRC_OBJECT_CLIPCONTAINER_H

#include "object.h"

namespace MO {

class ClipContainer : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(ClipContainer)

    bool isClipContainer() const Q_DECL_OVERRIDE { return true; }

    Type type() const Q_DECL_OVERRIDE { return T_CLIP_CONTAINER; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    // -------------- tracks -------------------

    /** The ClipContainer, this sequence is on (actually the parent) */
    //ClipContainer * clipContainer() const;

    // -------------- getter -------------------

    // --------------- setter ---------------------


signals:

public slots:

private:

};



} // namespace MO

#endif // MOSRC_OBJECT_CLIPCONTAINER_H
