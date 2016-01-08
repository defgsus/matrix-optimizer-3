/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/6/2016</p>
*/

#include <QTextStream>

#include "neurogl.h"
#include "gl/texture.h"
#include "gl/framebufferobject.h"
#include "gl/shadersource.h"
#include "gl/shader.h"
#include "gl/screenquad.h"
#include "gl/vertexarrayobject.h"
#include "geom/geometryfactory.h"
#include "geom/geometry.h"
#include "io/streamoperators_qt.h"

#if 1
#   include "io/log.h"
#   define MO_NEURO_DEBUG(arg__) MO_PRINT("NeuroGl::" << arg__)
#else
#   define MO_NEURO_DEBUG(unused__)
#endif

namespace MO {

struct NeuroGl::Private
{
    struct Stage
    {
        Stage() : fbo(0), shader(0), quad(0) { }
        GL::FrameBufferObject* fbo;
        GL::Shader* shader;
        GL::VertexArrayObject* quad;
        GL::Uniform *u_resolution,
                    *u_learnrate,
                    *u_rseed,
                    *u_weight_init;
    };

    Private(NeuroGl* p)
        : p             (p)
        , texIn         (0)
        , texWeight     (0)
        , texError      (0)
        , outputFormat  (gl::GL_RGBA32F)
        , typeDimension (4)
        , doRecompile   (true)
        , mode          (MODE_BYPASS)
        , learnrate     (0.1)
        , weightInitAmp (.5)
        , weightInitOffset(0.)
        , weightInitLocal(0.)
        , weightInitLocalPow(1.)
        , isInputSigned (true)
        , isOutputSigned(true)
        , isClampAlpha  (false)
        , randomSeed    (0)
    {
        stages.resize(2);
    }

    GL::ShaderSource getDefaultSource(const QSize& resIn, const QSize& resOut, const QSize& resWeight) const;
    void updateGl();
    void updateFbo(GL::FrameBufferObject*& fbo, const QSize& res) const;
    void updateShader(Stage* s, const GL::ShaderSource& src, const QString& name) const;
    void updateQuad(Stage* s, const QString& name) const;
    void updateBypassStage(const QSize& resIn, const QSize& resOut, Stage*) const;
    void updateFPropStage(const QSize& resIn, const QSize& resOut,
                          const QSize& resWeight, Stage*) const;
    void updateWeightInitStage(const QSize& resIn, const QSize& resOut,
                          const QSize& resWeight, Stage*) const;
    void clearStage(Stage*);
    bool checkStageReady(Stage*) const;
    void releaseGl();
    void bindInput();
    void bindWeight();
    void bindError();
    void renderStage(Stage* s);
    void step();

    NeuroGl* p;

    std::vector<Stage> stages;

    const GL::Texture* texIn, *texWeight, *texError;
    gl::GLenum outputFormat;
    int typeDimension;
    bool doRecompile;

