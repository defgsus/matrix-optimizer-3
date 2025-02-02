/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/27/2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_IMAGESTO_H
#define MOSRC_OBJECT_TEXTURE_IMAGESTO_H

#include "TextureObjectBase.h"

namespace MO {

/** An multiple images to texture loader */
class ImagesTO : public TextureObjectBase
{
public:

    MO_OBJECT_CONSTRUCTOR(ImagesTO);

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

    void setImageFilenames(const QStringList& fn);

private:

    QStringList filenames_, initFilenames_;
    ParameterImageList * pFilenames_;
    ParameterInt * pIndex_, *pMipmaps_;
    QList<GL::Texture*> tex_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_IMAGESTO_H
