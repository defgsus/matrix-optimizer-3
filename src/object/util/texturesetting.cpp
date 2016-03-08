/** @file texturesetting.cpp

    @brief Texture setting and allocator for Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#include <QImage>

#include "texturesetting.h"
#include "object/scene.h"
#include "object/util/scenesignals.h"
#include "object/param/parameters.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterint.h"
#include "object/param/parametertext.h"
#include "object/param/parametertexture.h"
#include "gl/texture.h"
#include "gl/framebufferobject.h"
#include "script/angelscript_image.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/imagereader.h"
#include "io/error.h"
#include "io/log.h"

using namespace gl;

namespace MO {


TextureSetting::TextureSetting(Object *parent)
    : object_       (parent)
    , paramTex_     (0)
    , paramFilename_(0)
{
}

TextureSetting::~TextureSetting()
{
}


void TextureSetting::serialize(IO::DataStream &io) const
{
    io.writeHeader("texs", 1);
}

void TextureSetting::deserialize(IO::DataStream &io)
{
    io.readHeader("texs", 1);
}

void TextureSetting::createParameters(
        const QString &id_suffix,
        ParameterTexture::InputType defaultType,
        bool normalMap)
{
    auto params = object_->params();

    params->beginEvolveGroup(false);

    paramTex_ = params->createTextureParameter("_img_tex" + id_suffix,
                                               tr("texture input"),
                                               tr("Connects to a texture from somewhere else or "
                                                  "creates a texture on-the-fly"));
    //paramTex_->setVisibleGraph(true);
    paramTex_->setInputType(defaultType);

    paramFilename_ = params->createFilenameParameter(
                "_imgfile" + id_suffix, tr("image file"), tr("Filename of the image"),
                normalMap? IO::FT_NORMAL_MAP : IO::FT_TEXTURE,
                normalMap? ":/normalmap/01.png" : ":/texture/mo_black.png");

    params->endEvolveGroup();
}


void TextureSetting::updateParameterVisibility()
{
    paramFilename_->setVisible( paramTex_->inputType() == ParameterTexture::IT_FILE );
}

void TextureSetting::getNeededFiles(IO::FileList& files)
{
    if (!paramFilename_)
        return;
    if (paramTex_->inputType() == ParameterTexture::IT_FILE)
        files.append(IO::FileListEntry(paramFilename_->value(),
                                       paramFilename_->fileType()));
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
         r = p_lastIsEnabled_ != is;
    p_lastIsCube_ = is;
    return r;
}

bool TextureSetting::checkEnabledChanged()
{
    bool is = paramTex_ && paramTex_->inputType() != ParameterTexture::IT_NONE,
         r = p_lastIsEnabled_ != is;
    p_lastIsEnabled_ = is;
    return r;
}

// ------------- opengl ---------------------

void TextureSetting::releaseGl()
{
    paramTex_->releaseGl();
}


void TextureSetting::bind(const RenderTime& time, uint slot)
{
    auto tex = paramTex_->value(time);
    if (!tex)
        return;
    p_isCube_ = tex->isCube();

    if (!tex)
        MO_GL_ERROR("No texture defined for TextureSetting::bind()");

    // ---- bind ----

    // set active slot
    slot += (uint)GL_TEXTURE0;
    GLint act;
    MO_CHECK_GL_THROW( glGetIntegerv(GL_ACTIVE_TEXTURE, &act) );
    if ((GLint)slot != act)
        MO_CHECK_GL_THROW( glActiveTexture(GLenum(slot)) );

    tex->bind();

    // ---- set texture params ----

    paramTex_->setTextureParam(tex);

    // set slot back
    if ((GLint)slot != act)
        MO_CHECK_GL_THROW( glActiveTexture(GLenum(act)) );
}


} // namespace MO
