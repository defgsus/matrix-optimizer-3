/** @file textureobjectbase.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#include <QList>

#include "textureobjectbase.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertexture.h"
#include "gl/context.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {


struct TextureObjectBase::PrivateTO
{
    PrivateTO(TextureObjectBase * to)
        : to            (to)
        , fbo           (0)
        , screenQuad    (0)
        , swapTex       (0)
        , outputTex      (0)
        , maxIns        (0)
        , alphaBlend    (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void createShaderQuad(const GL::ShaderSource& src, const QList<QString>& texNames);
    void drawFramebuffer(uint thread, Double time, int width, int height);
    void renderShaderQuad(uint index, Double time, uint thread, uint& texSlot);

    /** A quad and shader with associated uniforms */
    struct ShaderQuad
    {
        GL::ScreenQuad * quad;
        GL::Uniform
                * u_resolution,
                * u_time,
                * u_transformation;
        QList<GL::Uniform*> u_tex;
    };

    TextureObjectBase * to;

    GL::FrameBufferObject * fbo;
    GL::ScreenQuad *screenQuad;
    QList<ShaderQuad> shaderQuads;
    GL::Texture *swapTex;
    const GL::Texture * outputTex;
    uint maxIns;

    ParameterFloat * p_out_r, * p_out_g, * p_out_b, * p_out_a;
    ParameterSelect * p_magInterpol, * p_enableOut;
    ParameterInt * p_width, * p_height, * p_aa, * p_split;
    ParameterText * p_fragment;
    QList<ParameterTexture*> p_textures;

    GL::Uniform
        * u_out_color, * u_out_resolution;

    Float aspectRatio;
    AlphaBlendSetting alphaBlend;
};


TextureObjectBase::TextureObjectBase(QObject *parent)
    : ObjectGl      (parent)
    , p_to_         (new PrivateTO(this))
{
    setCreateRenderSettings(false);
}

TextureObjectBase::~TextureObjectBase()
{
    delete p_to_;
}

void TextureObjectBase::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);
    io.writeHeader("to", 1);
}

void TextureObjectBase::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);
    io.readHeader("to", 1);
}

QSize TextureObjectBase::resolution() const
{
    return p_to_->fbo
            ? QSize(p_to_->fbo->width(), p_to_->fbo->height())
            : QSize();
    /*return QSize(p_to_->p_width->baseValue(),
                 p_to_->p_height->baseValue());*/
}

Float TextureObjectBase::aspectRatio() const
{
    return p_to_->aspectRatio;
}

void TextureObjectBase::initMaximumTextureInputs(uint num)
{
    p_to_->maxIns = num;
}

const QList<ParameterTexture*>& TextureObjectBase::textureParams()
{
    return p_to_->p_textures;
}

void TextureObjectBase::createParameters()
{
    ObjectGl::createParameters();
    p_to_->createParameters();
}

void TextureObjectBase::PrivateTO::createParameters()
{

    to->params()->beginParameterGroup("to_res", tr("resolution"));

        p_width = to->params()->createIntParameter("to_width", tr("width"), tr("Width of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);
        p_height = to->params()->createIntParameter("to_height", tr("height"), tr("Height of rendered frame in pixels"),
                                      1024, 16, 4096*4, 16, true, false);

        p_split = to->params()->createIntParameter("to_split", tr("segments"),
                                    tr("Split rendering of the output into separate regions for better gui response"),
                                    1, 1, 4096, 1, true, false);

    to->params()->endParameterGroup();

    to->params()->beginParameterGroup("to_input", tr("input"));
    if (maxIns)
        to->initParameterGroupExpanded("to_input");

        for (uint i=0; i<maxIns; ++i)
        {
            p_textures.push_back( to->params()->createTextureParameter(
                                            QString("to_tex%1").arg(i),
                                            tr("texture %1").arg(i+1),
                                            tr("A texture input")) );
        }

    to->params()->endParameterGroup();

    to->params()->beginParameterGroup("to_output", tr("output"));

        p_enableOut = to->params()->createBooleanParameter("to_master_out", tr("enable"),
                           tr("Enables or disables sampling the output to the main framebuffer"),
                           tr("The texture object will render internally but not contribute to the main framebuffer"),
                           tr("The texture object will render it's output ontop the main framebuffer"),
                           false, true, true);

        p_out_r = to->params()->createFloatParameter("to_red", "red", tr("Red amount of output color"), 1.0, 0.1);
        p_out_g = to->params()->createFloatParameter("to_green", "green", tr("Green amount of output color"), 1.0, 0.1);
        p_out_b = to->params()->createFloatParameter("to_blue", "blue", tr("Blue amount of output color"), 1.0, 0.1);
        p_out_a = to->params()->createFloatParameter("to_alpha", tr("alpha"),
                      tr("Defines the opaqueness/transparency of the output [0,1]"),
                      1.0,
                      0.0, 1.0, 0.05);

        alphaBlend.createParameters(AlphaBlendSetting::M_MIX, false, "to_", "");

        p_magInterpol = to->params()->createBooleanParameter("to_cammaginterpol", tr("interpolation"),
                                                tr("The interpolation mode for pixel magnification"),
                                                tr("No interpolation"),
                                                tr("Linear interpolation"),
                                                true,
                                                true, false);

        p_aa = to->params()->createIntParameter("to_outaa", tr("anti-aliasing"),
                      tr("Sets the super-sampling when drawing the rendered frame onto the output"),
                      1,
                      1, 16, 1, true, false);

    to->params()->endParameterGroup();
}

void TextureObjectBase::onParameterChanged(Parameter * p)
{
    ObjectGl::onParameterChanged(p);

    if (p == p_to_->p_width
        || p == p_to_->p_height
        || p == p_to_->p_aa)
        requestReinitGl();
}

void TextureObjectBase::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();

}

void TextureObjectBase::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();


}

