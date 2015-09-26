/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/27/2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_IMAGESTO_H
#define MOSRC_OBJECT_TEXTURE_IMAGESTO_H

#include "textureobjectbase.h"

namespace MO {

/** An multiple images to texture loader */
class ImagesTO : public TextureObjectBase
{
    Q_OBJECT
public:

    MO_OBJECT_CONSTRUCTOR(ImagesTO);
    ~ImagesTO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, uint, Double) Q_DECL_OVERRIDE { }

    /* texture output interface */
    virtual const GL::Texture * valueTexture(uint channel, Double time, uint thread) const Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList&) Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

    void setImageFilenames(const QStringList& fn);

signals:

public slots:

private:

    QStringList filenames_, initFilenames_;
    ParameterImageList * pFilenames_;
    ParameterInt * pIndex_;
    QList<GL::Texture*> tex_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_IMAGESTO_H
