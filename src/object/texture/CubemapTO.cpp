/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/10/2016</p>
*/

#include "CubemapTO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterTexture.h"
#include "gl/Texture.h"
#include "io/FileManager.h"
#include "io/DataStream.h"
#include "io/log.h"
#include "io/streamoperators_glbinding.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(CubemapTO)

CubemapTO::CubemapTO()
    : TextureObjectBase ()
    , cubeTex_          (0)
{
    setName("Cubemap");
    initMaximumTextureInputs(6);
}

CubemapTO::~CubemapTO()
{
    delete cubeTex_;
}

void CubemapTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("tocubem", 1);
}

void CubemapTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("tocubem", 1);
}

void CubemapTO::createParameters()
{
    TextureObjectBase::createParameters();

}

void CubemapTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (false)
        requestReinitGl();
}

void CubemapTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void CubemapTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();
}

void CubemapTO::getNeededFiles(IO::FileList & files)
{
    TextureObjectBase::getNeededFiles(files);

}

void CubemapTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);

}

void CubemapTO::releaseGl(uint thread)
{
    if (cubeTex_ && cubeTex_->isAllocated())
        cubeTex_->release();
    delete cubeTex_;
    cubeTex_ = 0;

    TextureObjectBase::releaseGl(thread);
}

const GL::Texture * CubemapTO::valueTexture(uint chan, const RenderTime& time) const
{
    if (chan != 0)
        return nullptr;

    if (cubeTex_ == nullptr || hasInputChanged_(time))
        const_cast<CubemapTO*>(this)->buildCubemap_(time);

    return cubeTex_;
}

bool CubemapTO::hasInputChanged_(const RenderTime& time) const
{
    bool r = false;
    for (int i=0; i<6; ++i)
        r |= textureParams()[i]->hasChanged(time);
    return r;
}

void CubemapTO::buildCubemap_(const RenderTime& rt)
{
    clearError();

    std::vector<const GL::Texture*> texs;

    QSize res;
    for (int i=0; i<6; ++i)
    {
        auto tex = inputTexture(i, rt);
        if (!tex)
        {
            //setErrorMessage(tr("texture #%1 not assigned").arg(i));
            //return;
            texs.push_back(nullptr);
            continue;
        }
        // check type
        if (tex->isCube() || tex->is3d())
        {
            setErrorMessage(tr("texture #%1 is not 2D").arg(i));
            return;
        }
        // check resolution
        QSize tres(tex->width(), tex->height());
        if (!res.isValid())
            res = tres;
        else if (res != tres)
        {
            setErrorMessage(tr("texture #%1 has different size %2x%3, expected %4x%5")
                            .arg(i)
                            .arg(tres.width()).arg(tres.height())
                            .arg(res.width()).arg(res.height()));
            return;
        }
        // check format
        if (!texs.empty() && (tex->format() != texs.back()->format()))
        {
            std::stringstream s1; s1 << tex->format();
            std::stringstream s2; s2 << texs.back()->format();
            setErrorMessage(tr("texture #%1 has different format %2, expected %3")
                            .arg(i).arg(s2.str().c_str()).arg(s1.str().c_str()));
            return;
        }

        texs.push_back(tex);
    }
    try
    {
        if (cubeTex_ && cubeTex_->isAllocated())
        {
            if (cubeTex_->format() != texs.back()->format())
            {
                cubeTex_->release();
                delete cubeTex_;
                cubeTex_ = 0;
            }
        }

        if (!cubeTex_)
            cubeTex_ = new GL::Texture(
                        res.width(), res.height(),
                        texs.back()->format(),
                        gl::GL_RGBA, gl::GL_FLOAT,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

        cubeTex_->setName("cubemap_out");
        cubeTex_->create();

        for (size_t i=0; i<texs.size(); ++i)
        if (texs[i] != nullptr)
        {
            MO_CHECK_GL_THROW(
                gl::glCopyImageSubData(
                    texs[i]->handle(), texs[i]->target(),
                    0, 0, 0, 0,
                    cubeTex_->handle(), cubeTex_->target(),
                            //gl::GLenum(gl::GL_TEXTURE_CUBE_MAP_POSITIVE_X + i),
                    0, 0, 0, i,
                    res.width(), res.height(), 1)
            );
        }
    }
    catch (const Exception& e)
    {
        if (cubeTex_->isHandle())
            cubeTex_->release();
        delete cubeTex_;
        cubeTex_ = 0;

        setErrorMessage(tr("Failed to create cubemap: %1").arg(e.what()));
    }
}


} // namespace MO

