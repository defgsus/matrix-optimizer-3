/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/6/2016</p>
*/

#include <QTextStream>

#include "NeuroGl.h"
#include "gl/Texture.h"
#include "gl/FrameBufferObject.h"
#include "gl/ShaderSource.h"
#include "gl/Shader.h"
#include "gl/ScreenQuad.h"
#include "gl/VertexArrayObject.h"
#include "geom/GeometryFactory.h"
#include "geom/Geometry.h"
#include "io/streamoperators_qt.h"

#if 0
#   include "io/log.h"
#   define MO_NEURO_DEBUG(arg__) MO_PRINT("NeuroGl::" << arg__)
#else
#   define MO_NEURO_DEBUG(unused__) { }
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

    struct StageConfig
    {
        StageConfig()
            : isInputSigned         (true)
            , isInputWeightSigned   (true)
            , isInputErrorSigned    (true)
            , isOutputSigned        (true)
            , isOutputWeightSigned  (true)
            , isOutputErrorSigned   (true)
            , isErrorIsLabel        (false)
            , isClampAlpha          (false)
        { }

        bool isInputSigned, isInputWeightSigned, isInputErrorSigned,
             isOutputSigned, isOutputWeightSigned, isOutputErrorSigned,
             isErrorIsLabel, isClampAlpha;
    };

    Private(NeuroGl* p)
        : p                     (p)
        , texIn                 (0)
        , texWeight             (0)
        , texError              (0)
        , texPrevWeight         (0)
        , outputFormat          (gl::GL_RGBA32F)
        , typeDimension         (4)
        , doRecompile           (true)
        , doResetWeights        (true)
        , mode                  (MODE_BYPASS)
        , activation            (A_LINEAR)
        , learnrate             (0.1)
        , weightInitAmp         (.5)
        , weightInitOffset      (0.)
        , weightInitLocal       (0.)
        , weightInitLocalPow    (1.)
        , randomSeed            (0)
    {
        stages.resize(3);
    }

    GL::ShaderSource getDefaultSource(
            const QSize& resIn, const QSize& resOut, const QSize& resWeight,
            const StageConfig* = 0) const;
    void updateGl();
    /** Returns the valid output texture, or NULL */
    const GL::Texture * getOutputTexture(Stage*) const;
    bool updateFbo(GL::FrameBufferObject*& fbo, const QSize& res) const;
    void updateShader(Stage* s, const GL::ShaderSource& src, const QString& name) const;
    void updateQuad(Stage* s, const QString& name) const;
    void updateBypassStage(const QSize& resIn, const QSize& resOut, Stage*) const;
    void updateErrorStage(const QSize& resIn, const QSize& resOut, Stage*, StageConfig* = 0) const;
    void updateBPropStage(const QSize& resIn, const QSize& resOut,
                          const QSize& resWeight, Stage*, StageConfig* = 0) const;
    void updateFPropStage(const QSize& resIn, const QSize& resOut,
                          const QSize& resWeight, Stage*) const;
    void updateWeightInitStage(const QSize& resIn, const QSize& resOut,
                          const QSize& resWeight, Stage*) const;
    void clearStage(Stage*);
    bool checkStageReady(Stage*) const;
    void releaseGl();
    void bindInput(const GL::Texture * texIn = 0);
    void bindWeight(const GL::Texture * texIn = 0);
    void bindError(const GL::Texture * texIn = 0);
    void renderStage(Stage* s);
    void step();

    NeuroGl* p;

    std::vector<Stage> stages;

    const GL::Texture* texIn, *texWeight, *texError;
    mutable GL::Texture *texPrevWeight;
    gl::GLenum outputFormat;
    int typeDimension;
    bool doRecompile, doResetWeights;

    Mode mode;
    Activation activation;
    float learnrate,
        weightInitAmp, weightInitOffset,
        weightInitLocal, weightInitLocalPow;
    QSize inputRes, outputRes, weightRes;
    int randomSeed;
    StageConfig defaultConfig;
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

