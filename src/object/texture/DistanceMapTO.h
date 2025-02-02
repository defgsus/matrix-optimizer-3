/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/20/2015</p>
*/

#ifndef MO_DISABLE_EXP

#ifndef MOSRC_OBJECT_TEXTURE_DISTANCEMAPTO_H
#define MOSRC_OBJECT_TEXTURE_DISTANCEMAPTO_H


#include "textureobjectbase.h"

namespace MO {

/** .. */
class DistanceMapTO : public TextureObjectBase
{
public:

    MO_OBJECT_CONSTRUCTOR(DistanceMapTO);
    ~DistanceMapTO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, uint, Double) Q_DECL_OVERRIDE;

    /* texture output interface */
    //virtual const GL::Texture * valueTexture(Double time, uint thread) const Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_DISTANCEMAPTO_H

#endif // MO_DISABLE_EXP
