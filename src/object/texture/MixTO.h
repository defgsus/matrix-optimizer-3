/** @file mixto.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_MIXTO_H
#define MOSRC_OBJECT_TEXTURE_MIXTO_H

#include "TextureObjectBase.h"

namespace MO {

/** Mixer for multiple channels */
class MixTO : public TextureObjectBase
{
public:

    enum CombineFunc
    {
        CF_ADD,
        CF_SUB,
        CF_MUL,
        CF_DIV,
        CF_MIN,
        CF_MAX
    };
    enum ModFunc
    {
        MF_NONE,
        MF_INVERT,
        MF_INVERT_ALL
    };

    MO_OBJECT_CONSTRUCTOR(MixTO);

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

#endif // MOSRC_OBJECT_TEXTURE_MIXTO_H
