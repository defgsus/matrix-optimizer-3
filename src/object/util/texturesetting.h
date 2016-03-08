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
#include "object/param/parametertexture.h"
#include "gl/opengl_fwd.h"
#include "types/time.h"
#include "io/filetypes.h"

namespace MO {

/** A wrapper for texture parameters and load/generation functions */
class TextureSetting
{
    Q_DECLARE_TR_FUNCTIONS(TextureSetting)
public:

    /** Creates a texture setting for the given Object */
    explicit TextureSetting(Object *parent);
    ~TextureSetting();

    // -------------- io ---------------

    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    /** Appends the filename (and type) to the list of files */
    void getNeededFiles(IO::FileList& files);

    // ---------- parameters -----------

    /** Creates the texture-related parameters in parent Object.
        Each parameter id is appended with @p id_suffix, to enable
        more than one texture for an Object.
        @p defaultType is the default type of texture source. */
    void createParameters(
            const QString& id_suffix,
            ParameterTexture::InputType defaultType = ParameterTexture::IT_WHITE,
            bool normalMap = false);

    /** Sets the visibility of the parameters according to current settings. */
    void updateParameterVisibility();

    /** Access to the actual texture parameter */
    ParameterTexture* textureParam() const { return paramTex_; }

    // ------------ getter ---------------

    /** Is the texture a cubemap texture? */
    bool isCube() const;

    /** Returns true if one of the mip-map minifying modes is selected */
    bool isMipmap() const;

    /** Is enabled at all? */
    bool isEnabled() const;

    // -------- stateful getter ----------
    // (for single user)

    /** Returns true if isCube() has changed since the last
        call to isCube() or to this function. */
    bool checkCubeChanged();
    bool checkEnabledChanged();

    // ------------ opengl ---------------

    /** Releases the texture that might have been created by the
        ParameterTexture. */
    void releaseGl();

    /** Binds the texture to the given slot.
        Does nothing if type is TT_NONE.
        @throws GlException */
    void bind(const RenderTime & time, uint slot = 0);

private:

    Object * object_;

    ParameterTexture * paramTex_;
    ParameterFilename * paramFilename_;

    bool p_isCube_;
    mutable bool p_lastIsCube_, p_lastIsEnabled_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_TEXTURESETTING_H
