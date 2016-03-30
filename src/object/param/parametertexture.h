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

    enum InputType
    {
        IT_NONE,    //! Really no texture at all
        IT_BLACK,
        IT_WHITE,
        IT_INPUT,
        IT_FILE,
        IT_MASTER_FRAME,
        IT_MASTER_FRAME_DEPTH
    };
    static const QStringList inputTypeIds;
    static const QStringList inputTypeNames;
    static const QList<InputType> inputTypeValues;

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
    static const QStringList minModeNamesShort;
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
    ~ParameterTexture();

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("texture"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_TEXTURE; }

    virtual void copyFrom(Parameter* other) Q_DECL_OVERRIDE;

    QString baseValueString(bool ) const override { return "XXX"; }
    QString valueString(const RenderTime& , bool ) const override { return "XXX"; }

    // ---------------- getter -----------------

    const GL::Texture* value(const RenderTime& time) const;

    /** Returns true when the texture is different since the last call to value() */
    bool hasChanged(const RenderTime& time) const Q_DECL_OVERRIDE;

    InputType inputType() const;
    InputType defaultInputType() const;
    WrapMode wrapModeX() const;
    WrapMode wrapModeY() const;
    MagMode magMode() const;
    MinMode minMode() const;
    uint mipmaps() const;
    QString filename() const;

    /** Returns true when at least one of the minifiying modes is set to
        mip-mapping. */
    bool isMipmap() const;

    ParameterFilename* filenameParameter() const;

    // ---------------- setter -----------------

    void setInputType(InputType);
    void setDefaultInputType(InputType);
    void setWrapMode(WrapMode m);
    void setWrapModeX(WrapMode m);
    void setWrapModeY(WrapMode m);
    void setMagMode(MagMode m);
    void setMinMode(MinMode m);
    void setMipmaps(uint level);

    void setFilenameParameter(ParameterFilename*);

    // -------- opengl ----------------

    /** Applies the wrap and min/mag settings to the current
        OpenGL state. @p tex MUST BE BOUND! */
    void applyTextureParam(const GL::Texture* tex) const;

    /** Release any created textures */
    void releaseGl();

    // --------- modulation -----------

    int getModulatorTypes() const Q_DECL_OVERRIDE;

    virtual Modulator * getModulator(const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE;

private:
    struct Private;
    Private * p_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERTEXTURE_H
