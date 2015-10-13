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
#include "object/util/objecteditor.h"
#include "gl/context.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/datastream.h"
#include "io/log.h"

#if 0
#   define MO_TO_DEBUG(arg__) MO_DEBUG("TextureObjectBase(" << name() << ")::" << arg__)
#else
#   define MO_TO_DEBUG(unused__)
#endif

using namespace gl;

namespace MO {


struct TextureObjectBase::PrivateTO
{
    PrivateTO(TextureObjectBase * to)
        : to            (to)
        , fbo           (0)
        , screenQuad    (0)
        , swapTex       (0)
        , outputTex     (0)
        , maxIns        (0)
        , fboDepth      (1)
        , hasColorRange (false)
        , alphaBlend    (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void createShaderQuad(const GL::ShaderSource& src, const QList<QString>& texNames);
    void createFbo(const QSize& s, uint depth = 1);
    void drawFramebuffer(uint thread, Double time, int width, int height);
    void renderShaderQuad(uint index, Double time, uint thread, uint& texSlot);
    const QString& name() const { return to->name(); } // for debug

    /** A quad and shader with associated uniforms */
    struct ShaderQuad
    {
        GL::ScreenQuad * quad;
        GL::Uniform
                * u_resolution,
                * u_time,
                * u_transformation,
                * u_color_range_min,
                * u_color_range_max;
        QList<GL::Uniform*> u_tex;
    };

    TextureObjectBase * to;

    GL::FrameBufferObject * fbo;
    GL::ScreenQuad *screenQuad;
    QList<ShaderQuad> shaderQuads;
    GL::Texture *swapTex;
    const GL::Texture * outputTex;
    uint maxIns, fboDepth;
    QStringList inpNames;
    bool hasColorRange;
    QList<GL::Shader::CompileMessage> lastMessages;

    ParameterFloat  * p_out_r, * p_out_g, * p_out_b, * p_out_a,
                    * p_r_min, * p_r_max,
                    * p_g_min, * p_g_max,
                    * p_b_min, * p_b_max,
                    * p_a_min, * p_a_max,
                    * p_res_scale;
    ParameterSelect * p_magInterpol, * p_enableOut,
                * p_texType, * p_texFormat, *p_resMode;
    ParameterInt * p_width, * p_height, * p_depth, * p_aa, * p_split;
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
    initCreateRenderSettings(false);
    initDefaultUpdateMode(UM_ON_CHANGE);
    setNumberOutputs(ST_TEXTURE, 1);
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

uint TextureObjectBase::numberTextureInputs() const
{
    return p_to_->maxIns;
}

bool TextureObjectBase::hasTextureInput(uint index) const
{
    if (index >= (uint)p_to_->p_textures.size())
        return false;
    return p_to_->p_textures[index]->isModulated();
}

void TextureObjectBase::setEnableMasterOut(bool enable, bool sendGui)
{
    if (sendGui)
    {
        auto e = editor();
        MO_ASSERT(e, "can't set parameter");
        e->setParameterValue(p_to_->p_enableOut, enable);
    }
    else
        p_to_->p_enableOut->setValue(enable);
}

void TextureObjectBase::setResolutionMode(ResolutionMode mode, bool sendGui)
{
    if (sendGui)
    {
        auto e = editor();
        MO_ASSERT(e, "can't set parameter");
        e->setParameterValue(p_to_->p_resMode, mode);
    }
    else
        p_to_->p_resMode->setValue(mode);
}


void TextureObjectBase::initMaximumTextureInputs(uint num)
{
    p_to_->maxIns = num;
}

void TextureObjectBase::initMaximumTextureInputs(const QStringList& names)
{
    p_to_->maxIns = names.size();
    p_to_->inpNames = names;
}

void TextureObjectBase::initEnableColorRange(bool e)
{
    p_to_->hasColorRange = e;
}

void TextureObjectBase::init3dFramebuffer(uint depth)
{
    p_to_->fboDepth = depth;
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

    to->params()->beginParameterGroup("to_res", tr("resolution and format"));

        p_resMode  = to->params()->createSelectParameter("to_res_mode", tr("resolution"),
                    tr("Selects how the resolution is defined"),
                    { "custom", "input", "scaled", "fix_width", "fix_height" },
                    { tr("custom"), tr("input"), tr("input scaled"),
                      tr("input ratio width"), tr("input ratio height") },
                    { tr("Resolution can be freely set"),
                      tr("The resolution from the input texture is used"),
                      tr("A scaled resolution with same ratio as the input texture is used") },
                    { RM_CUSTOM, RM_INPUT, RM_INPUT_SCALED,
                      RM_INPUT_FIX_WIDTH, RM_INPUT_FIX_HEIGHT },
                    RM_INPUT,
                    true, false);

        p_width = to->params()->createIntParameter("to_width", tr("width"), tr("Width of texture in pixels"),
                                      1024, 16, 4096*4, 16, true, false);
        p_height = to->params()->createIntParameter("to_height", tr("height"), tr("Height of texture in pixels"),
                                      1024, 16, 4096*4, 16, true, false);
        p_depth = to->params()->createIntParameter("to_depth", tr("depth"), tr("Depth of texture in pixels"),
                                      1024, 16, 4096*4, 16, true, false);
        p_res_scale = to->params()->createFloatParameter("to_res_scale",
                        tr("scale"), tr("A multiplier for the input texture"),
                                                         1.f, 0.125f, true, false);
        p_res_scale->setMinValue(0.f);

        p_texFormat = to->params()->createTextureFormatParameter("to_format", tr("texture format"),
                                                    tr("The channel format of the output texture"));
        p_texType = to->params()->createTextureTypeParameter("to_type", tr("texture type"),
                                                    tr("The type-per-channel of the output texture"));

        p_split = to->params()->createIntParameter("to_split", tr("segments"),
                                    tr("Split rendering of the output into separate regions for better gui response"),
                                    1, 1, 4096, 1, true, false);
        /** @todo fix split/segmentation in TextureObjectBase */
        p_split->setZombie(true);

    to->params()->endParameterGroup();

    to->params()->beginParameterGroup("to_input", tr("input"));
    if (maxIns)
        to->initParameterGroupExpanded("to_input");

        for (uint i=0; i<maxIns; ++i)
        {
            p_textures.push_back( to->params()->createTextureParameter(
                                            QString("to_tex%1").arg(i),
                                            i < (uint)inpNames.size()
                                                ? inpNames[i]
                                                : tr("input %1").arg(i+1),
                                            tr("A texture input")) );

            p_textures.back()->setVisibleGraph(true);
        }

    to->params()->endParameterGroup();

    if (hasColorRange)
    {
        to->params()->beginParameterGroup("to_crange", tr("color range"));
        p_r_min = to->params()->createFloatParameter("to_red_min", tr("red min"),
                                                     tr("Minimum value of channel"), 0.0, 0.05);
        p_r_max = to->params()->createFloatParameter("to_red_max", tr("red max"),
                                                     tr("Maximum value of channel"), 1.0, 0.05);
        p_g_min = to->params()->createFloatParameter("to_green_min", tr("green min"),
                                                     tr("Minimum value of channel"), 0.0, 0.05);
        p_g_max = to->params()->createFloatParameter("to_green_max", tr("green max"),
                                                     tr("Maximum value of channel"), 1.0, 0.05);
        p_b_min = to->params()->createFloatParameter("to_blue_min", tr("blue min"),
                                                     tr("Minimum value of channel"), 0.0, 0.05);
        p_b_max = to->params()->createFloatParameter("to_blue_max", tr("blue max"),
                                                     tr("Maximum value of channel"), 1.0, 0.05);
        p_a_min = to->params()->createFloatParameter("to_alpha_min", tr("alpha min"),
                                                     tr("Minimum value of channel"), 0.0, 0.05);
        p_a_max = to->params()->createFloatParameter("to_alpha_max", tr("alpha max"),
                                                     tr("Maximum value of channel"), 1.0, 0.05);
        to->params()->endParameterGroup();
    }

    to->params()->beginParameterGroup("to_output", tr("master output"));

        p_enableOut = to->params()->createBooleanParameter("to_master_out", tr("enable"),
                           tr("Enables or disables sampling the output to the main framebuffer"),
                           tr("The texture object will render internally but not contribute to the main framebuffer"),
                           tr("The texture object will render it's output ontop the main framebuffer"),
                           false, true, true);

        p_out_r = to->params()->createFloatParameter("to_red", tr("red"), tr("Red amount of output color"), 1.0, 0.1);
        p_out_g = to->params()->createFloatParameter("to_green", tr("green"), tr("Green amount of output color"), 1.0, 0.1);
        p_out_b = to->params()->createFloatParameter("to_blue", tr("blue"), tr("Blue amount of output color"), 1.0, 0.1);
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
        || p == p_to_->p_res_scale
        || p == p_to_->p_resMode
        || p == p_to_->p_depth
        || p == p_to_->p_aa
        || p == p_to_->p_texFormat
        || p == p_to_->p_texType)
        requestReinitGl();
}

void TextureObjectBase::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();

}

void TextureObjectBase::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    const auto resMode = (ResolutionMode)p_to_->p_resMode->baseValue();
    bool res = resMode == RM_CUSTOM;
    p_to_->p_width->setVisible(res || resMode == RM_INPUT_FIX_WIDTH);
    p_to_->p_height->setVisible(res || resMode == RM_INPUT_FIX_HEIGHT);
    p_to_->p_res_scale->setVisible(resMode == RM_INPUT_SCALED);
    p_to_->p_depth->setVisible(res && false);
}

