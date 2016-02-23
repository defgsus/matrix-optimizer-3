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

/** A Parameter for texture input */
class ParameterTexture : public Parameter
{
public:
    /*
    enum Type
    {
        T_TEXTURE_2D,
        T_TEXTURE_CUBE
    };*/

    enum MagMode
    {
        MAG_NEAREST,
        MAG_LINEAR
    };
    static const QStringList magModeIds;
    static const QStringList magModeNames;
    static const QList<MagMode> magModeValues;

    enum MinMode
    {
        MIN_NEAREST,
        MIN_LINEAR,
        MIN_NEAREST_MIPMAP_NEAREST,
        MIN_LINEAR_MIPMAP_NEAREST,
        MIN_NEAREST_MIPMAP_LINEAR,
        MIN_LINEAR_MIPMAP_LINEAR
    };
    static const QStringList minModeIds;
    static const QStringList minModeNames;
    static const QList<MinMode> minModeValues;

    enum WrapMode
    {
        WM_CLAMP,
        WM_REPEAT,
        WM_MIRROR
    };
    static const QStringList wrapModeIds;
    static const QStringList wrapModeNames;
    static const QList<WrapMode> wrapModeValues;

    ParameterTexture(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("texture"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_TEXTURE; }

    QString baseValueString(bool ) const override { return "XXX"; }
    QString valueString(const RenderTime& , bool ) const override { return "XXX"; }

    // ---------------- getter -----------------

    const GL::Texture* value(const RenderTime& time) const;

    /** Returns true when the texture is different since the last call to value() */
    bool hasChanged(const RenderTime& time) const Q_DECL_OVERRIDE;

    WrapMode wrapModeX() const { return wrapModeX_; }
    WrapMode wrapModeY() const { return wrapModeY_; }
    MagMode magMode() const { return magMode_; }
    MinMode minMode() const { return minMode_; }

    /** Applies the wrap and min/mag settings */
    void setTextureParam(const GL::Texture*) const;

    // ---------------- setter -----------------

    /* Set this upon creation */
    //void setType(Type) { type_ = type; }

    void setWrapMode(WrapMode m) { setWrapModeX(m); setWrapModeY(m); }
    void setWrapModeX(WrapMode m) { wrapModeX_ = m; }
    void setWrapModeY(WrapMode m) { wrapModeY_ = m; }
    void setMagMode(MagMode m) { magMode_ = m; }
    void setMinMode(MinMode m) { minMode_ = m; }

    // --------- modulation -----------

    int getModulatorTypes() const Q_DECL_OVERRIDE;

    virtual Modulator * getModulator(const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE;

private:

    mutable const GL::Texture * lastTex_;
    mutable std::vector<int> lastHash_;

    WrapMode wrapModeX_, wrapModeY_;
    MagMode magMode_;
    MinMode minMode_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERTEXTURE_H
