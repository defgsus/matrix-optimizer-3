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
        GL::Attribute *u_position;
        GL::Uniform *u_resolution,
                    *u_learnrate;
    };

    Private(NeuroGl* p)
        : p             (p)
        , texIn         (0)
        , texWeight     (0)
        , texError      (0)
        , doRecompile   (true)
        , mode          (MODE_BYPASS)
        , learnrate     (0.1)
        , isInputSigned (true)
        , isOutputSigned(true)
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
    bool doRecompile;

    Mode mode;
    float learnrate;
    QSize inputRes, outputRes, weightRes;
    bool isInputSigned, isOutputSigned;
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

const QSize& NeuroGl::inputRes() const { return p_->inputRes; }
const QSize& NeuroGl::outputRes() const { return p_->outputRes; }
const QSize& NeuroGl::weightRes() const { return p_->weightRes; }

int NeuroGl::outputFormat() const { return (int)p_->outputFormat; }

bool NeuroGl::isInputSigned() const { return p_->isInputSigned; }
bool NeuroGl::isOutputSigned() const { return p_->isOutputSigned; }

// ------ setter ------------

void NeuroGl::setMode(Mode m) { p_->doRecompile = p_->mode != m; p_->mode = m; }

void NeuroGl::setLearnrate(float lr) { p_->learnrate = lr; }

void NeuroGl::setInputRes(const QSize& si)
    { p_->doRecompile = p_->inputRes != si; p_->inputRes = si; }
void NeuroGl::setOutputRes(const QSize& si)
    { p_->doRecompile = p_->outputRes != si; p_->outputRes = si; }
void NeuroGl::setWeightRes(const QSize& si)
    { p_->doRecompile = p_->weightRes != si; p_->weightRes = si; }

void NeuroGl::setInputTexture(const GL::Texture* t) { p_->texIn = t; }
void NeuroGl::setWeightTexture(const GL::Texture* t) { p_->texWeight = t; }
void NeuroGl::setErrorTexture(const GL::Texture* t) { p_->texError = t; }
void NeuroGl::setOutputFormat(int glenum) { p_->outputFormat = (gl::GLenum)glenum; }

void NeuroGl::setInputSigned(bool b)
    { p_->doRecompile = p_->isInputSigned != b; p_->isInputSigned = b; }
void NeuroGl::setOutputSigned(bool b)
    { p_->doRecompile = p_->isOutputSigned != b; p_->isOutputSigned = b; }

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
            s->shader = 0;
        }
    }
    if (!s->shader)
    {
        s->shader = new GL::Shader(name);
        s->shader->setSource(src);
        s->shader->compile();
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
            GEOM::GeometryFactory::createQuad(g, 1, 1);
            g->getVertexArrayObject(s->quad, s->shader);
        }
        catch (...)
        {
            g->releaseRef();
            delete s->quad;
            s->quad = 0;
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
                "#define LABEL_INPUT    %5\n\n"
                "// --- resolutions --\n\n")
            .arg(1)
            .arg(isInputSigned ? 1 : 0)
            .arg(isOutputSigned ? 1 : 0)
            .arg(isInputSigned ? 1 : 0)
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

    src.addDefine(defines);

    //std::cout << src.fragmentSource() << std::endl;
    return src;
}

void NeuroGl::Private::updateGl()
{
    MO_NEURO_DEBUG("updateGl()\n");

    // input resolution
    QSize resIn = inputRes;
    if (texIn)
    {
        QSize s(texIn->width(), texIn->height());
        doRecompile |= resIn != s;
        resIn = s;
    }

    // output resolution
    QSize resOut = outputRes;
    if (texError)
    {
        QSize s(texError->width(), texError->height());
        doRecompile |= resOut != s;
        resOut = s;
    }

    // weight resolution
    QSize resWeight = weightRes;
    if (texWeight)
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
        break;
        case MODE_FPROP:
            inputRes = resIn;
            outputRes = resOut;
            weightRes = resWeight;
            updateFPropStage(resIn, resOut, resWeight, &stages[0]);
        break;
        case MODE_BPROP:
            inputRes = resIn;
            outputRes = resOut;
            weightRes = resWeight;
            updateFPropStage(resIn, resOut, resWeight, &stages[0]);
        break;
    }

    doRecompile = false;
}

void NeuroGl::Private::releaseGl()
{
    // release each stage
    for (Stage& stage : stages)
    {
        if (stage.fbo)
        {
            if (stage.fbo->isCreated())
                stage.fbo->release();
            delete stage.fbo;
            stage.fbo = 0;
        }
        if (stage.shader)
        {
            if (stage.shader->isReady())
                stage.shader->releaseGL();
            delete stage.shader;
            stage.shader = 0;
        }
        if (stage.quad)
        {
            if (stage.quad->isCreated())
                stage.quad->release();
            delete stage.quad;
            stage.quad = 0;
        }
    }
}

void NeuroGl::Private::updateBypassStage(
        const QSize& resIn, const QSize& resOut, Stage* s) const
{
    MO_NEURO_DEBUG("updateBypassStage(" << resIn << ", " << resOut << ", " << s);

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

void NeuroGl::Private::step()
{
    // require input
    if (!texIn)
        return;

    updateGl();

    // something went wrong?
    if (!stages[0].fbo || !stages[0].fbo->isCreated()
        || !stages[0].shader || !stages[0].shader->isReady()
        || !stages[0].quad || !stages[0].quad->isCreated())
    {
        MO_GL_WARNING("NeuroGl not initialized\n"
                      << p->infoString());
        return;
    }

    switch (mode)
    {
        case MODE_BYPASS:
        {
            bindInput();
            renderStage(&stages[0]);
        }
        break;

        case MODE_FPROP:
        {
            if (!texWeight)
                return;

            bindInput();
            bindWeight();

            renderStage(&stages[0]);
        }
        break;

        case MODE_BPROP:
        {
            if (!texWeight)
                return;

            bindInput();
            bindWeight();
            bindError();

            renderStage(&stages[0]);
        }
        break;
    }

}

void NeuroGl::Private::renderStage(Stage* s)
{
    s->fbo->bind();
    s->shader->activate();
    s->quad->drawElements();
    s->shader->deactivate();
    s->fbo->unbind();
}

} // namespace MO