GL::ShaderSource NeuroGl::shaderSource(size_t i)
{
    if (i < p_->stages.size()
            && p_->stages[i].shader
            && p_->stages[i].shader->source())
        return *p_->stages[i].shader->source();
    if (i < p_->stages.size())
        return p_->getDefaultSource(p_->inputRes, p_->outputRes, p_->weightRes);
    return GL::ShaderSource();
}

NeuroGl::Mode NeuroGl::mode() const { return p_->mode; }
NeuroGl::Activation NeuroGl::activation() const { return p_->activation; }

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

bool NeuroGl::isInputSigned() const { return p_->defaultConfig.isInputSigned; }
bool NeuroGl::isOutputSigned() const { return p_->defaultConfig.isOutputSigned; }
bool NeuroGl::isClampAlpha() const { return p_->defaultConfig.isClampAlpha; }
bool NeuroGl::isErrorIsLabel() const { return p_->defaultConfig.isErrorIsLabel; }

// ------ setter ------------

void NeuroGl::setMode(Mode m)
    { p_->doRecompile |= p_->mode != m; p_->mode = m; }
void NeuroGl::setActivation(Activation m)
    { p_->doRecompile |= p_->activation != m; p_->activation = m; }

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

void NeuroGl::setInputTexture(const GL::Texture* t)
{
    MO_NEURO_DEBUG("setInputTexture(" << t << (t ? " (" + t->name() + ")" : QString()) << ")");
    p_->texIn = t;
}
void NeuroGl::setWeightTexture(const GL::Texture* t)
{
    MO_NEURO_DEBUG("setWeightTexture(" << t << (t ? " (" + t->name() + ")" : QString()) << ")");
    p_->texWeight = t;
}
void NeuroGl::setErrorTexture(const GL::Texture* t)
{
    MO_NEURO_DEBUG("setErrorTexture(" << t << (t ? " (" + t->name() + ")" : QString()) << ")");
    p_->texError = t;
}
void NeuroGl::setOutputFormat(int glenum)
    { p_->doRecompile |= p_->outputFormat != (gl::GLenum)glenum;
        p_->outputFormat = (gl::GLenum)glenum; }
void NeuroGl::setTypeDimension(int d)
    { p_->doRecompile |= p_->typeDimension != d; p_->typeDimension = d; }

void NeuroGl::setInputSigned(bool b)
    { p_->doRecompile |= p_->defaultConfig.isInputSigned != b; p_->defaultConfig.isInputSigned = b; }
void NeuroGl::setInputWeightSigned(bool b)
    { p_->doRecompile |= p_->defaultConfig.isInputWeightSigned != b; p_->defaultConfig.isInputWeightSigned = b; }
void NeuroGl::setInputErrorSigned(bool b)
    { p_->doRecompile |= p_->defaultConfig.isInputErrorSigned != b; p_->defaultConfig.isInputErrorSigned = b; }
void NeuroGl::setOutputSigned(bool b)
    { p_->doRecompile |= p_->defaultConfig.isOutputSigned != b; p_->defaultConfig.isOutputSigned = b; }
void NeuroGl::setOutputWeightSigned(bool b)
    { p_->doRecompile |= p_->defaultConfig.isOutputWeightSigned != b; p_->defaultConfig.isOutputWeightSigned = b; }
void NeuroGl::setOutputErrorSigned(bool b)
    { p_->doRecompile |= p_->defaultConfig.isOutputErrorSigned != b; p_->defaultConfig.isOutputErrorSigned = b; }

void NeuroGl::setClampAlpha(bool b)
    { p_->doRecompile |= p_->defaultConfig.isClampAlpha != b; p_->defaultConfig.isClampAlpha = b; }
void NeuroGl::setErrorIsLabel(bool b)
    { p_->doRecompile |= p_->defaultConfig.isErrorIsLabel != b; p_->defaultConfig.isErrorIsLabel = b; }

void NeuroGl::setResetWeights(bool e) { p_->doResetWeights = e; }

// ----- opengl ------------

