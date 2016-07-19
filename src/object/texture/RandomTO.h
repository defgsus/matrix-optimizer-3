/** @file randomto.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_RANDOMTO_H
#define MOSRC_OBJECT_TEXTURE_RANDOMTO_H

#include "TextureObjectBase.h"

namespace MO {

/** A more sophisticated noise generator */
class RandomTO : public TextureObjectBase
{
public:

    enum Pattern
    {
        P_RECT,
        P_PERLIN,
        P_VORONOISE,
        P_CIRCLE
    };

    enum FractalMode
    {
        FM_SINGLE = 0,
        FM_AVERAGE,
        FM_MAX,
        FM_RECURSIVE,
        FM_RANDOM
    };

    MO_OBJECT_CONSTRUCTOR(RandomTO);

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

#endif // MOSRC_OBJECT_TEXTURE_RANDOMTO_H