const QList<GL::Shader::CompileMessage>& TextureObjectBase::compileMessages() const
{
    return p_to_->lastMessages;
}

GL::FrameBufferObject * TextureObjectBase::fbo() const
{
    return p_to_->fbo;
}

GL::ShaderSource TextureObjectBase::shaderSource(uint index) const
{
    auto s = shaderQuad(index);
    return s ? *s->shader()->source() : GL::ShaderSource();
}

const GL::Texture * TextureObjectBase::valueTexture(uint chan, Double , uint ) const
{
    if (chan != 0)
        return 0;
    if (p_to_->outputTex)
        return p_to_->outputTex;
    return fbo() ? fbo()->colorTexture() : 0;
}

void TextureObjectBase::initGl(uint ) { p_to_->initGl(); }
void TextureObjectBase::releaseGl(uint ) { p_to_->releaseGl(); }
GL::ScreenQuad * TextureObjectBase::createShaderQuad(
        const GL::ShaderSource& src, const QList<QString>& texNames)
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
    MO_TO_DEBUG("initGl()");

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

    screenQuad = new GL::ScreenQuad(to->name() + "_outquad");
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
}

void TextureObjectBase::PrivateTO::releaseGl()
{
    MO_TO_DEBUG("releaseGl()");

    for (auto & q : shaderQuads)
        q.quad->release();
    shaderQuads.clear();

    if (screenQuad && screenQuad->isCreated())
        screenQuad->release();
    delete screenQuad;
    screenQuad = 0;

    if (swapTex && swapTex->isAllocated())
        swapTex->release();
    delete swapTex;
    swapTex = 0;

    if (fbo && fbo->isCreated())
        fbo->release();
    delete fbo;
    fbo = 0;
    outputTex = 0;
}


