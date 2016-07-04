/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_FLOATMATRIXTO_H
#define MOSRC_OBJECT_TEXTURE_FLOATMATRIXTO_H


#include "textureobjectbase.h"

namespace MO {

/** Float matrix to texture converter */
class FloatMatrixTO
        : public TextureObjectBase
{
public:

    MO_OBJECT_CONSTRUCTOR(FloatMatrixTO);

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, const RenderTime& time)
                                                                Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

    const GL::Texture* valueTexture(uint chan, const RenderTime& ) const;

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_FLOATMATRIXTO_H
