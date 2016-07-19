/** @file texturesetting.cpp

    @brief Texture setting and allocator for Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#include <QImage>

#include "TextureSetting.h"
#include "object/Scene.h"
#include "object/util/SceneSignals.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFilename.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterText.h"
#include "object/param/ParameterTexture.h"
#include "gl/Texture.h"
#include "gl/FrameBufferObject.h"
#include "script/angelscript_image.h"
#include "io/FileManager.h"
#include "io/DataStream.h"
#include "io/ImageReader.h"
#include "io/error.h"
#include "io/log.h"

using namespace gl;

namespace MO {


TextureSetting::TextureSetting(Object *parent)
    : object_       (parent)
    , paramTex_     (0)
    , paramFilename_(0)
    , p_isCube_     (false)
    , p_lastIsCube_ (false)
    , p_lastIsEnabled_(false)
    , loaded_version(-1)
{
}

TextureSetting::~TextureSetting()
{
}


void TextureSetting::serialize(IO::DataStream &io) const
{
    io.writeHeader("texs", 2);
    // v2 moved most work to ParameterTexture (no binary change)
}

void TextureSetting::deserialize(IO::DataStream &io)
{
    loaded_version = io.readHeader("texs", 2);
}

void TextureSetting::createParameters(const QString& id,
        const QString& name,
        ParameterTexture::InputType defaultType,
        bool normalMap)
{
//    MO_PRINT("TextureSetting(" << object_->name() << ") createParameters");

    auto params = object_->params();

    params->beginEvolveGroup(false);

    paramTex_ = params->createTextureParameter(
                id, name,
                tr("Connects to a texture from somewhere else or "
                   "creates a texture on-the-fly"));
    //paramTex_->setVisibleGraph(true);
    paramTex_->setInputType(defaultType);
    paramTex_->setDefaultInputType(defaultType);

    paramFilename_ = params->createFilenameParameter(
                id + "_filename", tr("image file"), tr("Filename of the image"),
                normalMap? IO::FT_NORMAL_MAP : IO::FT_TEXTURE,
                normalMap? ":/normalmap/01.png" : ":/texture/mo_black.png");
    paramTex_->setFilenameParameter(paramFilename_);

    params->endEvolveGroup();

//    MO_PRINT("TextureSetting(" << object_->name() << ") createParameters finish");
}


void TextureSetting::updateParameterVisibility()
{
    paramFilename_->setVisible(
                paramTex_->isVisible() &&
                paramTex_->inputType() == ParameterTexture::IT_FILE );
}

void TextureSetting::setVisible(bool v)
{
    paramTex_->setVisible(v);
    updateParameterVisibility();
}

void TextureSetting::getNeededFiles(IO::FileList& files)
{
    if (!paramFilename_)
        return;
    if (paramTex_->inputType() == ParameterTexture::IT_FILE)
        files.append(IO::FileListEntry(paramFilename_->value(),
                                       paramFilename_->fileType()));
}

void TextureSetting::fixCompatibility()
{
    // make a guess about the source of texture
    if (loaded_version == 1)
    {
        if (!paramTex_->modulatorIds().isEmpty())
            paramTex_->setInputType(ParameterTexture::IT_INPUT);
        else
        if (paramFilename_->value() != paramFilename_->defaultValue())
            paramTex_->setInputType(ParameterTexture::IT_FILE);

        loaded_version = -1;
    }
}

// --------------- getter -------------------

bool TextureSetting::isCube() const
{
    // XXX Return the last known state from bind()
    return p_lastIsCube_ = p_isCube_;
}

bool TextureSetting::isMipmap() const
{
    return paramTex_ && paramTex_->isMipmap();
}

bool TextureSetting::isEnabled() const
{
    return p_lastIsEnabled_ =
            paramTex_ && paramTex_->inputType() != ParameterTexture::IT_NONE;
}

// ------- stateful getter ------------------

bool TextureSetting::checkCubeChanged()
{
    bool is = p_isCube_,
         r = p_lastIsCube_ != is;
    p_lastIsCube_ = is;
    return r;
}

bool TextureSetting::checkEnabledChanged()
{
    if (!paramTex_)
        return false;
    bool is = paramTex_->inputType() != ParameterTexture::IT_NONE,
         r = p_lastIsEnabled_ != is;
    p_lastIsEnabled_ = is;
    return r;
}

bool TextureSetting::checkFilenameChanged()
{
    if (!paramFilename_)
        return false;
    bool r = p_lastFilename_ != paramFilename_->value();
    p_lastFilename_ = paramFilename_->value();
    return r;
}

bool TextureSetting::checkAnyChanged()
{
    // Important: compare bitwise to call each function and reset checked-states
    return checkEnabledChanged() | checkFilenameChanged() | checkCubeChanged();
}

bool TextureSetting::onParameterChange(Parameter *p)
{
    if (p == paramFilename_)
    {
        return checkFilenameChanged();
    }
    if (p == paramTex_)
    {
        return checkAnyChanged();
    }
    return false;
}

// ------------- opengl ---------------------

void TextureSetting::releaseGl()
{
    paramTex_->releaseGl();
}


void TextureSetting::bind(const RenderTime& time, uint* slot)
{
    auto tex = paramTex_->value(time);
    if (!tex)
        return;
    p_isCube_ = tex->isCube();


    // ---- bind ----

    if (slot)
    {
        tex->setActiveTexture(*slot);
        ++(*slot);
    }
    tex->bind();

    // ---- set texture params ----

    paramTex_->applyTextureParam(tex);
}


} // namespace MO
