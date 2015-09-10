/** @file texturesetting.h

    @brief Texture setting and allocator for Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_TEXTURESETTING_H
#define MOSRC_OBJECT_UTIL_TEXTURESETTING_H

#include <QObject>
#include <QStringList>

#include "object/objectfactory.h"
#include "gl/opengl_fwd.h"
#include "io/filetypes.h"

namespace MO {

/** A wrapper for texture parameters and load/generation functions */
class TextureSetting : public QObject
{
    Q_OBJECT
public:
    enum TextureType
    {
        TEX_NONE,
        TEX_PARAM,
        TEX_FILE,
        TEX_MASTER_FRAME,
        TEX_MASTER_FRAME_DEPTH,
        TEX_CAMERA_FRAME,
        TEX_CAMERA_FRAME_DEPTH
    };
    static const QStringList textureTypeNames;

    enum WrapMode
    {
        WM_CLAMP,
        WM_REPEAT,
        WM_MIRROR
    };

    /** Creates a texture setting for the given Object */
    explicit TextureSetting(Object *parent, GL::ErrorReporting = GL::ER_THROW);
    ~TextureSetting();

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

    // ------------ getter ---------------

    /** Returns true when type != TT_NONE */
    bool isEnabled() const;

    /** Is the texture a cubemap texture? */
    bool isCube() const;

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
        leads to undefined behaviour. */
    bool initGl();
    /** Releases the texture.
        @warning Calling initGl() and releaseGl() out of logical order
        leads to undefined behaviour. */
    void releaseGl();

    /** Binds the texture to the given slot.
        Does nothing if type is TT_NONE */
    bool bind(uint slot = 0);

protected slots:

    bool setTextureFromImage_(const QString& fn);
    bool setTextureFromAS_(const QString& script);
    bool updateCameraFbo_();
    bool updateSceneFbo_();

private:

    Object * object_;

    GL::ErrorReporting rep_;

    GL::Texture * texture_;
    const GL::Texture * constTexture_;

    ParameterSelect * paramType_, *paramInterpol_,
                    *paramWrapX_, *paramWrapY_;
    ParameterFilename * paramFilename_;
    //ParameterText * paramAngelScript_;
    ParameterInt * paramCamera_;
    ParameterTexture * paramTex_;

    QString errorStr_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_TEXTURESETTING_H
