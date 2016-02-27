/** @file texturesetting.h

    @brief Texture setting and allocator for Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_TEXTURESETTING_H
#define MOSRC_OBJECT_UTIL_TEXTURESETTING_H

#include <QCoreApplication> // for Q_DECLARE_TR_FUNCTIONS()
#include <QStringList>

#include "object/util/objectfactory.h"
#include "gl/opengl_fwd.h"
#include "types/time.h"
#include "io/filetypes.h"

namespace MO {

/** A wrapper for texture parameters and load/generation functions */
class TextureSetting
{
    Q_DECLARE_TR_FUNCTIONS(TextureSetting)
public:
    enum TextureType
    {
        TEX_NONE,
        TEX_PARAM,
        TEX_FILE,
        TEX_MASTER_FRAME,
        TEX_MASTER_FRAME_DEPTH
    };
    static const QStringList textureTypeNames;

    enum WrapMode
    {
        WM_CLAMP,
        WM_REPEAT,
        WM_MIRROR
    };

    /** Creates a texture setting for the given Object */
    explicit TextureSetting(Object *parent);
    ~TextureSetting();

    /** If enableNone is true in createParameters() and
        this settings is true, a dummy texture will be created
        for the None setting.
        Default is false. */
    void setNoneTextureProxy(bool enable) { createNoneTex_ = enable; }

    // -------------- io ---------------

    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    /** Appends the filename (and type) to the list of files,
        if type is TT_FILE. */
    void getNeededFiles(IO::FileList & files, IO::FileType ft);

    // ---------- parameters -----------

    /** Creates the texture-related parameters in parent Object.
        Each parameter id is appended with @p id_suffix, to enable
        more than one texture for an Object.
        @p defaultType is the default value for the type parameter.
        If @p enableNone is true, the TT_NONE type is choosable. */
    void createParameters(const QString& id_suffix,
                          TextureType defaultType = TEX_FILE,
                          bool enableNone = false,
                          bool normalMap = false);

    /** Returns true when a change to parameter @p p requires
        a reinitialization.
        Call in Object::onParameterChanged() and e.g. call
        requestReinitGl() when this returns true. */
    bool needsReinit(Parameter * p) const;

    /** Sets the visibility of the parameters according to current settings. */
    void updateParameterVisibility();

    /** Access to the input texture parameter */
    ParameterTexture* textureParam() const { return paramTex_; }

    // ------------ getter ---------------

    /** Returns true when type != TT_NONE */
    bool isEnabled() const;

    /** Is the texture a cubemap texture? */
    bool isCube() const;

    /** Returns true if one of the mip-map minifying modes is selected */
    bool isMipmap() const;

    /** The width of the texture, when initialized */
    uint width() const;
    /** The height of the texture, when initialized */
    uint height() const;

    /** Returns read access to the texture object.
        @note Never release or modify the texture through it's opengl handle!
        There might be other Objects that use the texture.
        Generally just use bind()/unbind() */
    const GL::Texture * texture() const { return constTexture_; }

    /** Returns a string with a description of the error, empty string otherwise */
    const QString& errorString() const { return errorStr_; }

    // ------------ opengl ---------------

    /** Creates or queries the texture, depending on the texture type.
        Does nothing if type is TT_NONE.
        @warning Calling initGl() and releaseGl() out of logical order
        leads to undefined behaviour.
        @throws GlException */
    void initGl();

    /** Releases the texture.
        @warning Calling initGl() and releaseGl() out of logical order
        leads to undefined behaviour. */
    void releaseGl();

    /** Binds the texture to the given slot.
        Does nothing if type is TT_NONE.
        @throws GlException */
    void bind(const RenderTime & time, uint slot = 0);

protected slots:

    void setTextureFromImage_(const QString& fn);
    void setTextureFromAS_(const QString& script);
    void setNoneTexture_();
    void updateSceneFbo_();

private:

    Object * object_;

    GL::Texture * texture_;
    const GL::Texture * constTexture_;

    ParameterSelect * paramType_, *paramInterpol_,
                    *paramWrapX_, *paramWrapY_, *paramMinify_;
    ParameterInt * paramMipmaps_;
    ParameterFilename * paramFilename_;
    //ParameterText * paramAngelScript_;
    ParameterTexture * paramTex_;

    QString errorStr_;
    bool createNoneTex_,
         isParamCube_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_TEXTURESETTING_H
