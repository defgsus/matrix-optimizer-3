/** @file colorto.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_COLORTO_H
#define MOSRC_OBJECT_TEXTURE_COLORTO_H

#include "TextureObjectBase.h"

namespace MO {

/** All kinds of color processing */
class ColorTO : public TextureObjectBase
{
public:

    MO_OBJECT_CONSTRUCTOR(ColorTO);

    //virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, const RenderTime& time) Q_DECL_OVERRIDE;

    /* texture output interface */
    //virtual const GL::Texture * valueTexture(Double time, uint thread) const Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_COLORTO_H
