/** @file appicons.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.01.2015</p>
*/

#include <QIcon>
#include <QColor>
#include <QPixmap>
#include <QBitmap>

#include "appicons.h"
#include "object/object.h"
#include "object/transform/translation.h"
#include "object/transform/axisrotation.h"
#include "object/transform/scale.h"
#include "object/transform/shear.h"
#include "object/transform/look.h"
#include "object/transform/lookat.h"
#include "object/transform/mix.h"
#include "object/transform/transformationinput.h"
#include "object/transform/mirrortrans.h"
#include "object/transform/cleartrans.h"
#include "object/scene.h"
#include "object/control/trackfloat.h"
#include "object/control/sequencefloat.h"
#include "object/control/modulatorobjectfloat.h"
#include "object/control/derivativeobjectfloat.h"
#include "object/audio/filterao.h"
#include "object/texture/blurto.h"
#include "object/ascriptobject.h"
#include "object/visual/oscillograph.h"
#include "object/visual/imagegallery.h"
#include "object/textobject.h"
#include "object/control/oscinputobject.h"
#ifndef MO_DISABLE_SPATIAL
#   include "object/synthesizer.h"
#   include "object/microphonegroup.h"
#endif
#include "io/error.h"

namespace MO {

struct AppIcons::Private
{
    enum IconId
    {
        I_NONE,
        I_3D,
        I_GEOMETRY,
        I_PARAMETER,
        I_SOUNDSOURCE,
        I_MICROPHONE,
        I_CAMERA,
        I_TRACK,
        I_TRANSLATION,
        I_ROTATION,
        I_SCALE,
        I_SHEAR,
        I_LOOK,
        I_LOOKAT,
        I_MIX,
        I_CLEAR,
        I_MIRROR,
        I_INPUT,
        I_GROUP,
        I_LIGHT,
        I_MODULATOR,
        I_AUDIO,
        I_AU_ENV,           // XXX AudioUnits not present any more
        I_AU_FILTER,
        I_AU_FILTERBANK,
        I_MUSIC_NOTE,
        I_CLIP,
        I_CLIP_CONTROL,
        I_ANGELSCRIPT,
        I_OSCILLOGRAPH,
        I_GLSL,
        I_DERIVATIVE,
        I_TEX,
        I_TO_BLUR,
        I_TEXT,
        I_MICROPHONE_GROUP,
        I_GALLERY,
        I_OSCIN
    };

    struct IconDesc { IconId id; QString name; };


    static Private * instance_;
    QMap <IconId, QString> nameMap_;
    QColor defColor;
    QSize defSize;

    static Private * instance()
    {
        if (!instance_)
            instance_ = new Private();
        return instance_;
    }

    // ---------------------------------- //

    Private();

    void init();

    IconId idForObject(const Object * obj) const;
    IconId idForType(int type) const;