void NeuroGl::releaseGl() { p_->releaseGl(); }

void NeuroGl::Private::bindInput(const GL::Texture * texIn)
{
    if (!texIn)
        texIn = Private::texIn;

    if (texIn && texIn->isAllocated())
    {
        MO_NEURO_DEBUG("bind input texture '"
                       << texIn->name() << "' to slot " << SLOT_INPUT);
        GL::Texture::setActiveTexture(SLOT_INPUT);
        texIn->bind();
    }
    else
        MO_GL_WARNING("NeuroGl: No input texture bound");
}

void NeuroGl::Private::bindWeight(const GL::Texture * texWeight)
{
    if (!texWeight)
        texWeight = Private::texWeight;

    if (texWeight)
    {
        MO_NEURO_DEBUG("bind weight texture '"
                       << texWeight->name() << "' to slot " << SLOT_WEIGHT);
        GL::Texture::setActiveTexture(SLOT_WEIGHT);
        texWeight->bind();
    }
    else
        MO_GL_WARNING("NeuroGl: No weight texture bound");
}

void NeuroGl::Private::bindError(const GL::Texture* texError)
{
    if (!texError)
        texError = Private::texError;

    if (texError)
    {
        MO_NEURO_DEBUG("bind error texture '"
                       << texError->name() << "' to slot " << SLOT_ERROR);
        GL::Texture::setActiveTexture(SLOT_ERROR);
        texError->bind();
    }
    else
        MO_GL_WARNING("NeuroGl: No error texture bound");
}

const GL::Texture* NeuroGl::Private::getOutputTexture(Stage* stage) const
{
    return stage->fbo
            && stage->fbo->numColorTextures()
                && stage->fbo->colorTexture()
                    && stage->fbo->colorTexture()->isAllocated()
                        ? stage->fbo->colorTexture() : 0;
}

const GL::Texture* NeuroGl::outputTexture() const
{
    return p_->getOutputTexture(&p_->stages[0]);
}

const GL::Texture* NeuroGl::weightOutputTexture() const
{
    return p_->getOutputTexture(&p_->stages[1]);
}

const GL::Texture* NeuroGl::errorOutputTexture() const
{
    // return input error texture
    // if we do not process the error ourselves
    if (p_->mode == MODE_FULL_BP
     && !p_->defaultConfig.isErrorIsLabel)
        return p_->texError;

    return p_->getOutputTexture(&p_->stages[2]);
}

void NeuroGl::updateGl() { p_->updateGl(); }

void NeuroGl::step()
{
    p_->step();

    GL::Texture::setActiveTexture(0);
}

bool NeuroGl::Private::updateFbo(GL::FrameBufferObject *&fbo, const QSize &res) const
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
        return true;
    }
    return false;
}

void NeuroGl::Private::updateShader(
        Stage* s, const GL::ShaderSource& src, const QString& name) const
{
    MO_NEURO_DEBUG("updateShader(" << s << ", " << name << ")");

    if (s->shader)
    {
        if (s->shader->isReady())
        {
            // keep if source is unchanged
            if (*s->shader->source() == src)
                return;

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
        if (u_tex) { u_tex->ints[0] = SLOT_INPUT;
            MO_NEURO_DEBUG(u_tex->name() << ": slot " << u_tex->ints[0]); }
        u_tex = s->shader->getUniform("u_tex_weight");
        if (u_tex) { u_tex->ints[0] = SLOT_WEIGHT;
            MO_NEURO_DEBUG(u_tex->name() << ": slot " << u_tex->ints[0]); }
        u_tex = s->shader->getUniform("u_tex_error");
        if (u_tex) { u_tex->ints[0] = SLOT_ERROR;
            MO_NEURO_DEBUG(u_tex->name() << ": slot " << u_tex->ints[0]); }
        u_tex = s->shader->getUniform("u_tex_prev_out");
        if (u_tex) { u_tex->ints[0] = SLOT_PREV_OUT;
            MO_NEURO_DEBUG(u_tex->name() << ": slot " << u_tex->ints[0]); }

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
            g->releaseRef("NeuroGl::updateQuad failed");
            delete s->quad;
            s->quad = 0;
            e << "\nwhen creating shader quad for NeuroGl stage";
            throw;
        }
        g->releaseRef("NeuroGl::updateQuad finish");
    }
}