void TextureObjectBase::PrivateTO::createFbo(const QSize &s, uint depth)
{
    outputTex = 0;
    if (!fbo)
    {
        int width = s.width(),
            height = s.height(),
            format = p_texFormat->baseValue(),
            type = p_texType->baseValue();

        aspectRatio = (Float)width/std::max(1, height);
        fboDepth = depth;

        fbo = new GL::FrameBufferObject(
                width,
                height,
                depth,
                gl::GLenum(Parameters::getTexFormat(format, type)),
                gl::GLenum(format),
                gl::GL_FLOAT,
                0,//GL::FrameBufferObject::A_DEPTH,
                false);
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
}

bool TextureObjectBase::hasInputTextureChanged(Double time, uint thread) const
{
    for (const ParameterTexture * t : p_to_->p_textures)
    {
        if (t->hasChanged(time, thread))
            return true;
    }
    return false;
}

GL::ScreenQuad * TextureObjectBase::shaderQuad(uint index) const
{
    return index < (uint)p_to_->shaderQuads.size() ? p_to_->shaderQuads[index].quad : 0;
}

void TextureObjectBase::PrivateTO::createShaderQuad(
        const GL::ShaderSource& csrc, const QList<QString>& texNames)
{
    MO_TO_DEBUG("createShaderQuad() curnum == " << shaderQuads.size());

    // create framebuffer if we havn't already
    //if (!fbo)
        //createFbo(16, 16);

    lastMessages.clear();

    // create quad and compile shader

    ShaderQuad quad;
    const QString qname = to->name() + QString("_shaderquad%1").arg(shaderQuads.size());
    quad.quad = new GL::ScreenQuad(qname);

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
        auto shader = quad.quad->shader();

        // uniforms

        quad.u_time = shader->getUniform("u_time", false);
        quad.u_resolution = shader->getUniform("u_resolution", false);
        quad.u_transformation = shader->getUniform("u_transformation", false);
        quad.u_color_range_min = shader->getUniform("u_color_range_min", false);
        quad.u_color_range_max = shader->getUniform("u_color_range_max", false);

        for (auto & n : texNames)
            quad.u_tex << shader->getUniform(n, false);

        shaderQuads << quad;

    }
    catch (Exception& )
    {
        lastMessages = quad.quad->shader()->compileMessages();
        to->setErrorMessage(quad.quad->shader()->compileMessagesString());

        // clean-up
        delete quad.quad;

        throw;
    }
}

