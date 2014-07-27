/** @file shader.cpp

    @brief GLSL shader wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created about 6/2014 as part of github.com/defgsus/scheeder</p>
    <p>reworked for MO3 7/27/2014</p>
*/

#include <QStringList>

#include "shader.h"
#include "io/log.h"
#include "shadersource.h"

namespace MO {
namespace GL {


void privateUniformDeleter(Uniform * u) { delete u; }
void privateAttributeDeleter(Attribute * a) { delete a; }


Uniform::Uniform()
    :   type_    (0),
        size_    (0),
        location_(0)
{
    floats[0] = floats[1] = floats[2] = floats[3] = 0.f;
    ints[0] = ints[1] = ints[2] = ints[3] = 0;
}

Attribute::Attribute()
    :   type_    (0),
        size_    (0),
        location_(0)
{ }


void Uniform::copyValuesFrom_(Uniform * u)
{
    floats[0] = u->floats[0];
    floats[1] = u->floats[1];
    floats[2] = u->floats[2];
    floats[3] = u->floats[3];
    ints[0] = u->ints[0];
    ints[1] = u->ints[1];
    ints[2] = u->ints[2];
    ints[3] = u->ints[3];
}


Shader::Shader()
    :   prog_             (-1),
        sourceChanged_      (false),
        ready_              (false),
        activated_          (false),
        isGlFuncInitialized_(false)
{
    MO_DEBUG_GL("Shader::Shader()");
}

Shader::~Shader()
{
    MO_DEBUG_GL("Shader::~Shader()");

    if (ready())
        MO_WARNING("delete of shader object with bound resources!");
}

void Shader::setSource(const ShaderSource * s)
{
    if (s->isEmpty())
        MO_GL_WARNING("Shader::setSource() with empty ShaderSource");

    if (!s->fragmentSource().isEmpty())
        setFragmentSource(s->fragmentSource());

    if (!s->vertexSource().isEmpty())
        setVertexSource(s->vertexSource());
}

void Shader::setVertexSource(const QString &text)
{
    vertSource_ = text;
    sourceChanged_ = true;
}

void Shader::setFragmentSource(const QString &text)
{
    fragSource_ = text;
    sourceChanged_ = true;
}

Uniform * Shader::getUniform(size_t index)
{
    MO_ASSERT(index < uniforms_.size(), "uniform index out of range");
    return uniforms_[index].get();
}

Uniform * Shader::getUniform(const QString &name)
{
    for (auto u : uniformList_)
        if (u->name() == name)
            return u;
    return 0;
}

const Attribute * Shader::getAttribute(size_t index) const
{
    MO_ASSERT(index < attribs_.size(), "attribute index out of range");
    return attribs_[index].get();
}

const Attribute * Shader::getAttribute(const QString &name) const
{
    for (auto a : attributeList_)
        if (a->name() == name)
            return a;
    return 0;
}

bool Shader::compile()
{
    if (!isGlFuncInitialized_)
    {
        initializeOpenGLFunctions();
        isGlFuncInitialized_ = true;
    }

    // init state
    ready_ = false;
    sourceChanged_ = false;
    log_ = "";
    uniforms_.clear();
    uniformList_.clear();
    attribs_.clear();
    attributeList_.clear();

    // delete previous shader object
    MO_CHECK_GL( if (glIsProgram(prog_)) glDeleteProgram(prog_) );

    // create shader object
    MO_CHECK_GL( prog_ = glCreateProgram() );

    // test if working
    if (!glIsProgram(prog_))
    {
        log_ += "could not create ProgramObject\n";
        return false;
    }

    // compile the vertex shader
    if (!compileShader_(GL_VERTEX_SHADER, "vertex shader", vertSource_))
        return false;

    // compile the fragment shader
    if (!compileShader_(GL_FRAGMENT_SHADER, "fragment shader", fragSource_))
        return false;

    // link program object
    MO_CHECK_GL( glLinkProgram(prog_) );

    GLint linked;
    MO_CHECK_GL( glGetProgramiv(prog_, GL_LINK_STATUS, &linked) );
    if (!linked)
    {
        log_ += "shader programm link error\n";
        prog_ = -1;
    }

    // print linker log
    GLint blen = 0;
    GLsizei slen = 0;
    MO_CHECK_GL( glGetProgramiv(prog_, GL_INFO_LOG_LENGTH , &blen) );
    if (blen > 1)
    {
        std::vector<GLchar> compiler_log(blen+1);
        MO_CHECK_GL( glGetProgramInfoLog(prog_, blen, &slen, &compiler_log[0]) );
        log_ += "linker log:\n" + QString(&compiler_log[0]) + "\n";
    }


    if (!linked)
        return false;

    getUniforms_();
    getAttributes_();

    // keep copy of previous uniforms
    oldUniforms_ = uniforms_;

    return ready_ = true;
}


bool Shader::compileShader_(GLenum type, const QString& typeName, const QString &source)
{
    if (source.isEmpty())
        return false;

    int shadername;
    MO_CHECK_GL( shadername = glCreateShader(type) );
    if (!glIsShader(shadername))
    {
        log_ += "error creating " + typeName + " ShaderObject\n";
        return false;
    }

    // get the latin1 char source
    // NOTE: QString::toStdString() is a temporary, we can't just
    // take the pointer to it because the pointet-at memory might
    // have been deallocated already.
    // This is the totally safe way.
    // (unless the driver tries a buffer overflow attack :)
    std::vector<GLchar> src(source.size()+1);
    memcpy(&src[0], source.toStdString().c_str(), source.size());
    const GLchar * psrc[1];
    psrc[0] = &src[0];

    // attach source
    MO_CHECK_GL( glShaderSource(shadername, 1, psrc, 0) );
    // compile
    MO_CHECK_GL( glCompileShader(shadername) );

    // check compile status
    bool compiled = false;
    GLint cc;
    MO_CHECK_GL( glGetShaderiv(shadername, GL_COMPILE_STATUS, &cc); )
    if (!cc)
    {
        log_ += typeName + " compile ERROR\n";
    }
    else
    {
        log_ += typeName + " compiled..\n";
        compiled = true;
    }

    // print compiler log
    GLint blen = 0;
    GLsizei slen = 0;
    MO_CHECK_GL( glGetShaderiv(shadername, GL_INFO_LOG_LENGTH , &blen) );
    if (blen > 1)
    {
        std::vector<GLchar> compiler_log(blen+1);
        MO_CHECK_GL( glGetShaderInfoLog(shadername, blen, &slen, &compiler_log[0]) );
        log_ += "compiler log:\n" + QString(&compiler_log[0]) + "\n";
    }

    // attach to programObject
    MO_CHECK_GL( glAttachShader(prog_, shadername) );

    return compiled;
}


void Shader::activate()
{
    if (!ready())
        return;

    MO_CHECK_GL( glUseProgram(prog_) );
    activated_ = true;
}

void Shader::deactivate()
{
    // TODO:
    // On OSX this gives GL_INVALID_OPERATION
    // although the spec says that's the way to do it
    MO_CHECK_GL( glUseProgram(0) );
    activated_ = false;
}


void Shader::getUniforms_()
{
    // get number of used uniforms
    GLint numu;
    MO_CHECK_GL( glGetProgramiv(prog_, GL_ACTIVE_UNIFORMS, &numu) );

    // get max length of variable names
    GLint labelLength;
    MO_CHECK_GL( glGetProgramiv(prog_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &labelLength) );

    // don't expose these to user
    QStringList specialUniforms;// YYY = appSettings->getShaderUniforms();

    // get each uniform data
    for (int i=0; i<numu; ++i)
    {
        Uniform * u = new Uniform;

        // plain old char* strings always need a bit of extra code ..
        GLsizei length;
        std::vector<GLchar> name(labelLength);
        MO_CHECK_GL(
            glGetActiveUniform(prog_, i, name.size(), &length, &u->size_, &u->type_, &name[0])
            );
        name.resize(length);
        u->name_ = QString(&name[0]);

        // discard for special uniforms from the user-interface
        if (specialUniforms.contains(u->name_))
        {
            delete u;
            continue;
        }

        // find location of uniform
        MO_CHECK_GL( u->location_ = glGetUniformLocation(prog_, &name[0]) );

        // keep in list
        uniforms_.push_back(std::shared_ptr<Uniform>(u, privateUniformDeleter));
        uniformList_.append(u);

        // see if we have values from previous uniforms
        for (auto j = oldUniforms_.begin(); j!=oldUniforms_.end(); ++j)
        if (j->get()->name_ == u->name_ && j->get()->type_ == u->type_)
        {
            u->copyValuesFrom_(j->get());
            break;
        }
    }

#ifdef MO_DO_DEBUG_GL
    MO_DEBUG_GL("uniforms:");
    for (auto u : uniformList_)
        MO_DEBUG_GL(u->location() << "\t " << u->name());
#endif
}

void Shader::getAttributes_()
{
    // get number of used uniforms
    GLint num;
    MO_CHECK_GL( glGetProgramiv(prog_, GL_ACTIVE_ATTRIBUTES, &num) );

    // get max length of variable names
    GLint labelLength;
    MO_CHECK_GL( glGetProgramiv(prog_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &labelLength) );

    // get each attributes
    for (int i=0; i<num; ++i)
    {
        Attribute * a = new Attribute();

        // plain old char* strings always need a bit of extra code ..
        GLsizei length;
        std::vector<GLchar> name(labelLength);
        MO_CHECK_GL(
            glGetActiveAttrib(prog_, i, name.size(), &length, &a->size_, &a->type_, &name[0])
            );
        name.resize(length);
        a->name_ = QString(&name[0]);

        // find location of attribute
        MO_CHECK_GL( a->location_ = glGetAttribLocation(prog_, &name[0]) );

        // keep in list
        attribs_.push_back(std::shared_ptr<Attribute>(a, privateAttributeDeleter));
        attributeList_.append(a);
    }


#ifdef MO_DO_DEBUG_GL
    MO_DEBUG_GL("attributes:");
    for (auto a : attributeList_)
        MO_DEBUG_GL(a->location() << "\t " << a->name());
#endif
}

void Shader::sendUniform(const Uniform * u)
{
    switch (u->type())
    {
    case GL_SAMPLER_2D:
    case GL_INT:
        MO_CHECK_GL( glUniform1i(u->location_, u->ints[0]) );
    break;
    case GL_FLOAT:
        MO_CHECK_GL( glUniform1f(u->location_, u->floats[0]) );
    break;
    case GL_FLOAT_VEC2:
        MO_CHECK_GL( glUniform2f(u->location_, u->floats[0], u->floats[1]) );
    break;
    case GL_FLOAT_VEC3:
//        qDebug() << activated_ << u->location_ << u->floats[0] << u->floats[1] << u->floats[2];
        MO_CHECK_GL( glUniform3f(u->location_, u->floats[0], u->floats[1], u->floats[2]) );
    break;
    case GL_FLOAT_VEC4:
        MO_CHECK_GL( glUniform4f(u->location_, u->floats[0], u->floats[1], u->floats[2], u->floats[3]) );
    break;
    default:
        MO_GL_WARNING("unsupported uniform type '" << u->type_ << "'");
    }
}

void Shader::sendUniforms()
{
    for (size_t i=0; i<numUniforms(); ++i)
        sendUniform(getUniform(i));
}

void Shader::releaseGL()
{
    MO_CHECK_GL( glDeleteProgram(prog_) );
    ready_ = activated_ = false;
}


} // namespace GL
} // namespace MO
