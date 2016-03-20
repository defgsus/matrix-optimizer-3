/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/19/2016</p>
*/

#include <QThread>

#include "shaderao.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parametertext.h"
#include "audio/tool/audiobuffer.h"
#include "gl/offscreencontext.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/vertexarrayobject.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "geom/geometry.h"
#include "geom/geometryfactory.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

#if 0
#   define MO_DEBUG_SAO(arg__) MO_DEBUG("ShaderAO::" << arg__)
#else
#   define MO_DEBUG_SAO(unused__)
#endif

namespace MO {

MO_REGISTER_OBJECT(ShaderAO)

class ShaderAO::Private
{
    public:

    Private(ShaderAO *ao)
        : ao        (ao)
        , context   (0)
        , fbo       (0)
        , shader    (0)
        , vao       (0)
    { }

    ~Private()
    {
        releaseGl();
    }

    void getSource(GL::ShaderSource* src);
    void initGl();
    void releaseGl();
    void render(const RenderTime& time);

    ShaderAO * ao;
    GL::OffscreenContext * context;
    GL::FrameBufferObject * fbo;
    GL::Shader * shader;
    GL::VertexArrayObject * vao;
    std::vector<gl::GLfloat> texData;
    size_t texW, texH;

    bool needRecompile;
    QString failedCode;
    ParameterText* paramGlsl;

    GL::Uniform
            *u_time,
            *u_resolution,
            *u_samplerate,
            *u_buffersize,
            *u_iSampleRate;
};

ShaderAO::ShaderAO()
    : AudioObject   ()
    , p_         (new Private(this))
{
    setName("AudioShader");
    setNumberAudioInputsOutputs(4, 4);
}

ShaderAO::~ShaderAO()
{
    delete p_;
}

void ShaderAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aoshader",1);
}

void ShaderAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aoshader",1);
}

void ShaderAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("shader", tr("shader"));
    initParameterGroupExpanded("shader");

        p_->paramGlsl = params()->createTextParameter(
                    "glsl", tr("shader source"),
                    tr("The glsl source to generate audio data"),
                    TT_GLSL,
                    "// can return float, or vec2-4"
                    "vec4 mainSound(in float time)\n"
                    "{\n"
                    "\treturn vec4(sin(time * 440. * 3.14159265*2.));\n"
                    "}\n");

    params()->endParameterGroup();
}

void ShaderAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    p_->needRecompile = true;
}

void ShaderAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->paramGlsl)
        p_->needRecompile = true;
}

void ShaderAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();
}


void ShaderAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
}


void ShaderAO::onCloseAudioThread()
{
    AudioObject::onCloseAudioThread();
    p_->releaseGl();
}

GL::ShaderSource ShaderAO::valueShaderSource(uint channel) const
{
    GL::ShaderSource src;
    if (channel == 0)
        p_->getSource(&src);
    return src;
}

void ShaderAO::Private::getSource(GL::ShaderSource *src)
{
    src->loadVertexSource(":/shader/shader_ao.vert");
    src->loadFragmentSource(":/shader/shader_ao.frag");
    src->replace("//%mo_user_function%", paramGlsl->baseValue());
}