TextureObjectBase::ResolutionMode TextureObjectBase::getResolutionMode() const
{
    return ResolutionMode( p_to_->p_resMode->baseValue() );
}

gl::GLenum TextureObjectBase::getTextureFormat() const
{
    return (gl::GLenum)
            Parameters::getTexFormat(p_to_->p_texFormat->baseValue(),
                                     p_to_->p_texType->baseValue());
    //return gl::GLenum( p_to_->p_texFormat->baseValue() );
}

QSize TextureObjectBase::adjustResolution(const QSize& res) const
{
    switch (getResolutionMode())
    {
        default:
        case RM_CUSTOM:
            return res;

        case RM_INPUT_SCALED:
        {
            Float s = p_to_->p_res_scale->baseValue();
            return QSize(std::max(uint(2), uint(res.width() * s)),
                         std::max(uint(2), uint(res.height() * s)));
        }

        case RM_INPUT_FIX_WIDTH:
        {
            Float a = Float(res.height()) / std::max(int(1), res.width());
            return QSize(p_to_->p_width->baseValue(),
                         res.width() * a);
        }

        case RM_INPUT_FIX_HEIGHT:
        {
            Float a = Float(res.width()) / std::max(int(1), res.height());
            return QSize(p_to_->p_height->baseValue(),
                         res.height() * a);
        }
    }
}

void TextureObjectBase::PrivateTO::renderShaderQuad(uint index, Double time, uint thread, uint& texSlot)
{
    MO_TO_DEBUG("renderShaderQuad(" << index << ", " << time << ", " << thread << ", " << texSlot << ")");

    if (index >= (uint)shaderQuads.size())
        return;

    // check correct resolution
    uint width = p_width->baseValue(),
         height = p_height->baseValue();

    auto resMode = to->getResolutionMode();
    if (resMode != RM_CUSTOM)
    {
        // -- find input resolution --
        for (int i=0; i<p_textures.length(); ++i)
        {
            const GL::Texture * tex = p_textures[i]->value(time, thread);
            if (tex)
            {
                width = tex->width();
                height = tex->height();
                break;
            }
        }

        QSize res = to->adjustResolution(QSize(width, height));
        width = res.width();
        height = res.height();
    }

    // update FBO/resolution
    if (!fbo)
        createFbo(QSize(width, height), fboDepth);
    else
    if (fbo->width() != width || fbo->height() != height
            || fbo->depth() != fboDepth)
    {
        fbo->release();
        delete fbo;
        fbo = 0;
        createFbo(QSize(width, height), fboDepth);
    }


    const ShaderQuad & quad = shaderQuads[index];

    const auto res = to->resolution();

    fbo->bind();

    // exchange render target for multi-stage rendering
    if (shaderQuads.size() > 1 && index > 0)
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
        quad.u_resolution->setFloats(res.width(), res.height(),
                                     1.f / res.width(), 1.f / res.height());
    if (hasColorRange)
    {
        if (quad.u_color_range_min)
            quad.u_color_range_min->setFloats(
                        p_r_min->value(time, thread),
                        p_g_min->value(time, thread),
                        p_b_min->value(time, thread),
                        p_a_min->value(time, thread));
        if (quad.u_color_range_max)
            quad.u_color_range_max->setFloats(
                        p_r_max->value(time, thread),
                        p_g_max->value(time, thread),
                        p_b_max->value(time, thread),
                        p_a_max->value(time, thread));
    }

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

    MO_CHECK_GL( gl::glDisable(gl::GL_BLEND) );

    MO_EXTEND_EXCEPTION(
        quad.quad->draw(res.width(), res.height(), p_split->baseValue())
                , "in TextureObjectBase::renderGl()")

    gl::glFlush();
    gl::glFinish();
    fbo->setChanged();

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
    auto tex = to->valueTexture(0, time, thread);
    if (!tex)
        return;

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