GL::FrameBufferObject * TextureObjectBase::fbo() const
{
    return p_to_->fbo;
}

const GL::Texture * TextureObjectBase::valueTexture(Double , uint ) const
{
    if (p_to_->outputTex)
        return p_to_->outputTex;
    return fbo() ? fbo()->colorTexture() : 0;
}

void TextureObjectBase::initGl(uint ) { p_to_->initGl(); }
void TextureObjectBase::releaseGl(uint ) { p_to_->releaseGl(); }
GL::ScreenQuad * TextureObjectBase::createShaderQuad(const GL::ShaderSource& src, const QList<QString>& texNames)
{
    p_to_->createShaderQuad(src, texNames);
    return p_to_->shaderQuads.back().quad;
}

void TextureObjectBase::drawFramebuffer(uint thread, Double time, int width, int height)
{
    p_to_->drawFramebuffer(thread, time, width, height);
}
void TextureObjectBase::renderShaderQuad(uint index, Double time, uint thread, uint& texSlot)
{
    p_to_->renderShaderQuad(index, time, thread, texSlot);
}

void TextureObjectBase::PrivateTO::initGl()
{
    // size of frame

    int width = p_width->baseValue(),
        height = p_height->baseValue();

    aspectRatio = (Float)width/std::max(1, height);


    // screen-quad

    int aa = p_aa->baseValue();
    const bool doAa = aa > 1;

    QString defines;
    defines += QString("#define MO_USE_COLOR");
    if (doAa)
        defines += QString("\n#define MO_ANTIALIAS (%1)").arg(aa);

    screenQuad = new GL::ScreenQuad(to->name() + "_outquad", GL::ER_THROW);
    try
    {
        screenQuad->create(
                    ":/shader/framebufferdraw.vert",
                    ":/shader/framebufferdraw.frag",
                    defines);
    }
    catch (Exception&)
    {
        releaseGl();
        throw;
    }

    // uniforms

    u_out_color = screenQuad->shader()->getUniform("u_color", true);
    u_out_color->setFloats(1,1,1,1);
    if (doAa)
        u_out_resolution = screenQuad->shader()->getUniform("u_resolution", true);
    else
        u_out_resolution = 0;

    // create framebuffer

    outputTex = 0;
    fbo = new GL::FrameBufferObject(
                width,
                height,
                gl::GL_RGBA,
                gl::GL_FLOAT,
                0,//GL::FrameBufferObject::A_DEPTH,
                false,
                GL::ER_THROW);
    fbo->setName(to->name());

    try
    {
        fbo->create();
    }
    catch (Exception&)
    {
        releaseGl();
        throw;
    }
    fbo->unbind();
}

void TextureObjectBase::PrivateTO::releaseGl()
{
    for (auto & q : shaderQuads)
        q.quad->release();
    shaderQuads.clear();

    if (screenQuad)
        screenQuad->release();
    delete screenQuad;
    screenQuad = 0;

    if (swapTex)
        swapTex->release();
    delete swapTex;
    swapTex = 0;

    if (fbo)
        fbo->release();
    delete fbo;
    fbo = 0;
    outputTex = 0;
}

GL::ScreenQuad * TextureObjectBase::shaderQuad(uint index)
{
    return index < (uint)p_to_->shaderQuads.size() ? p_to_->shaderQuads[index].quad : 0;
}

