/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/10/2016</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_CUBEMAPTO_H
#define MOSRC_OBJECT_TEXTURE_CUBEMAPTO_H

#include "textureobjectbase.h"

namespace MO {

/** A cubemap generator */
class CubemapTO : public TextureObjectBase
{
public:

    MO_OBJECT_CONSTRUCTOR(CubemapTO);

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, const RenderTime& ) Q_DECL_OVERRIDE { }

    /* texture output interface */
    virtual const GL::Texture * valueTexture(uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList&) Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

private:

    void buildCubemap_(const RenderTime& rt);
    bool hasInputChanged_(const RenderTime& time) const;

    GL::Texture * cubeTex_;
};

} // namespace MO


#endif // MOSRC_OBJECT_TEXTURE_CUBEMAPTO_H
