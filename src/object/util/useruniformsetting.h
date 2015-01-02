/** @file useruniformsetting.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.01.2015</p>
*/

#ifndef MOSRC_OBJECT_UTIL_USERUNIFORMSETTING_H
#define MOSRC_OBJECT_UTIL_USERUNIFORMSETTING_H

#include <QObject>
#include <QVector>

#include "object/object_fwd.h"
#include "types/float.h"


namespace MO {
namespace GL { class Shader; class Uniform; }

class UserUniformSetting : public QObject
{
    Q_OBJECT
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

    /** Update all found uniform values */
    void updateUniforms(Double time, uint thread);

protected slots:

private:

    Object * object_;
    uint num_;

    struct Uniform
    {
        ParameterText * p_name;
        ParameterSelect * p_type;
        uint gltype;
        QVector<ParameterFloat*> p_float;
        GL::Uniform * uniform;
    };

    QVector<Uniform> uniforms_;
};

} // namespace MO


#endif // MOSRC_OBJECT_UTIL_USERUNIFORMSETTING_H
