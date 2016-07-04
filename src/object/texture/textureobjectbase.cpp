/** @file textureobjectbase.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#include <QList>

#include "textureobjectbase.h"
#include "object/scene.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertexture.h"
#include "object/param/parameterfilename.h"
#include "object/util/objecteditor.h"
#include "object/util/texturesetting.h"
#include "gl/context.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/currenttime.h"
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
        , hasInternalFbo(true)
        , doAllowMultiPass(false)
        , doAllowResolutionChange(true)
        , p_resMode     (0)
        , p_width       (0)
        , p_height      (0)
        , p_split       (0)
        , alphaBlend    (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void createScreenQuad();
    void createShaderQuad(const GL::ShaderSource& src, const QList<QString>& texNames);
    void createFbo(const QSize& s, uint depth = 1);
    void drawFramebuffer(const RenderTime& time, int width, int height);
    void renderShaderQuad(uint index, const RenderTime& time, uint* texSlot);
    void exchangeFeedbackTexture();

    const QString& name() const { return to->name(); } // for debug

    /** A quad and shader with associated uniforms */
    struct ShaderQuad
    {
        GL::ScreenQuad * quad;
        GL::Uniform
                * u_resolution,
                * u_time, * u_time_delta,
                * u_transformation,
                * u_color_range_min,
                * u_color_range_max,
                * u_tex_res,
                * u_tex_feedback,
                * u_pass;
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
    bool hasColorRange, hasInternalFbo,
        doAllowMultiPass, doAllowResolutionChange;
    QList<GL::Shader::CompileMessage> lastMessages;

    ParameterFloat  * p_out_r, * p_out_g, * p_out_b, * p_out_a,
                    * p_r_min, * p_r_max,
                    * p_g_min, * p_g_max,
                    * p_b_min, * p_b_max,
                    * p_a_min, * p_a_max,
                    * p_res_scale;
    ParameterSelect * p_magInterpol, * p_enableOut,
                * p_texType, * p_texFormat, *p_resMode;
    ParameterInt * p_width, * p_height, * p_depth, * p_aa, * p_split,
                    * p_numPasses;
    ParameterText * p_fragment;
    ParameterTexture* p_feedbackTex;
    QList<TextureSetting*> p_textures;
    QList<ParameterTexture*> texParamsCopy;
    QList<bool> texIsCube;

    GL::Uniform
        * u_out_color, * u_out_resolution;

    Float aspectRatio;
    AlphaBlendSetting alphaBlend;
};


TextureObjectBase::TextureObjectBase()
    : ObjectGl      ()
    , p_to_         (new PrivateTO(this))
{
    initCreateRenderSettings(false);
    initDefaultUpdateMode(UM_ON_CHANGE);
    setNumberOutputs(ST_TRANSFORMATION, 0); // undo ObjectGl
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
    // determine from input texture
    // XXX hacky because of the RenderTime(0)
    if (p_to_->p_resMode->baseValue() != RM_CUSTOM)
    if (auto tex = valueTexture(0, RenderTime(0, MO_GFX_THREAD)))
    {
        //std::cout << "TEX " << tex->isAllocated() << std::endl;
        if (tex->isAllocated())
            return adjustResolution(QSize(tex->width(), tex->height()));
    }

    if (p_to_->fbo)
        return QSize(p_to_->fbo->width(), p_to_->fbo->height());

    if (p_to_->p_resMode->baseValue() == RM_CUSTOM)
        return getDesiredResolution();

    return getDesiredResolution();
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
    return p_to_->p_textures[index]->textureParam()->isModulated();
}

bool TextureObjectBase::isMasterOutputEnabled() const
{
    return p_to_->p_enableOut->baseValue();
}