GL::ShaderSource NeuroGl::Private::getDefaultSource(
        const QSize& resIn, const QSize& resOut, const QSize& resWeight,
        const StageConfig* conf) const
{
    if (!conf)
        conf = &defaultConfig;

    GL::ShaderSource src;

    src.loadVertexSource(":/shader/neuro.vert");
    src.loadFragmentSource(":/shader/neuro.frag");

    src.pasteDefaultIncludes();

    QString defines;
    defines += QString(
                "// --- config ---\n\n"
                "#define TYPE_DIMENSION       %1\n"
                "#define SIGNED_INPUT         %2\n"
                "#define SIGNED_INPUT_WEIGHT  %3\n"
                "#define SIGNED_INPUT_ERROR   %4\n"
                "#define SIGNED_OUTPUT        %5\n"
                "#define SIGNED_OUTPUT_WEIGHT %6\n"
                "#define SIGNED_OUTPUT_ERROR  %7\n"
                "#define CLAMP_ALPHA_1        %8\n"
                "#define LABEL_INPUT          %9\n"
                "#define ACTIVATION           %10\n"
                "\n// --- resolutions --\n\n")
            .arg(typeDimension)
            .arg(conf->isInputSigned ? 1 : 0)
            .arg(conf->isInputWeightSigned ? 1 : 0)
            .arg(conf->isInputErrorSigned ? 1 : 0)
            .arg(conf->isOutputSigned ? 1 : 0)
            .arg(conf->isOutputWeightSigned ? 1 : 0)
            .arg(conf->isOutputErrorSigned ? 1 : 0)
            .arg(conf->isClampAlpha ? 1 : 0)
            .arg(conf->isErrorIsLabel || mode == MODE_ERROR ? 1 : 0)
            .arg(activation)
            ;

    if (!resIn.isEmpty())
    defines += QString(
                "// resolution of input texture (0)\n"
                "#define INPUT_RES ivec2(%1, %2)\n")
                .arg(resIn.width()).arg(resIn.height());
    if (!resOut.isEmpty())
    defines += QString(
                "// res of output texture (actual output of shader stage)\n"
                "#define OUTPUT_RES ivec2(%1, %2)\n")
                .arg(resOut.width()).arg(resOut.height());
    if (!resWeight.isEmpty())
    defines += QString(
                "// res of weight texture (1)\n"
                "#define WEIGHT_RES ivec2(%1, %2)\n")
                .arg(resWeight.width()).arg(resWeight.height());
    // XXX: ERROR_RES is currently not different to OUTPUT_RES!
    if (!resOut.isEmpty())
    defines += QString(
                "// resolution of error texture (2)\n"
                "#define ERROR_RES ivec2(%1, %2)\n")
                .arg(resOut.width()).arg(resOut.height());


    defines += "\n";

    if (!resIn.isEmpty())
        defines += "const int NUM_IN = INPUT_RES.x * INPUT_RES.y;\n";
    if (!resOut.isEmpty())
        defines += "const int NUM_OUT = OUTPUT_RES.x * OUTPUT_RES.y;\n";
    if (!resWeight.isEmpty())
        defines += "const int NUM_WEIGHT = WEIGHT_RES.x * WEIGHT_RES.y;\n";

    src.addDefine(defines);

    //std::cout << defines << std::endl;
    //std::cout << src.addLineNumbers(src.fragmentSource()) << std::endl;
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
    // take output resolution from label/error texture
    if ((mode == MODE_BPROP
         || mode == MODE_ERROR
         || mode == MODE_FULL_BP) && texError)
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
            clearStage(&stages[2]);
        break;

        case MODE_ERROR:
            inputRes = resIn;
            outputRes = resOut;
            clearStage(&stages[0]);
            clearStage(&stages[1]);
            updateErrorStage(resIn, resOut, &stages[2]);
        break;

        case MODE_FPROP:
            inputRes = resIn;
            outputRes = resOut;
            weightRes = resWeight;
            updateFPropStage(resIn, resOut, resWeight, &stages[0]);
            clearStage(&stages[1]);
            clearStage(&stages[2]);
        break;

        case MODE_BPROP:
            inputRes = resIn;
            outputRes = resOut;
            weightRes = resWeight;
            clearStage(&stages[0]);
            updateBPropStage(resIn, resOut, resWeight, &stages[1]);
            clearStage(&stages[2]);
        break;

        case MODE_FULL_BP:
        {
            inputRes = resIn;
            outputRes = resOut;
            weightRes = resWeight;
            updateFPropStage(resIn, resOut, resWeight, &stages[0]);

            auto conf = defaultConfig;
            conf.isErrorIsLabel = false;
            conf.isInputSigned = defaultConfig.isOutputSigned;
            conf.isInputErrorSigned = defaultConfig.isOutputErrorSigned;
            conf.isInputWeightSigned = defaultConfig.isOutputWeightSigned;
            updateBPropStage(resIn, resOut, resWeight, &stages[1], &conf);

            if (defaultConfig.isErrorIsLabel)
            {
                conf = defaultConfig;
                //conf.isErrorIsLabel = false;
                conf.isInputErrorSigned = defaultConfig.isOutputErrorSigned;
                updateErrorStage(resIn, resOut, &stages[2], &conf);
            }
            else
                clearStage(&stages[2]);
        }
        break;

        case MODE_WEIGHT_INIT:
            inputRes = resIn;
            outputRes = resOut;
            weightRes = resWeight;
            clearStage(&stages[0]);
            updateWeightInitStage(resIn, resOut, resWeight, &stages[1]);
            clearStage(&stages[2]);
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
    {
        MO_NEURO_DEBUG("dimensions not set for BYPASS stage");
        return;
    }

    updateFbo(s->fbo, resOut);

    if (doRecompile)
    {
        GL::ShaderSource src = getDefaultSource(resIn, resOut, QSize());
        src.addDefine("#define MODE M_BYPASS");
        updateShader(s, src, "bypass");
        updateQuad(s, "bypass");
    }
}

