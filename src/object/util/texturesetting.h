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

namespace MO {

class TextureSetting : public QObject
{
    Q_OBJECT
public:
    enum TextureType
    {
        TT_NONE,
        TT_FILE,
        TT_MASTER_FRAME,
        TT_CAMERA_FRAME
    };
    static const QStringList textureTypeNames;

    /** Creates a texture setting for the given Object */
    explicit TextureSetting(Object *parent, GL::ErrorReporting = GL::ER_THROW);
    ~TextureSetting();

    // -------------- io ---------------

    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    // ---------- parameters -----------

    /** Creates the texture-related parameters in parent Object.
        Each parameter id is appended with @p id_suffix, to enable
        more than one texture for an Object.
        @p defaultType is the default value for the type parameter.
        If @p enableNone is true, the TT_NONE type is choosable. */
    void createParameters(const QString& id_suffix,
                          TextureType defaultType = TT_FILE,
                          bool enableNone = false);

    /** Returns true when a change to parameter @p p requires
        a reinitialization.
        Call in Object::onParameterChanged() and e.g. call
        requestReinitGl() when this returns true. */
    bool needsReinit(Parameter * p) const;

    // ------------ getter ---------------

    /** Returns true when type != TT_NONE */
    bool isEnabled() const;

    /** The width of the texture, when initialized */
    uint width() const;
    /** The height of the texture, when initialized */
    uint height() const;

    /** Returns read access to the texture object.
        @note Never release or modify the texture through it's opengl handle!
        There might be other Objects that use the texture.
        Generally just use bind()/unbind() */
    const GL::Texture * texture() const { return constTexture_; }

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

    /** Binds the texture to the active slot.
        Does nothing if type is TT_NONE */
    bool bind(uint slot = 0);
    /** Does nothing if type is TT_NONE */
    void unbind(uint slot = 0);

private:

    Object * object_;

    GL::ErrorReporting rep_;

    GL::Texture * texture_;
    const GL::Texture * constTexture_;

    ParameterSelect * paramType_;
    ParameterFilename * paramFilename_;
    ParameterInt * paramCamera_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_TEXTURESETTING_H