void TextureObjectBase::setMasterOutputEnabled(bool enable, bool sendGui)
{
    if (sendGui)
    {
        if (auto e = editor())
        {
            e->setParameterValue(p_to_->p_enableOut, enable);
            return;
        }
        MO_WARNING("Can't set master-out parameter via GUI as expected");
    }
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

void TextureObjectBase::initInternalFbo(bool e)
{
    p_to_->hasInternalFbo = e;
}

void TextureObjectBase::initEnableColorRange(bool e)
{
    p_to_->hasColorRange = e;
}

void TextureObjectBase::init3dFramebuffer(uint depth)
{
    p_to_->fboDepth = depth;
}

void TextureObjectBase::initAllowMultiPass(bool a)
{
    p_to_->doAllowMultiPass = a;
}

void TextureObjectBase::initEnableResolutionChange(bool a)
{
    p_to_->doAllowResolutionChange = a;
}

const QList<ParameterTexture*>& TextureObjectBase::textureParams() const
{
    return p_to_->texParamsCopy;
}

void TextureObjectBase::createParameters()
{
    ObjectGl::createParameters();
    p_to_->createParameters();
}

void TextureObjectBase::PrivateTO::createParameters()
{
    if (hasInternalFbo)
    {
        to->params()->beginParameterGroup("to_res", tr("resolution and format"));
        to->params()->beginEvolveGroup(false);

            p_resMode  = to->params()->createSelectParameter(
                        "to_res_mode", tr("resolution"),
            tr("Selects how the resolution is defined"),
            { "custom", "input", "scaled", "fix_width", "fix_height" },
            { tr("custom"), tr("input"), tr("input scaled"),
              tr("input ratio width"), tr("input ratio height") },
            { tr("Resolution can be freely set"),
              tr("The resolution from the input texture is used"),
              tr("A scaled resolution with same ratio as the input texture is used"),
              tr("Fixed with and height choosen to match original aspect ratio"),
              tr("Fixed height and width choosen to match original aspect ratio") },
            { RM_CUSTOM, RM_INPUT, RM_INPUT_SCALED,
              RM_INPUT_FIX_WIDTH, RM_INPUT_FIX_HEIGHT },
            RM_INPUT,
            true, false);

            p_width = to->params()->createIntParameter(
                        "to_width", tr("width"), tr("Width of texture in pixels"),
                        1024, 16, 4096*4, 16, true, false);
            p_height = to->params()->createIntParameter(
                        "to_height", tr("height"), tr("Height of texture in pixels"),
                        1024, 16, 4096*4, 16, true, false);
            p_depth = to->params()->createIntParameter(
                        "to_depth", tr("depth"), tr("Depth of texture in pixels"),
                        1024, 16, 4096*4, 16, true, false);
            p_res_scale = to->params()->createFloatParameter("to_res_scale",
                            tr("scale"), tr("A multiplier for the input texture"),
                                                             1.f, 0.125f, true, false);
            p_res_scale->setMinValue(0.f);
            if (!doAllowResolutionChange)
            {
                p_resMode->setZombie(true);
                p_width->setZombie(true);
                p_height->setZombie(true);
                p_depth->setZombie(true);
                p_res_scale->setZombie(true);
            }

            p_texFormat = to->params()->createTextureFormatParameter(
                        "to_format", tr("texture format"),
                        tr("The channel format of the output texture"));
            p_texType = to->params()->createTextureTypeParameter(
                        "to_type", tr("texture type"),
                        tr("The type-per-channel of the output texture"));


        to->params()->endEvolveGroup();
        to->params()->endParameterGroup();
    }

    to->params()->beginParameterGroup("to_res", tr("resolution and format"));
    to->params()->beginEvolveGroup(false);

        p_split = to->params()->createIntParameter("to_split", tr("segments"),
                    tr("Split rendering of the output into separate regions "
                       "for better gui response"),
                                    1, 1, 4096, 1, true, false);
        /** @todo fix split/segmentation in TextureObjectBase */
        p_split->setZombie(true);

        p_numPasses = to->params()->createIntParameter(
                    "to_num_passes", tr("number of passes"),
            tr("Iterates a number of times over the u_tex_feedback texture."),
                1, 1, 1024, 1, true, false);
        p_numPasses->setVisible(doAllowMultiPass);

    to->params()->endEvolveGroup();
    to->params()->endParameterGroup();


    to->params()->beginParameterGroup("to_input", tr("input"));
    if (maxIns)
        to->initParameterGroupExpanded("to_input");

        for (uint i=0; i<maxIns; ++i)
        {
            //MO_PRINT("TEXPARAM " << i);
            auto tex = new TextureSetting(to);
            p_textures.push_back(tex);

            tex->createParameters(QString("to_tex%1").arg(i),
                                  tr("texture #%1").arg(i+1),
                                  ParameterTexture::IT_INPUT);
            texParamsCopy.push_back(tex->textureParam());
            texIsCube.push_back(false);

            tex->textureParam()->setVisibleGraph(true);
            //MO_PRINT("TEXPARAM " << i << " done");
        }

        p_feedbackTex = to->params()->createTextureParameter(
                    "to_tex_feedback", tr("feedback texture"),
                    tr("Parameters of the feedback texture (the previous frame)"));

        p_feedbackTex->setInputType(ParameterTexture::IT_INTERNAL);

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
    to->params()->beginEvolveGroup(false);

        p_enableOut = to->params()->createBooleanParameter(
            "to_master_out", tr("enable"),
            tr("Enables or disables rendering the output to the main framebuffer"),
            tr("The texture object will render internally but not contribute "
               "to the main framebuffer"),
            tr("The texture object will render it's output ontop the main "
               " framebuffer"),
            false, true, true);

        p_out_r = to->params()->createFloatParameter(
            "to_red", tr("red"), tr("Red amount of output color"), 1.0, 0.1);
        p_out_g = to->params()->createFloatParameter(
            "to_green", tr("green"), tr("Green amount of output color"), 1.0, 0.1);
        p_out_b = to->params()->createFloatParameter(
            "to_blue", tr("blue"), tr("Blue amount of output color"), 1.0, 0.1);
        p_out_a = to->params()->createFloatParameter
                ("to_alpha", tr("alpha"),
                tr("Defines the opaqueness/transparency of the output [0,1]"),
                1.0,
                0.0, 1.0, 0.05);

        alphaBlend.createParameters(AlphaBlendSetting::M_MIX, false, "to_", "");

        p_magInterpol = to->params()->createBooleanParameter(
                "to_cammaginterpol", tr("interpolation"),
                tr("The interpolation mode for pixel magnification"),
                tr("No interpolation"),
                tr("Linear interpolation"),
                true,
                true, false);

        p_aa = to->params()->createIntParameter("to_outaa", tr("anti-aliasing"),
      tr("Sets the super-sampling when drawing the rendered frame onto the output"),
      1,
      1, 16, 1, true, false);

    to->params()->endEvolveGroup();
    to->params()->endParameterGroup();
}

void TextureObjectBase::onParameterChanged(Parameter * p)
{
    ObjectGl::onParameterChanged(p);

    bool r = false;
    for (auto set : p_to_->p_textures)
        r |= set->onParameterChange(p);

    if (r
        || p == p_to_->p_width
        || p == p_to_->p_height
        || p == p_to_->p_res_scale
        || p == p_to_->p_resMode
        || p == p_to_->p_depth
        || p == p_to_->p_aa
        || p == p_to_->p_texFormat
        || p == p_to_->p_texType
        )
        requestReinitGl();
}

void TextureObjectBase::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();

}

