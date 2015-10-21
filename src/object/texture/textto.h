/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/21/2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_TEXTTO_H
#define MOSRC_OBJECT_TEXTURE_TEXTTO_H


#include "textureobjectbase.h"

namespace MO {

/** An image to texture loader */
class TextTO : public TextureObjectBase
{
    Q_OBJECT
public:

    MO_OBJECT_CONSTRUCTOR(TextTO);
    ~TextTO();

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

    void setText(const QString& text);

signals:

public slots:

private:

    QString text_;
    ParameterText * pText_;
    ParameterInt * pMipmaps_;
    ParameterFloat * pSize_, *pR_, *pG_, *pB_, *pA_,
        *pbR_, *pbG_, *pbB_, *pbA_;
    ParameterSelect * pAlignH_, *pAlignV_;
    mutable GL::Texture * tex_;
    bool doRenderText_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_TEXTTO_H