    QIcon getIcon(IconId id, const QColor& color, const QSize& size) const;
};

AppIcons::Private * AppIcons::Private::instance_ = 0;

AppIcons::Private::Private()
    : defColor      (200,200,200)
    , defSize       (64, 64)
{
    init();
}

void AppIcons::Private::init()
{
    nameMap_.insert(I_NONE, ":/icon/obj_none.png");
    nameMap_.insert(I_3D, ":/icon/obj_3d.png");
    nameMap_.insert(I_GEOMETRY, ":/icon/obj_geometry.png");
    nameMap_.insert(I_PARAMETER, ":/icon/obj_parameter.png");
    nameMap_.insert(I_SOUNDSOURCE, ":/icon/obj_soundsource.png");
    nameMap_.insert(I_MICROPHONE, ":/icon/obj_microphone.png");
    nameMap_.insert(I_CAMERA, ":/icon/obj_camera.png");
    nameMap_.insert(I_TRACK, ":/icon/obj_track.png");
    nameMap_.insert(I_TRANSLATION, ":/icon/obj_translation.png");
    nameMap_.insert(I_ROTATION, ":/icon/obj_rotation.png");
    nameMap_.insert(I_SCALE, ":/icon/obj_scale.png");
    nameMap_.insert(I_SHEAR, ":/icon/obj_shear.png");
    nameMap_.insert(I_LOOK, ":/icon/obj_look.png");
    nameMap_.insert(I_LOOKAT, ":/icon/obj_lookat.png");
    nameMap_.insert(I_MIX, ":/icon/obj_mix.png");
    nameMap_.insert(I_CLEAR, ":/icon/obj_trans_clear.png");
    nameMap_.insert(I_MIRROR, ":/icon/obj_trans_mirror.png");
    nameMap_.insert(I_INPUT, ":/icon/obj_trans_input.png");
    nameMap_.insert(I_GROUP, ":/icon/obj_group.png");
    nameMap_.insert(I_LIGHT, ":/icon/obj_light.png");
    nameMap_.insert(I_MODULATOR, ":/icon/obj_modulator.png");
    nameMap_.insert(I_AU_ENV, ":/icon/obj_au_env.png");
    nameMap_.insert(I_AU_FILTER, ":/icon/obj_au_filter.png");
    nameMap_.insert(I_AU_FILTERBANK, ":/icon/obj_au_filterbank.png");
    nameMap_.insert(I_MUSIC_NOTE, ":/icon/music_note.png");
    nameMap_.insert(I_CLIP, ":/icon/obj_clip.png");
    nameMap_.insert(I_CLIP_CONTROL, ":/icon/obj_clipcontroller.png");
    nameMap_.insert(I_AUDIO, ":/icon/obj_audio.png");
    nameMap_.insert(I_ANGELSCRIPT, ":/icon/obj_angelscript.png");
    nameMap_.insert(I_OSCILLOGRAPH, ":/icon/obj_oscillograph.png");
    nameMap_.insert(I_GLSL, ":/icon/obj_glsl.png");
    nameMap_.insert(I_DERIVATIVE, ":/icon/obj_derivative.png");
    nameMap_.insert(I_TEX, ":/icon/obj_tex.png");
    nameMap_.insert(I_TO_BLUR, ":/icon/obj_to_blur.png");
    nameMap_.insert(I_TEXT, ":/icon/obj_text.png");
    nameMap_.insert(I_MICROPHONE_GROUP, ":/icon/obj_microphone_group.png");
    nameMap_.insert(I_GALLERY, ":/icon/obj_gallery.png");
    nameMap_.insert(I_OSCIN, ":/icon/obj_oscin.png");
}


AppIcons::Private::IconId AppIcons::Private::idForObject(const Object * o) const
{
    if (o->isTransformation())
    {
        if (qobject_cast<const Translation*>(o))
            return I_TRANSLATION;
        if (qobject_cast<const AxisRotation*>(o))
            return I_ROTATION;
        if (qobject_cast<const Scale*>(o))
            return I_SCALE;
        if (qobject_cast<const Shear*>(o))
            return I_SHEAR;
        if (qobject_cast<const Look*>(o))
            return I_LOOK;
        if (qobject_cast<const LookAt*>(o))
            return I_LOOKAT;
        if (qobject_cast<const Mix*>(o))
            return I_MIX;
        if (qobject_cast<const ClearTrans*>(o))
            return I_CLEAR;
        if (qobject_cast<const MirrorTrans*>(o))
            return I_MIRROR;
        if (qobject_cast<const TransformationInput*>(o))
            return I_INPUT;
    }

    if (o->isAudioObject())
    {
        //if (qobject_cast<const FilterAO*>(o))
        //    return I_AU_FILTER;

        return I_AUDIO;
    }

    if (qobject_cast<const OscInputObject*>(o))
        return I_OSCIN;

    if (qobject_cast<const AScriptObject*>(o))
        return I_ANGELSCRIPT;

    if (qobject_cast<const Oscillograph*>(o))
        return I_OSCILLOGRAPH;

    if (qobject_cast<const ImageGallery*>(o))
        return I_GALLERY;

    if (qobject_cast<const DerivativeObjectFloat*>(o))
        return I_DERIVATIVE;

#ifndef MO_DISABLE_SPATIAL
#ifndef MO_DISABLE_EXP
    if (qobject_cast<const MicrophoneGroup*>(o))
        return I_MICROPHONE_GROUP;
#endif
    if (qobject_cast<const Synthesizer*>(o))
            return I_MUSIC_NOTE;
#endif

    if (qobject_cast<const BlurTO*>(o))
        return I_TO_BLUR;

    if (qobject_cast<const TextObject*>(o))
        return I_TEXT;

    if (o->isGeometry()) return I_GEOMETRY;
    if (o->isClip()) return I_CLIP;
    if (o->isShader()) return I_GLSL;
    if (o->isTexture()) return I_TEX;
    if (o->isClipController()) return I_CLIP_CONTROL;
    if (o->type() & Object::T_GROUP) return I_GROUP;
    if (o->isTrack()) return I_TRACK;
    if (o->type() & Object::TG_FLOAT) return I_PARAMETER;
    if (o->isCamera()) return I_CAMERA;
    if (o->isMicrophone()) return I_MICROPHONE;
    if (o->isSoundSource()) return I_SOUNDSOURCE;
    if (o->isGl()) return I_3D;
    if (o->isParameter()) return I_PARAMETER;
    if (o->isLightSource()) return I_LIGHT;
    if (o->isModulatorObject()) return I_MODULATOR;

    return I_NONE;
}

AppIcons::Private::IconId AppIcons::Private::idForType(int type) const
{
    switch (type)
    {
        case Object::T_OBJECT: return I_3D;
        case Object::T_GEOMETRY: return I_GEOMETRY;
        case Object::T_TRANSFORMATION: return I_TRANSLATION;
        case Object::T_MICROPHONE: return I_MICROPHONE;
        case Object::T_CAMERA: return I_CAMERA;
        case Object::T_SOUNDSOURCE: return I_SOUNDSOURCE;
        case Object::T_CLIP: return I_CLIP;
        case Object::T_CLIP_CONTROLLER: return I_CLIP_CONTROL;
        case Object::T_AUDIO_OBJECT: return I_AUDIO;
        case Object::T_ANGELSCRIPT: return I_ANGELSCRIPT;
        case Object::T_SHADER: return I_GLSL;
        case Object::T_TEXTURE: return I_TEX;
    }
    if (type & Object::TG_TRACK) return I_TRACK;
    if (type & Object::TG_FLOAT) return I_PARAMETER;

    return I_NONE;
}


QIcon AppIcons::Private::getIcon(IconId id, const QColor& color, const QSize& size) const
{
    MO_ASSERT(nameMap_.contains(id), "icon id out of range " << id);

    QString name = nameMap_.value(id);

    QIcon org(name);

    QPixmap pix(size);
    pix.fill(color);
    pix.setMask(org.pixmap(size).mask());

    return QIcon(pix);
}




// -------------------------------- public interface -----------------------------

void AppIcons::setDefaultColor(const QColor& color) { Private::instance()->defColor = color; }
void AppIcons::setDefaultSize(const QSize& s) { Private::instance()->defSize = s; }

QIcon AppIcons::iconForType(int type)
{
    auto p = Private::instance();

    auto id = p->idForType(type);
    return p->getIcon(id, p->defColor, p->defSize);
}

QIcon AppIcons::iconForObject(const Object * o)
{
    auto p = Private::instance();

    auto id = p->idForObject(o);
    return p->getIcon(id, p->defColor, p->defSize);
}

QIcon AppIcons::iconForObject(const Object * o, const QColor& color, const QSize& size)
{
    auto p = Private::instance();

    auto id = p->idForObject(o);
    return p->getIcon(id, color, size.isValid() ? size : p->defSize);
}


} // namespace MO