void TextureObjectBase::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    for (auto set : p_to_->p_textures)
        set->updateParameterVisibility();

    if (p_to_->hasInternalFbo)
    {
        const auto resMode = (ResolutionMode)p_to_->p_resMode->baseValue();
        bool res = resMode == RM_CUSTOM;
        p_to_->p_width->setVisible(res || resMode == RM_INPUT_FIX_WIDTH);
        p_to_->p_height->setVisible(res || resMode == RM_INPUT_FIX_HEIGHT);
        p_to_->p_res_scale->setVisible(resMode == RM_INPUT_SCALED);
        p_to_->p_depth->setVisible(res && false);
    }
}

void TextureObjectBase::getNeededFiles(IO::FileList &l)
{
    ObjectGl::getNeededFiles(l);

    for (auto set : p_to_->p_textures)
        set->getNeededFiles(l);
}

const QList<GL::Shader::CompileMessage>& TextureObjectBase::compileMessages() const
{
    return p_to_->lastMessages;
}

GL::FrameBufferObject * TextureObjectBase::fbo() const
{
    return p_to_->fbo;
}

GL::ShaderSource TextureObjectBase::valueShaderSource(uint index) const
{
    auto s = shaderQuad(index);
    return s ? *s->shader()->source() : GL::ShaderSource();
}

const GL::Texture * TextureObjectBase::valueTexture(uint chan, const RenderTime& ) const
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

void TextureObjectBase::drawFramebuffer(
        const RenderTime& time, int width, int height)
{
    p_to_->drawFramebuffer(time, width, height);
}
void TextureObjectBase::renderShaderQuad(
        uint index, const RenderTime& time, uint* texSlot)
{
    p_to_->renderShaderQuad(index, time, texSlot);
}

void TextureObjectBase::PrivateTO::initGl()
{
    MO_TO_DEBUG("initGl()");

    to->clearError();

    // size of frame
    if (hasInternalFbo)
    {
        int width = p_width->baseValue(),
            height = p_height->baseValue();

        aspectRatio = (Float)width/std::max(1, height);
    }
    else
        aspectRatio = 1.;
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

    for (auto p : p_textures)
        p->releaseGl();
}

