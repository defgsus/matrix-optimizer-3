/** @file lensdistto.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_LENSDISTTO_H
#define MOSRC_OBJECT_TEXTURE_LENSDISTTO_H


#include "textureobjectbase.h"

namespace MO {

/** Classic lens distorion with chromatic abberation and user function */
class LensDistTO : public TextureObjectBase
{
public:

    MO_OBJECT_CONSTRUCTOR(LensDistTO);

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, const RenderTime& time) Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_LENSDISTTO_H
