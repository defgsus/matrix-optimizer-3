/** @file sprite.h

    @brief A sprite, always facing the camera

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/19/2014</p>
*/

#ifndef MO_DISABLE_EXP

#ifndef MOSRC_OBJECT_SPRITE_H
#define MOSRC_OBJECT_SPRITE_H


#include "objectgl.h"

namespace MO {

class Sprite : public ObjectGl
{
    Q_OBJECT
public:

    MO_OBJECT_CONSTRUCTOR(Sprite);

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings& rs, uint, Double time) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList &files) Q_DECL_OVERRIDE;

signals:

private slots:

private:
    GL::Drawable * draw_;
    TextureSetting * texture_;

    ParameterFloat * cr_, *cg_, *cb_, *ca_,
            *numRep_, *timeSpan_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SPRITE_H

#endif // #ifndef MO_DISABLE_EXP