void ShaderAO::Private::initGl()
{
    MO_DEBUG_SAO("initGl()");
    texW = 1024; texH = 1;
    try
    {
        // ----------- context ------------

        if (!context)
        {
            context = new GL::OffscreenContext;
            context->createGl();
        }
        else
            context->makeCurrent();

        context->setSize(QSize(texW, texH));
        gl::glClearColor(0,0,0,0);
        gl::glDisable(gl::GL_BLEND);

        // ---------- framebuffer ---------

        if (!fbo)
            fbo = new GL::FrameBufferObject(
                        texW, texH, gl::GL_RGBA16F, gl::GL_FLOAT);
        else
            if (fbo->isCreated())
                fbo->release();
        fbo->create();
        texData.resize(texW*texH*4);

        // ------------- shader ---------------

        if (!shader)
            shader = new GL::Shader("audio");
        else
            if (shader->isReady())
                shader->releaseGL();

        GL::ShaderSource src;
        getSource(&src);

        shader->setSource(src);

        try
        {
            failedCode.clear();
            shader->compile();
        }
        catch (...)
        {
            failedCode = paramGlsl->baseValue();
            throw;
        }

        u_time = shader->getUniform("u_time", true);
        u_resolution = shader->getUniform("u_resolution", true);
        u_samplerate = shader->getUniform("u_samplerate", true);
        u_buffersize = shader->getUniform("u_buffersize", true);
        u_iSampleRate = shader->getUniform("iSampleRate");

        // --------------- quad geom -----------

        if (!vao)
            vao = new GL::VertexArrayObject("audio_quad");
        else if (vao && vao->isCreated())
        {
            vao->release();
            vao->clear();
        }

        auto geom = new GEOM::Geometry;
        ScopedRefCounted del(geom, "shader quad");

        GEOM::GeometryFactory::createQuad(geom, 2., 2.);

        geom->getVertexArrayObject(vao, shader);

        needRecompile = false;
    }
    catch (const Exception& e)
    {
        for (auto msg : shader->compileMessages())
            if (msg.program == GL::Shader::P_FRAGMENT
              || msg.program == GL::Shader::P_LINKER)
                paramGlsl->addErrorMessage(msg.line, msg.text);

        releaseGl();
        throw;
    }
}

void ShaderAO::Private::releaseGl()
{
    MO_DEBUG_SAO("releaseGl()");

    if (vao && vao->isCreated())
        vao->release();
    delete vao;
    vao = 0;
    if (shader && shader->isReady())
        shader->releaseGL();
    delete shader;
    shader = 0;
    if (fbo && fbo->isCreated())
        fbo->release();
    delete fbo;
    fbo = 0;
    if (context)
        context->destroyGl();
    delete context;
    context = 0;
}

void ShaderAO::Private::render(const RenderTime& time)
{
    MO_DEBUG_SAO("render(" << time << ")");

    context->makeCurrent();

    fbo->bind();
    fbo->setViewport();
    gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

    if (u_time)
        u_time->setFloats(time.second());
    if (u_resolution)
        u_resolution->setFloats(
                    fbo->width(), fbo->height(), 1./fbo->width(), 1./fbo->height());
    if (u_samplerate)
        u_samplerate->setFloats(ao->sampleRate(), ao->sampleRateInv());
    if (u_buffersize)
        u_buffersize->ints[0] = time.bufferSize();

    if (u_iSampleRate)
        u_iSampleRate->floats[0] = ao->sampleRate();

    shader->activate();
    shader->sendUniforms();
    vao->drawElements();
    shader->deactivate();

    MO_DEBUG_SAO("download tex");
    fbo->colorTexture(0)->download(&texData[0], gl::GL_RGBA, gl::GL_FLOAT);

    fbo->unbind();
}

void ShaderAO::processAudio(const RenderTime& time)
{
    MO_DEBUG_SAO("processAudio(" << time << ")");

    if ((!p_->context
        || p_->needRecompile)
        && (p_->failedCode.isEmpty()
            || p_->paramGlsl->baseValue() != p_->failedCode))
        p_->initGl();

    if (!p_->context)
    {
        if (p_->failedCode.isEmpty())
            setErrorMessage("No OpenGL context");
        return;
    }

    if (p_->context->thread() != QThread::currentThread())
    {
        setErrorMessage("Audio thread change without notice");
        return;
    }

    p_->render(time);

    auto outs = audioOutputs(time.thread());
    size_t k=0;
    for (auto out : outs)
    {
        if (out)
        {
            const size_t num = std::min(p_->texW, out->blockSize());
            for (size_t i=0; i<num; ++i)
                out->write(i, p_->texData[i*4 + k]);
        }
        ++k;
    }
}


} // namespace MO
