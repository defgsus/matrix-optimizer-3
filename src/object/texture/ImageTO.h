/** @file imageto.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_IMAGETO_H
#define MOSRC_OBJECT_TEXTURE_IMAGETO_H

#include "TextureObjectBase.h"

namespace MO {

/** An image to texture loader */
class ImageTO : public TextureObjectBase
{
public:

    MO_OBJECT_CONSTRUCTOR(ImageTO);

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

    void setImageFilename(const QString& fn);

private:

    QString initFilename_;
    ParameterFilename * pFilename_;
    ParameterInt * pMipmaps_;
    GL::Texture * tex_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_IMAGETO_H
