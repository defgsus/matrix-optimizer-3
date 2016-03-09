/** @file useruniformsetting.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.01.2015</p>
*/

#ifndef MOSRC_OBJECT_UTIL_USERUNIFORMSETTING_H
#define MOSRC_OBJECT_UTIL_USERUNIFORMSETTING_H

#include <QCoreApplication> // for Q_DECLARE_TR_FUNCTIONS()
#include <QVector>

#include "object/object_fwd.h"
#include "object/interface/evolutioneditinterface.h"
#include "types/time.h"

namespace MO {
namespace GL { class Shader; class Uniform; class Texture; }

class UserUniformSetting
        : public QObject
{
    Q_DECLARE_TR_FUNCTIONS(UserUniformSetting)
public:
    /** Creates a uniform setting for the given Object */
    explicit UserUniformSetting(Object *parent, uint maxUniforms = 10);
    ~UserUniformSetting();

    // -------------- io ---------------

    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    // ---------- parameters -----------

    /** Creates the parameters in parent Object.
        Each parameter id is appended with @p id_suffix, to enable
        more than one uniform group for an Object */
    void createParameters(const QString& id_suffix);

    /** Returns true when parameter @p p requires
        a reinitialization.
        Call in Object::onParameterChanged() and e.g. call
        requestReinitGl() when this returns true. */
    bool needsReinit(Parameter * p) const;

    /** Sets the visibility of the parameters according to current settings. */
    void updateParameterVisibility();

    // ------------ getter ---------------

    /** Returns a list of declarations with each defined uniform */
    QString getDeclarations() const;

    // ------------ opengl ---------------

    /** Locate all relevant uniforms in the shader.
        Shader must be compiled. */
    void tieToShader(GL::Shader *);

    /** Update all found uniform values.
        Will also bind textures when needed, starting with @p texSlot,
        which is increased for every bound texture. */
    void updateUniforms(const MO::RenderTime & time, uint* texSlot);

    void releaseGl();

protected slots:

private:

    Object * object_;
    uint num_;

    enum UniformType
    {
        UT_NONE,
        UT_F1, UT_F2, UT_F3, UT_F4,
        UT_TEX, UT_TEX_C,
        UT_T1, UT_T2, UT_T3, UT_T4
    };

    struct Uniform
    {
        bool isUsed() const;
        bool isTextureInput() const;
        bool isBufferTexture() const;

        uint gltype,
             num_floats;

        ParameterText * p_name;
        ParameterSelect * p_type;
        ParameterInt * p_length;
        ParameterFloat * p_timerange;
        TextureSetting * p_texSet;
        QVector<ParameterFloat*> p_float;

        GL::Uniform * uniform;
        GL::Texture * ownTexture;
        std::vector<float> texBuf;
    };

    QVector<Uniform> uniforms_;
    Double uploadTime_;
};

} // namespace MO


#endif // MOSRC_OBJECT_UTIL_USERUNIFORMSETTING_H