void NeuroGl::Private::updateErrorStage(
        const QSize& resIn, const QSize& resOut, Stage* s, StageConfig* conf) const
{
    MO_NEURO_DEBUG("updateErrorStage(" << resIn << ", " << resOut << ", " << s << ")"
                   " doRecompile=" << doRecompile);

    if (resIn.isEmpty() || resOut.isEmpty())
    {
        MO_NEURO_DEBUG("dimensions not set for ERROR stage");
        return;
    }

    updateFbo(s->fbo, resOut);

    if (doRecompile)
    {
        GL::ShaderSource src = getDefaultSource(resIn, resOut, QSize(), conf);
        src.addDefine("#define MODE M_ERROR");
        updateShader(s, src, "geterror");
        updateQuad(s, "geterror");
    }
}

void NeuroGl::Private::updateFPropStage(
        const QSize& resIn, const QSize& resOut, const QSize& resWeight,
        Stage* s) const
{
    if (resIn.isEmpty() || resOut.isEmpty() || resWeight.isEmpty())
    {
        MO_NEURO_DEBUG("dimensions not set for FPROP stage");
        return;
    }
    updateFbo(s->fbo, resOut);

    if (doRecompile)
    {
        GL::ShaderSource src = getDefaultSource(resIn, resOut, resWeight);
        src.addDefine("#define MODE M_FPROP");
        updateShader(s, src, "fprop");
        updateQuad(s, "fprop");
    }
}