    Mode mode;
    float learnrate,
        weightInitAmp, weightInitOffset,
        weightInitLocal, weightInitLocalPow;
    QSize inputRes, outputRes, weightRes;
    bool isInputSigned, isOutputSigned, isClampAlpha;
    int randomSeed;
};


NeuroGl::NeuroGl()
    : p_        (new Private(this))
{

}

NeuroGl::~NeuroGl()
{
    delete p_;
}

// ------ getter ------------

QString NeuroGl::infoString() const
{
    QString str;
    QTextStream s(&str);
    s << "NeuroGl"
      << " in=" << p_->inputRes.width() << "x" << p_->inputRes.height()
      << " out=" << p_->outputRes.width() << "x" << p_->outputRes.height()
      << " weight=" << p_->weightRes.width() << "x" << p_->weightRes.height()
      << " inTex=" << (p_->texIn && p_->texIn->isAllocated() ? "yes" : "no")
      << " errTex=" << (p_->texError && p_->texError->isAllocated() ? "yes" : "no")
      << " weightTex=" << (p_->texWeight && p_->texWeight->isAllocated() ? "yes" : "no")
         ;
    for (const Private::Stage& stage : p_->stages)
    {
        s << "\nstage:"
          << " fbo=";
        if (!stage.fbo)
            s << "NULL";
        else if (!stage.fbo->isCreated())
            s << "empty";
        else
            s << stage.fbo->width() << "x" << stage.fbo->height();

        s << " shader=";
        if (!stage.shader)
            s << "NULL";
        else if (!stage.shader->isReady())
            s << "empty";
        else
            s << "yes";

        s << " quad=";
        if (!stage.quad)
            s << "NULL";
        else if (!stage.quad->isCreated())
            s << "empty";
        else
            s << "yes";
    }
    return str;
}

NeuroGl::Mode NeuroGl::mode() const { return p_->mode; }

float NeuroGl::learnrate() const { return p_->learnrate; }
int NeuroGl::randomSeed() const { return p_->randomSeed; }
float NeuroGl::weightInitAmp() const { return p_->weightInitAmp; }
float NeuroGl::weightInitOffset() const { return p_->weightInitOffset; }
float NeuroGl::weightInitLocalAmp() const { return p_->weightInitLocal; }
float NeuroGl::weightInitLocalPow() const { return p_->weightInitLocalPow; }

const QSize& NeuroGl::inputRes() const { return p_->inputRes; }
const QSize& NeuroGl::outputRes() const { return p_->outputRes; }
const QSize& NeuroGl::weightRes() const { return p_->weightRes; }

int NeuroGl::outputFormat() const { return (int)p_->outputFormat; }
int NeuroGl::typeDimension() const { return p_->typeDimension; }

bool NeuroGl::isInputSigned() const { return p_->isInputSigned; }
bool NeuroGl::isOutputSigned() const { return p_->isOutputSigned; }
bool NeuroGl::isClampAlpha() const { return p_->isClampAlpha; }

// ------ setter ------------

void NeuroGl::setMode(Mode m)
    { p_->doRecompile |= p_->mode != m; p_->mode = m; }

void NeuroGl::setLearnrate(float lr) { p_->learnrate = lr; }
void NeuroGl::setRandomSeed(int s) { p_->randomSeed = s; }
void NeuroGl::setWeightInitAmp(float w) { p_->weightInitAmp = w; }
void NeuroGl::setWeightInitOffset(float w) { p_->weightInitOffset = w; }
void NeuroGl::setWeightInitLocalAmp(float w) { p_->weightInitLocal = w; }
void NeuroGl::setWeightInitLocalPow(float w) { p_->weightInitLocalPow = w; }

void NeuroGl::setInputRes(const QSize& si)
    { p_->doRecompile |= p_->inputRes != si; p_->inputRes = si; }
void NeuroGl::setOutputRes(const QSize& si)
    { p_->doRecompile |= p_->outputRes != si; p_->outputRes = si; }
void NeuroGl::setWeightRes(const QSize& si)
    { p_->doRecompile |= p_->weightRes != si; p_->weightRes = si; }

void NeuroGl::setInputTexture(const GL::Texture* t) { p_->texIn = t; }
void NeuroGl::setWeightTexture(const GL::Texture* t) { p_->texWeight = t; }
void NeuroGl::setErrorTexture(const GL::Texture* t) { p_->texError = t; }
void NeuroGl::setOutputFormat(int glenum)
    { p_->doRecompile |= p_->outputFormat != (gl::GLenum)glenum;
        p_->outputFormat = (gl::GLenum)glenum; }
void NeuroGl::setTypeDimension(int d)
    { p_->doRecompile |= p_->typeDimension != d; p_->typeDimension = d; }

void NeuroGl::setInputSigned(bool b)
    { p_->doRecompile |= p_->isInputSigned != b; p_->isInputSigned = b; }
void NeuroGl::setOutputSigned(bool b)
    { p_->doRecompile |= p_->isOutputSigned != b; p_->isOutputSigned = b; }
void NeuroGl::setClampAlpha(bool b)
    { p_->doRecompile |= p_->isClampAlpha != b; p_->isClampAlpha = b; }

// ----- opengl ------------

void NeuroGl::releaseGl() { p_->releaseGl(); }

void NeuroGl::Private::bindInput()
{
    if (texIn)
    {
        GL::Texture::setActiveTexture(SLOT_INPUT);
        texIn->bind();
    }
}

void NeuroGl::Private::bindWeight()
{
    if (texWeight)
    {
        GL::Texture::setActiveTexture(SLOT_WEIGHT);
        texWeight->bind();
    }
}

void NeuroGl::Private::bindError()
{
    if (texError)
    {
        GL::Texture::setActiveTexture(SLOT_ERROR);
        texError->bind();
    }
}

const GL::Texture* NeuroGl::outputTexture() const
{
    return p_->stages[0].fbo
            && p_->stages[0].fbo->numColorTextures()
                ? p_->stages[0].fbo->colorTexture() : 0;
}

const GL::Texture* NeuroGl::weightTexture() const
{
    return p_->stages[1].fbo
            && p_->stages[1].fbo->numColorTextures()
                ? p_->stages[1].fbo->colorTexture() : 0;
}

void NeuroGl::updateGl() { p_->updateGl(); }

void NeuroGl::step(int iterations)
{
    p_->step();
}

void NeuroGl::Private::updateFbo(GL::FrameBufferObject *&fbo, const QSize &res) const
{
    MO_NEURO_DEBUG("updateFbo(" << res << ")");

    if (fbo && ((int)fbo->width() != res.width()
             || (int)fbo->height() != res.height()
             || fbo->format() != outputFormat)
            )
    {
        if (fbo->isCreated())
            fbo->release();
        fbo = 0;
    }

    if (!fbo)
    {
        fbo = new GL::FrameBufferObject(
                    res.width(), res.height(), outputFormat, gl::GL_FLOAT);
        fbo->create();
    }
}

void NeuroGl::Private::updateShader(
        Stage* s, const GL::ShaderSource& src, const QString& name) const
{
    MO_NEURO_DEBUG("updateShader(" << s << ", " << name << ")");

    if (s->shader)
    {
        if (s->shader->isReady() && *s->shader->source() != src)
        {
            s->shader->releaseGL();
        }
        delete s->shader;
        s->shader = 0;
    }
    if (!s->shader)
    {
        s->shader = new GL::Shader(name);
        s->shader->setSource(src);
        s->shader->compile();

        auto u_tex = s->shader->getUniform("u_tex_input");
        if (u_tex) u_tex->ints[0] = SLOT_INPUT;
        u_tex = s->shader->getUniform("u_tex_weight");
        if (u_tex) u_tex->ints[0] = SLOT_WEIGHT;
        u_tex = s->shader->getUniform("u_tex_error");
        if (u_tex) u_tex->ints[0] = SLOT_ERROR;
        u_tex = s->shader->getUniform("u_tex_prev_out");
        if (u_tex) u_tex->ints[0] = SLOT_PREV_OUT;

        s->u_learnrate = s->shader->getUniform("u_learnrate");
        s->u_resolution = s->shader->getUniform("u_resolution");
        s->u_rseed = s->shader->getUniform("u_rseed");
        s->u_weight_init = s->shader->getUniform("u_weight_init");
    }
}

void NeuroGl::Private::updateQuad(Stage* s, const QString& name) const
{
    MO_NEURO_DEBUG("updateQuad(" << s << ", " << name << ")");

    if (s->quad)
    {
        if (s->quad->isCreated())
            s->quad->release();
        delete s->quad;
        s->quad = 0;
    }
    if (!s->quad)
    {
        s->quad = new GL::VertexArrayObject(name);
        auto g = new GEOM::Geometry();
        try
        {
            GEOM::GeometryFactory::createQuad(g, 2, 2);
            g->getVertexArrayObject(s->quad, s->shader);
        }
        catch (Exception& e)
        {
            g->releaseRef();
            delete s->quad;
            s->quad = 0;
            e << "\nwhen creating shader quad for NeuroGl stage";
            throw;
        }
        g->releaseRef();
    }
}

GL::ShaderSource NeuroGl::Private::getDefaultSource(
        const QSize& resIn, const QSize& resOut, const QSize& resWeight) const
{
    GL::ShaderSource src;

    src.loadVertexSource(":/shader/neuro.vert");
    src.loadFragmentSource(":/shader/neuro.frag");

    src.pasteDefaultIncludes();

    QString defines;
    defines += QString(
                "// --- config ---\n\n"
                "#define TYPE_DIMENSION %1\n"
                "#define SIGNED_INPUT   %2\n"
                "#define SIGNED_OUTPUT  %3\n"
                "#define SIGNED_WEIGHTS %4\n"
                "#define CLAMP_ALPHA_1  %5\n\n"
                "#define LABEL_INPUT    %6\n\n"
                "// --- resolutions --\n\n")
            .arg(typeDimension)
            .arg(isInputSigned ? 1 : 0)
            .arg(isOutputSigned ? 1 : 0)
            .arg(isInputSigned ? 1 : 0)
            .arg(isClampAlpha ? 1 : 0)
            .arg(0)
            ;

    if (!resIn.isEmpty())
    defines += QString(
                "// resolution of input texture (0)\n"
                "#define INPUT_RES ivec2(%1, %2)\n")
                .arg(resIn.width()).arg(resIn.height());
    if (!resOut.isEmpty())
    defines += QString(
                "// res of output texture\n"
                "// in BPROP mode equals previous output res\n"
                "#define OUTPUT_RES ivec2(%1, %2)\n")
                .arg(resOut.width()).arg(resOut.height());
    if (!resWeight.isEmpty())
    defines += QString(
                "// res of weight texture (1)\n"
                "#define WEIGHT_RES ivec2(%1, %2)\n")
                .arg(resWeight.width()).arg(resWeight.height());
    /*if (!resOut.isEmpty())
    defines += QString(
                "// resolution of error texture (2)\n"
                "const ivec2 ERROR_RES = ivec2(%1, %2);\n")
                .arg(resOut.width()).arg(resOut.height());
    */

    defines += "\n";

    if (!resIn.isEmpty())
        defines += "const int NUM_IN = INPUT_RES.x * INPUT_RES.y;\n";
    if (!resOut.isEmpty())
        defines += "const int NUM_OUT = OUTPUT_RES.x * OUTPUT_RES.y;\n";
    if (!resWeight.isEmpty())
        defines += "const int NUM_WEIGHT = WEIGHT_RES.x * WEIGHT_RES.y;\n";

    src.addDefine(defines);

    //std::cout << defines << std::endl;
    //std::cout << src.fragmentSource() << std::endl;
    return src;
}

void NeuroGl::Private::updateGl()
{
    MO_NEURO_DEBUG("updateGl() doRecompile=" << doRecompile);

    // input resolution
    QSize resIn = inputRes;
    if (mode != MODE_WEIGHT_INIT && texIn)
    {
        QSize s(texIn->width(), texIn->height());
        doRecompile |= resIn != s;
        resIn = s;
    }

    // output resolution
    QSize resOut = outputRes;
    if (mode == MODE_BPROP && texError)
    {
        QSize s(texError->width(), texError->height());
        doRecompile |= resOut != s;
        resOut = s;
    }

    // weight resolution
    QSize resWeight = weightRes;
    if (mode != MODE_WEIGHT_INIT && texWeight)
    {
        QSize s(texWeight->width(), texWeight->height());
        doRecompile |= resWeight != s;
        resWeight = s;
    }

    // -- fprob layer --

    switch (mode)
    {
        case MODE_BYPASS:
            inputRes = resIn;
            outputRes = resOut;
            updateBypassStage(resIn, resOut, &stages[0]);
            clearStage(&stages[1]);
        break;
        case MODE_FPROP:
            inputRes = resIn;
            outputRes = resOut;
            weightRes = resWeight;
            updateFPropStage(resIn, resOut, resWeight, &stages[0]);
            clearStage(&stages[1]);
        break;
        case MODE_BPROP:
            inputRes = resIn;
            outputRes = resOut;
            weightRes = resWeight;
            updateFPropStage(resIn, resOut, resWeight, &stages[0]);
        break;
        case MODE_WEIGHT_INIT:
            inputRes = resIn;
            outputRes = resOut;
            weightRes = resWeight;
            clearStage(&stages[0]);
            updateWeightInitStage(resIn, resOut, resWeight, &stages[1]);
        break;
    }

    doRecompile = false;
}

void NeuroGl::Private::releaseGl()
{
    // release each stage
    for (Stage& stage : stages)
        clearStage(&stage);
}

void NeuroGl::Private::clearStage(Stage* s)
{
    if (s->fbo)
    {
        if (s->fbo->isCreated())
            s->fbo->release();
        delete s->fbo;
        s->fbo = 0;
    }
    if (s->shader)
    {
        if (s->shader->isReady())
            s->shader->releaseGL();
        delete s->shader;
        s->shader = 0;
    }
    if (s->quad)
    {
        if (s->quad->isCreated())
            s->quad->release();
        delete s->quad;
        s->quad = 0;
    }
}


void NeuroGl::Private::updateBypassStage(
        const QSize& resIn, const QSize& resOut, Stage* s) const
{
    MO_NEURO_DEBUG("updateBypassStage(" << resIn << ", " << resOut << ", " << s << ")"
                   " doRecompile=" << doRecompile);

    if (resIn.isEmpty() || resOut.isEmpty())
        return;

    updateFbo(s->fbo, resOut);

    if (doRecompile)
    {
        GL::ShaderSource src = getDefaultSource(resIn, resOut, QSize());
        src.addDefine("#define MODE M_BYPASS");
        updateShader(s, src, "bypass");
        updateQuad(s, "bypass");
    }
}

void NeuroGl::Private::updateFPropStage(
        const QSize& resIn, const QSize& resOut, const QSize& resWeight, Stage* s) const
{
    if (resIn.isEmpty() || resOut.isEmpty() || resWeight.isEmpty())
        return;

    updateFbo(s->fbo, resOut);

    if (doRecompile)
    {
        GL::ShaderSource src = getDefaultSource(resIn, resOut, resWeight);
        src.addDefine("#define MODE M_FPROP");
        updateShader(s, src, "fprop");
        updateQuad(s, "fprop");
    }
}


void NeuroGl::Private::updateWeightInitStage(
        const QSize& resIn, const QSize& resOut, const QSize& resWeight, Stage* s) const
{
    if (resIn.isEmpty() || resOut.isEmpty() || resWeight.isEmpty())
        return;

    updateFbo(s->fbo, resWeight);

    if (doRecompile)
    {
        GL::ShaderSource src = getDefaultSource(resIn, resOut, resWeight);
        src.addDefine("#define MODE M_WEIGHT_INIT");
        updateShader(s, src, "weight");
        updateQuad(s, "weight");
    }
}

bool NeuroGl::Private::checkStageReady(Stage* s) const
{
    return (s->fbo && s->fbo->isCreated()
         && s->shader && s->shader->isReady()
         && s->quad && s->quad->isCreated());
}

void NeuroGl::Private::step()
{
    updateGl();

    switch (mode)
    {
        case MODE_BYPASS:
        {
            if (!checkStageReady(&stages[0]))
            {
                MO_GL_WARNING("NeuroGl not initialized\n" << p->infoString());
                return;
            }

            bindInput();
            renderStage(&stages[0]);
        }
        break;

        case MODE_FPROP:
        {
            if (!checkStageReady(&stages[0]))
            {
                MO_GL_WARNING("NeuroGl not initialized\n" << p->infoString());
                return;
            }

            bindInput();
            bindWeight();

            renderStage(&stages[0]);
        }
        break;

        case MODE_BPROP:
        {
            if (!checkStageReady(&stages[0])
              || !checkStageReady(&stages[1]))
            {
                MO_GL_WARNING("NeuroGl not initialized\n" << p->infoString());
                return;
            }

            bindInput();
            bindWeight();
            bindError();

            renderStage(&stages[0]);
        }
        break;

        case MODE_WEIGHT_INIT:
        {
            if (!checkStageReady(&stages[1]))
            {
                MO_GL_WARNING("NeuroGl not initialized\n" << p->infoString());
                return;
            }
            renderStage(&stages[1]);
        }
        break;
    }

}

void NeuroGl::Private::renderStage(Stage* s)
{
    MO_NEURO_DEBUG("renderStage(" << s << ")");

    s->fbo->bind();
    s->fbo->setViewport();
    MO_CHECK_GL_THROW( gl::glClearColor(0, 0, 0, 0) );
    MO_CHECK_GL_THROW( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );
    s->shader->activate();
    if (s->u_learnrate)
        s->u_learnrate->floats[0] = learnrate;
    if (s->u_resolution)
        s->u_resolution->setFloats(s->fbo->width(), s->fbo->height());
    if (s->u_rseed)
        s->u_rseed->ints[0] = randomSeed;
    if (s->u_weight_init)
        s->u_weight_init->setFloats(weightInitOffset, weightInitAmp,
                                    weightInitLocal, weightInitLocalPow);
    s->shader->sendUniforms();
    s->quad->drawElements();
    s->shader->deactivate();
    s->fbo->unbind();
    s->fbo->setChanged();
}

} // namespace MO
