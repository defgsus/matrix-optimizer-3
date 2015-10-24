/** @file parametertexture.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.05.2015</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERTEXTURE_H
#define MOSRC_OBJECT_PARAM_PARAMETERTEXTURE_H

#include "parameter.h"
#include "types/time.h"

namespace MO {
namespace GL { class Texture; }

class ParameterTexture : public Parameter
{
public:
    /*
    enum Type
    {
        T_TEXTURE_2D,
        T_TEXTURE_CUBE
    };*/

    ParameterTexture(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("texture"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_TEXTURE; }

    // ---------------- getter -----------------

    const GL::Texture* value(const RenderTime& time) const;

    /** Returns true when the texture is different since the last call to value() */
    bool hasChanged(const RenderTime& time) const Q_DECL_OVERRIDE;

    // ---------------- setter -----------------

    /* Set this upon creation */
    //void setType(Type) { type_ = type; }

    // --------- modulation -----------

    int getModulatorTypes() const Q_DECL_OVERRIDE;

    virtual Modulator * getModulator(const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE;

private:

    mutable const GL::Texture * lastTex_;
    mutable std::vector<int> lastHash_;

};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERTEXTURE_H
