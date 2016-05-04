/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 5/4/2016</p>
*/

#ifdef MO_ENABLE_FFMPEG

#ifndef MOSRC_OBJECT_TEXTURE_VIDEOTO_H
#define MOSRC_OBJECT_TEXTURE_VIDEOTO_H

#include "textureobjectbase.h"

namespace MO {

/** Video to texture */
class VideoTO : public TextureObjectBase
{
public:

    MO_OBJECT_CONSTRUCTOR(VideoTO);

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

    void setVideoFilename(const QString& fn);

private:
    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_VIDEOTO_H

#endif // #ifdef MO_ENABLE_FFMPEG