void TextureObjectBase::PrivateTO::createShaderQuad(
        const GL::ShaderSource& csrc, const QList<QString>& texNames)
{
    // create quad and compile shader

    ShaderQuad quad;
    const QString qname = to->name() + QString("_shaderquad%1").arg(shaderQuads.size());
    quad.quad = new GL::ScreenQuad(qname, GL::ER_THROW);

    auto src = new GL::ShaderSource(csrc);

    // resolve includes
    src->replaceIncludes([this](const QString& url, bool do_search)
    {
        QString inc = to->getGlslInclude(url, do_search);
        return inc.isEmpty() ? inc : ("// ----- include '" + url + "' -----\n" + inc);
    });

    try
    {
        quad.quad->create(src, 0);
        auto shader = screenQuad->shader();

        // uniforms

        quad.u_time = shader->getUniform("u_time", false);
        quad.u_resolution = shader->getUniform("u_resolution", false);
        quad.u_transformation = shader->getUniform("u_transformation", false);

        for (auto & n : texNames)
            quad.u_tex << shader->getUniform(n, false);

        shaderQuads << quad;

    }
    catch (Exception& )
    {
        // XXX Should send errors to gui/module somehow

        // clean-up
        delete quad.quad;

        throw;
    }
}


void TextureObjectBase::PrivateTO::renderShaderQuad(uint index, Double time, uint thread, uint& texSlot)
{
    if (!fbo || index >= (uint)shaderQuads.size())
        return;

    const ShaderQuad & quad = shaderQuads[index];

    const auto res = to->resolution();

    fbo->bind();

    // exchange render target for multi-stage rendering
    if (shaderQuads.size() > 1)
    {
        // create a swap buffer
        if (!swapTex)
        {
            swapTex = GL::Texture::constructFrom(fbo->colorTexture());
            swapTex->create();
        }
        swapTex = fbo->swapColorTexture(swapTex);
    }

    //if (index == 0)
    {
        MO_CHECK_GL( gl::glViewport(0, 0, res.width(), res.height()) );
        MO_CHECK_GL( gl::glClearColor(0,0,0,0) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );
    }

    // --- set shader uniforms ---

    if (quad.u_time)
        quad.u_time->floats[0] = time;
    if (quad.u_resolution)
        quad.u_resolution->setFloats(res.width(), res.height());

    // --- bind textures ---

    for (int i=0; i<p_textures.length(); ++i)
    {
        const GL::Texture * tex;

        // bind the last frame as input to later stages
        if (i == 0 && index > 0)
            tex = swapTex;
        else
            tex = p_textures[i]->value(time, thread);
        if (tex)
        {
            // bind to slot x
            MO_CHECK_GL( gl::glActiveTexture(gl::GL_TEXTURE0 + texSlot) );
            tex->bind();
            // tell shader
            if (i < quad.u_tex.length() && quad.u_tex[i])
                quad.u_tex[i]->ints[0] = texSlot;

            ++texSlot;
        }
    }
    MO_CHECK_GL( gl::glActiveTexture(gl::GL_TEXTURE0) );

    // --- render ---

    MO_EXTEND_EXCEPTION(
        quad.quad->draw(res.width(), res.height(), p_split->baseValue())
                , "in TextureObjectBase::renderGl()")

    gl::glFlush();
    gl::glFinish();

    // get the actual output of last stage
    outputTex = fbo->colorTexture();

    fbo->unbind();
}


void TextureObjectBase::PrivateTO::drawFramebuffer(uint thread, Double time, int width, int height)
{
    if (p_enableOut->value(time, thread) == 0)
        return;

    gl::glFinish();

    // get output texture
    auto tex = to->valueTexture(time, thread);
    if (!tex)
        return;
    MO_PRINT(tex->name());

    // -- shader uniforms --

    if (u_out_color)
        u_out_color->setFloats(
                    p_out_r->value(time, thread),
                    p_out_g->value(time, thread),
                    p_out_b->value(time, thread),
                    p_out_a->value(time, thread));

    if (u_out_resolution)
        u_out_resolution->setFloats(width, height,
                             1.f / std::max(1, width),
                             1.f / std::max(1, height));

    // -- render fbo frame onto current context --

    // set blendmode
    alphaBlend.apply(time, thread);

    // bind the color texture from the fbo

    MO_CHECK_GL( glActiveTexture(GL_TEXTURE0) );
    tex->bind();

    // set interpolation mode
    if (p_magInterpol->baseValue())
        tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    else
        tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));

    // set edge-clamp
    // XXX not sure if needed or if should be parameterized..
    tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
    tex->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));

    MO_EXTEND_EXCEPTION(
        // draw the texture
        screenQuad->drawCentered(width, height, aspectRatio)
                , "in TextureObjectBase::drawFrameBuffer()");

    gl::glFinish();
}



} // namespace MO