void NeuroGl::Private::updateBPropStage(
        const QSize& resIn, const QSize& resOut, const QSize& resWeight,
        Stage* s, StageConfig* conf) const
{
    if (resIn.isEmpty() || resOut.isEmpty() || resWeight.isEmpty())
    {
        MO_NEURO_DEBUG("dimensions not set for BPROP stage");
        return;
    }

    if (updateFbo(s->fbo, resWeight))
        texPrevWeight = 0;

    if (doRecompile)
    {
        GL::ShaderSource src = getDefaultSource(resIn, resOut, resWeight, conf);
        src.addDefine("#define MODE M_BPROP");
        updateShader(s, src, "bprop");
        updateQuad(s, "bprop");
    }
}

void NeuroGl::Private::updateWeightInitStage(
        const QSize& resIn, const QSize& resOut, const QSize& resWeight, Stage* s) const
{
    if (resIn.isEmpty() || resOut.isEmpty() || resWeight.isEmpty())
    {
        MO_NEURO_DEBUG("dimensions not set for WEIGHT_INIT stage");
        return;
    }

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
    bool ready = s->fbo && s->fbo->isCreated()
         && s->shader && s->shader->isReady()
         && s->quad && s->quad->isCreated();
    if (!ready)
        MO_NEURO_DEBUG("stage not ready");
    return ready;
}

void NeuroGl::Private::step()
{
    updateGl();

    MO_CHECK_GL_THROW( gl::glDisable(gl::GL_BLEND) );

    switch (mode)
    {
        case MODE_BYPASS:
        {
            if (!checkStageReady(&stages[0]))
                return;

            bindInput();
            renderStage(&stages[0]);
        }
        break;

        case MODE_ERROR:
        {
            if (!checkStageReady(&stages[2]))
                return;

            bindInput();
            bindError();
            renderStage(&stages[2]);
        }
        break;

        case MODE_FPROP:
        {
            if (!checkStageReady(&stages[0]))
                return;

            bindInput();
            bindWeight();

            renderStage(&stages[0]);
        }
        break;

        case MODE_BPROP:
        {
            if (!checkStageReady(&stages[1]))
                return;

            bindInput();
            bindWeight();
            bindError();

            renderStage(&stages[1]);
        }
        break;

        case MODE_FULL_BP:
        {
            if (!checkStageReady(&stages[0])
                || !checkStageReady(&stages[1])
                || (defaultConfig.isErrorIsLabel && !checkStageReady(&stages[2])))
                    return;

            bindInput();

            // bind previous output weights
            if (!doResetWeights && texPrevWeight && texPrevWeight->isAllocated())
            {
                bindWeight(texPrevWeight);
            }
            // bind input weights
            else
            {
                doResetWeights = false;
                bindWeight();
            }

            // fprop
            renderStage(&stages[0]);

            if (defaultConfig.isErrorIsLabel)
            {
                // error
                bindInput(p->outputTexture());
                bindError();
                renderStage(&stages[2]);

                bindError(p->errorOutputTexture());
            }
            else
                bindError();

            // bprop
            renderStage(&stages[1]);

            // remember previous weights is present
            auto w = p->weightOutputTexture();
            if (w && w->isAllocated())
            {
                // swap weight output target/source
                GL::FrameBufferObject* fbo = stages[1].fbo;
                fbo->bind();
                if (texPrevWeight)
                {
                    MO_NEURO_DEBUG("Exchanging weight output texture '"
                                   << texPrevWeight << "'");
                    texPrevWeight = fbo->swapColorTexture(texPrevWeight);
                }
                else
                {
                    MO_NEURO_DEBUG("Getting weight output texture");
                    auto tex = GL::Texture::constructFrom(w);
                    tex->create();
                    texPrevWeight = fbo->swapColorTexture(tex);
                }
                fbo->unbind();
            }

        }
        break;

        case MODE_WEIGHT_INIT:
        {
            if (!checkStageReady(&stages[1]))
                return;

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
    //s->shader->dumpUniforms();
    s->quad->drawElements();
    s->shader->deactivate();
    s->fbo->unbind();
    s->fbo->setChanged();
}

} // namespace MO
