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
        TT_FILENAME,
        TT_MASTER_FRAME,
        TT_CAMERA_FRAME
    };
    static const QStringList textureTypeNames;

    explicit TextureSetting(Object *parent, GL::ErrorReporting = GL::ER_THROW);

    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    void createParameters(const QString& id_suffix);

    // ------------ getter ---------------

    uint width() const;
    uint height() const;

    const GL::Texture * texture() const { return texture_; }

    // ------------ opengl ---------------

    bool initGl();
    void releaseGl();

    bool bind();
    void unbind();

private:

    Object * object_;

    GL::ErrorReporting rep_;

    GL::Texture * texture_;

    ParameterSelect * paramType_;
    ParameterFilename * paramFilename_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_TEXTURESETTING_H