void TextureObjectBase::PrivateTO::createScreenQuad()
{
    MO_TO_DEBUG("createScreenQuad()");

    if (screenQuad)
        return;

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
    catch (Exception& e)
    {
        delete screenQuad;
        screenQuad = 0;
        to->setErrorMessage(e.what());
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


void TextureObjectBase::PrivateTO::createFbo(const QSize &s, uint depth)
{
    if (!hasInternalFbo)
        return;

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

GL::Texture * TextureObjectBase::createTexture() const
{
    if (!p_to_->hasInternalFbo)
        return 0;

    int width = p_to_->p_width->baseValue(),
        height = p_to_->p_height->baseValue(),
        format = p_to_->p_texFormat->baseValue(),
        type = p_to_->p_texType->baseValue();

    auto tex = new GL::Texture(
            width,
            height,
            gl::GLenum(Parameters::getTexFormat(format, type)),
            gl::GLenum(format),
            gl::GLenum(type),
            0
            );

    return tex;
}

bool TextureObjectBase::hasInputTextureChanged(const RenderTime & time) const
{
    for (auto set : p_to_->p_textures)
    {
        if (set->isEnabled() && set->textureParam()->hasChanged(time))
            return true;
    }
    return false;
}

const GL::Texture* TextureObjectBase::inputTexture(uint index, const RenderTime& rt) const
{
    if ((int)index >= textureParams().size())
        return 0;
    auto tex = textureParams()[index]->value(rt);
    // update type flag
    if (index < (uint)p_to_->texIsCube.size())
        p_to_->texIsCube[index] = tex ? tex->isCube() : false;

    // XXX texture would need to be bound
    //if (tex)
    //    textureParams()[index]->setTextureParam(tex);
    return tex;
}

QString TextureObjectBase::getInputTextureDeclarations(const QStringList &names) const
{
    const int num = std::min(p_to_->p_textures.size(), names.size());
    QString decl;
    for (int i=0; i<num; ++i)
    {
        if (p_to_->p_textures[i]->isCube()
            || p_to_->texIsCube[i])
            decl += "uniform samplerCube " + names[i] + ";\n";
        else
            decl += "uniform sampler2D " + names[i] + ";\n";
    }
    return decl;
}

GL::ScreenQuad * TextureObjectBase::shaderQuad(uint index) const
{
    return index < (uint)p_to_->shaderQuads.size() ? p_to_->shaderQuads[index].quad : 0;
}

void TextureObjectBase::PrivateTO::createShaderQuad(
        const GL::ShaderSource& csrc, const QList<QString>& texNames)
{
    MO_TO_DEBUG("createShaderQuad() curnum == " << shaderQuads.size());

    lastMessages.clear();

    // create quad and compile shader

    ShaderQuad quad;
    const QString qname = to->name()
            + QString("_shaderquad%1").arg(shaderQuads.size());
    quad.quad = new GL::ScreenQuad(qname);

    auto src = new GL::ShaderSource(csrc);
    // render-flag
    if (auto s = to->sceneObject())
        if (s->isRendering())
            src->addDefine("#ifndef MO_RENDER\n#define MO_RENDER\n#endif", false);

    // resolve includes
    src->replaceIncludes([this](const QString& url, bool do_search)
    {
        Object* src;
        QString inc = to->getGlslInclude(url, do_search, &src);
        if (inc.isEmpty())
            return QString("/* empty */\n");
        else
        {
            /** @todo add error chain via ParameterText */
            return "/* " + url + " */\n" + inc;
        }
    });

    // -- insert texture defs --

    // XXX hhacky
    RenderTime rtime(CurrentTime::time(), MO_GFX_THREAD);
    for (uint i=0; i<to->numberTextureInputs(); ++i)
        to->inputTexture(i, rtime); // to get isCube flag

    src->replace("//%mo_texture_decl%",
                 to->getInputTextureDeclarations(texNames), true);

    // compile and vao
    try
    {
        quad.quad->create(src, 0);
        auto shader = quad.quad->shader();

        // uniforms

        quad.u_time = shader->getUniform("u_time", false);
        quad.u_time_delta = shader->getUniform("u_time_delta", false);
        quad.u_pass = shader->getUniform("u_pass", false);
        quad.u_resolution = shader->getUniform("u_resolution", false);
        quad.u_transformation = shader->getUniform("u_transformation", false);
        quad.u_color_range_min = shader->getUniform("u_color_range_min", false);
        quad.u_color_range_max = shader->getUniform("u_color_range_max", false);
        quad.u_tex_feedback = shader->getUniform("u_tex_feedback", false);

        // pre-init the texture slots in order of the supplied uniform names
        int texSlot = 0;
        for (auto & n : texNames)
        {
            quad.u_tex << shader->getUniform(n, false);
            if (quad.u_tex.back())
                quad.u_tex.back()->ints[0] = texSlot++;
        }

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

gl::GLenum TextureObjectBase::getDesiredTextureFormat() const
{
    if (!p_to_->hasInternalFbo)
        return gl::GL_RGBA;

    return (gl::GLenum)
            Parameters::getTexFormat(p_to_->p_texFormat->baseValue(),
                                     p_to_->p_texType->baseValue());
    //return gl::GLenum( p_to_->p_texFormat->baseValue() );
}

QSize TextureObjectBase::getDesiredResolution() const
{
    if (!p_to_->hasInternalFbo)
        return QSize();

    return QSize(
                p_to_->p_width->baseValue(),
                p_to_->p_height->baseValue());
}

QSize TextureObjectBase::adjustResolution(const QSize& res) const
{
    if (!p_to_->hasInternalFbo)
        return res;

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

void TextureObjectBase::PrivateTO::renderShaderQuad(
        uint index, const RenderTime& time, uint* texSlot)
{
    MO_TO_DEBUG("renderShaderQuad(" << index << ", "
                << time << ", " << *texSlot << ")");

    if (index >= (uint)shaderQuads.size()
        || !hasInternalFbo)
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
            const GL::Texture * tex = p_textures[i]->textureParam()->value(time);
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
        fbo = nullptr;
        createFbo(QSize(width, height), fboDepth);
    }


    const ShaderQuad & quad = shaderQuads[index];

    const auto res = to->resolution();

    fbo->bind();

    // exchange render target for multi-stage rendering
    // or when u_tex_feedback is used
    bool doSwap =
            (shaderQuads.size() > 1 && index > 0)
            || quad.u_tex_feedback != nullptr;

    if (doSwap)
        exchangeFeedbackTexture();


    //if (index == 0)
    {
        MO_CHECK_GL( gl::glViewport(0, 0, res.width(), res.height()) );
        MO_CHECK_GL( gl::glClearColor(0,0,0,0) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );
    }

    // --- set shader uniforms ---

    // (Time is set later)

    if (quad.u_resolution)
        quad.u_resolution->setFloats(res.width(), res.height(),
                                     1.f / res.width(), 1.f / res.height());
    if (hasColorRange)
    {
        if (quad.u_color_range_min)
            quad.u_color_range_min->setFloats(
                        p_r_min->value(time),
                        p_g_min->value(time),
                        p_b_min->value(time),
                        p_a_min->value(time));
        if (quad.u_color_range_max)
            quad.u_color_range_max->setFloats(
                        p_r_max->value(time),
                        p_g_max->value(time),
                        p_b_max->value(time),
                        p_a_max->value(time));
    }

    // --- bind input textures ---

    for (int i=0; i<p_textures.length(); ++i)
    {
        const GL::Texture * tex;

        // bind the last frame as input to later stages
        if (i == 0 && index > 0)
            tex = swapTex;
        else
            tex = p_textures[i]->textureParam()->value(time);
        if (tex)
        {
            // bind to slot x
            GL::Texture::setActiveTexture(*texSlot);
            tex->bind();
            p_textures[i]->textureParam()->applyTextureParam(tex);
            // tell shader
            if (i < quad.u_tex.length())
            {
                if (quad.u_tex[i])
                    quad.u_tex[i]->ints[0] = *texSlot;
            }

            ++(*texSlot);
        }
    }

    // --- render ---

    //quad.quad->shader()->dumpUniforms();

    MO_CHECK_GL( gl::glDisable(gl::GL_BLEND) );
    MO_CHECK_GL( gl::glDisable(gl::GL_DEPTH_TEST) );

    auto dtime = time;
    const int numPass = p_numPasses->value(time);
    dtime.setDelta(dtime.delta() / numPass);
    for (int i=0; i<numPass; ++i)
    {
        if (quad.u_time)
            quad.u_time->floats[0] = dtime.second();
        if (quad.u_time_delta)
            quad.u_time_delta->floats[0] = dtime.delta();
        dtime += dtime.delta();
        if (quad.u_pass)
            quad.u_pass->ints[0] = i;

        // multi-pass things
        if (i > 0)
        {
            exchangeFeedbackTexture();
        }

        // bind feedback texture
        if (quad.u_tex_feedback && swapTex)
        {
            GL::Texture::setActiveTexture(*texSlot);
            quad.u_tex_feedback->ints[0] = *texSlot;

            GL::Texture* fbtex = swapTex;
            fbtex->bind();
            if (i < 2)
                p_feedbackTex->applyTextureParam(fbtex);
        }

        MO_EXTEND_EXCEPTION(
            quad.quad->draw(res.width(), res.height(), p_split->baseValue())
                    , "in TextureObjectBase::renderGl()")
    }

    if (quad.u_tex_feedback && swapTex)
        ++(*texSlot); // skip feedback texture slot

    gl::glFlush();
    gl::glFinish();
    fbo->setChanged();

    // get the actual output of last stage
    outputTex = fbo->colorTexture();

    fbo->unbind();

    GL::Texture::setActiveTexture(0);
}

void TextureObjectBase::PrivateTO::exchangeFeedbackTexture()
{
    MO_ASSERT(fbo, "");

    // create a swap buffer
    if (!swapTex)
    {
        swapTex = GL::Texture::constructFrom(fbo->colorTexture());
        swapTex->create();
    }
    swapTex = fbo->swapColorTexture(swapTex);
}

/** @todo multi-pass not defined/implemented for
    multiple different shader stages (multiple calls to createShaderQuad()).
    And a few other things are different here, especially
    regarding texture inputs/binding. */
void TextureObjectBase::renderShaderQuad(
        GL::FrameBufferObject* fbo, uint index, const RenderTime& time,
        bool doClear) const
{
    MO_TO_DEBUG("renderShaderQuad(" << fbo->name() << ", "
                << index << ", " << time << ")");

    if (index >= (uint)p_to_->shaderQuads.size()
      || fbo->width() == 0 || fbo->height() == 0)
        return;

    const PrivateTO::ShaderQuad& quad = p_to_->shaderQuads[index];

    fbo->bind();

    if (doClear)
    {
        // output textures
        MO_CHECK_GL( gl::glViewport(0, 0, fbo->width(), fbo->height()) );
        MO_CHECK_GL( gl::glClearColor(0,0,0,0) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );
    }

    // --- set shader uniforms ---

    if (quad.u_time)
        quad.u_time->floats[0] = time.second();
    if (quad.u_resolution)
        quad.u_resolution->setFloats(fbo->width(), fbo->height(),
                                     1.f / fbo->width(), 1.f / fbo->height());
    if (p_to_->hasColorRange)
    {
        if (quad.u_color_range_min)
            quad.u_color_range_min->setFloats(
                        p_to_->p_r_min->value(time),
                        p_to_->p_g_min->value(time),
                        p_to_->p_b_min->value(time),
                        p_to_->p_a_min->value(time));
        if (quad.u_color_range_max)
            quad.u_color_range_max->setFloats(
                        p_to_->p_r_max->value(time),
                        p_to_->p_g_max->value(time),
                        p_to_->p_b_max->value(time),
                        p_to_->p_a_max->value(time));
    }

    // --- render ---

    MO_EXTEND_EXCEPTION(
        quad.quad->draw(fbo->width(), fbo->height(), p_to_->p_split->baseValue())
                , "in TextureObjectBase::renderGl()")

    gl::glFlush();
    gl::glFinish();
    fbo->setChanged();
}



void TextureObjectBase::PrivateTO::drawFramebuffer(const RenderTime & time, int width, int height)
{
    if (p_enableOut->value(time) == 0)
        return;

    gl::glFinish();

    // get output texture
    auto tex = to->valueTexture(0, time);
    if (!tex)
    {
        int num = to->getNumberOutputs(ST_TEXTURE),
              i = 1;
        while (tex == 0 && i < num)
            tex = to->valueTexture(i++, time);
    }
    if (!tex)
        return;

    if (!screenQuad)
        createScreenQuad();

    // -- shader uniforms --

    if (u_out_color)
        u_out_color->setFloats(
                    p_out_r->value(time),
                    p_out_g->value(time),
                    p_out_b->value(time),
                    p_out_a->value(time));

    if (u_out_resolution)
        u_out_resolution->setFloats(width, height,
                             1.f / std::max(1, width),
                             1.f / std::max(1, height));

    // -- render fbo frame onto current context --

    // set blendmode
    alphaBlend.apply(time);

    // bind the color texture from the fbo

    GL::Texture::setActiveTexture(0);
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
