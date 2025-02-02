/** @file shader.cpp

    @brief GLSL shader wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created about 6/2014 as part of github.com/defgsus/scheeder</p>
    <p>reworked for MO3 7/27/2014</p>
*/

#include <QStringList>

#include "Shader.h"
#include "io/log_gl.h"
#include "ShaderSource.h"

using namespace gl;

namespace MO {
namespace GL {


void privateUniformDeleter(Uniform * u) { delete u; }
void privateAttributeDeleter(Attribute * a) { delete a; }


Uniform::Uniform(Shader * s)
    : type_     (GLenum(0))
    , size_     (0)
    , location_ (0)
    , autoSend_ (true)
    , shader_   (s)
{
    floats[0] = floats[1] = floats[2] = floats[3] = 0.f;
    ints[0] = ints[1] = ints[2] = ints[3] = 0;
}

void Uniform::set(const Mat4 & m)
{
    for (int i=0; i<4; ++i)
        for (int j=0; j<4; ++j)
            floats[i*4+j] = m[i][j];
}

void Uniform::copyValuesFrom_(Uniform * u)
{
    for (int i=0; i<16; ++i)
        floats[i] = u->floats[i];

    ints[0] = u->ints[0];
    ints[1] = u->ints[1];
    ints[2] = u->ints[2];
    ints[3] = u->ints[3];
}

void Uniform::send()
{
    shader_->sendUniform(this);
}


Attribute::Attribute()
    :   type_    (GLenum(0)),
        size_    (0),
        location_(0)
{ }


Shader::Shader(const QString &name)
    : source_             (new ShaderSource()),
      name_               (name.isEmpty()? "unnamed" : name),
      prog_               (-1),
      sourceChanged_      (false),
      ready_              (false),
      activated_          (false)
{
    MO_DEBUG_GL("Shader::Shader('" << name << "')");
}

Shader::~Shader()
{
    MO_DEBUG_GL("Shader(" << name_ << ")::~Shader()");

    if (isReady())
        MO_WARNING("delete of shader object '"
                   << name() << "' with bound resources - OpenGL resource leak");
}

void Shader::setSource(const ShaderSource& s)
{
    if (s.isEmpty())
        MO_GL_ERROR( "Shader(" << name_ << ")::setSource() with empty source" );

    *source_ = s;
    sourceChanged_ = true;
}

Uniform * Shader::getUniform(size_t index)
{
    MO_ASSERT(index < uniforms_.size(), "uniform index out of range");
    return uniforms_[index].get();
}

Uniform * Shader::getUniform(const QString &name, bool expect)
{
    for (auto u : uniformList_)
        if (u->name() == name)
            return u;

    if (expect)
        MO_GL_ERROR(
            "Uniform '" << name << "' expected but not found in Shader(" << name_ << ")");

    return 0;
}

const Attribute * Shader::getAttribute(size_t index) const
{
    MO_ASSERT(index < attribs_.size(), "attribute index out of range in Shader(" << name_ << ")");
    return attribs_[index].get();
}

const Attribute * Shader::getAttribute(const QString &name) const
{
    for (auto a : attributeList_)
        if (a->name() == name)
            return a;
    return 0;
}

void Shader::compile()
{
    // init state
    ready_ = false;
    sourceChanged_ = false;
    log_ = "";
    msg_.clear();
    uniforms_.clear();
    uniformList_.clear();
    attribs_.clear();
    attributeList_.clear();

    // delete previous shader object
    MO_CHECK_GL_THROW( if (glIsProgram(prog_)==GL_TRUE) glDeleteProgram(prog_) );

    // create shader object
    MO_CHECK_GL_THROW( prog_ = glCreateProgram() );

    // test if working
    if (glIsProgram(prog_) == GL_FALSE)
    {
        log_ += "could not create ProgramObject\n";
        MO_GL_ERROR("Could not create ProgramObject (" << prog_ << ")");
    }

    // compile the vertex shader
    compileShader_(GL_VERTEX_SHADER, P_VERTEX,
                   "vertex shader", source_->vertexSource());

    // compile the fragment shader
    compileShader_(GL_FRAGMENT_SHADER, P_FRAGMENT,
                   "fragment shader", source_->fragmentSource());

    if (!source_->geometrySource().isEmpty())
        compileShader_(GL_GEOMETRY_SHADER, P_GEOMETRY,
                       "geometry shader", source_->geometrySource());

    // link program object
    MO_CHECK_GL_THROW( glLinkProgram(prog_) );

    GLint linked;
    MO_CHECK_GL_THROW( glGetProgramiv(prog_, GL_LINK_STATUS, &linked) );
    if (!linked)
    {
        addMessage_(P_LINKER, "shader programm link error");
        prog_ = -1;
    }

    // print linker log
    // (only works when linking succeeded)
    if (linked)
    {
        // generate more description
        glValidateProgram(prog_);

        GLint blen = 0;
        GLsizei slen = 0;
        MO_CHECK_GL_THROW( glGetProgramiv(prog_, GL_INFO_LOG_LENGTH , &blen) );
        if (blen > 1)
        {
            std::vector<GLchar> compiler_log(blen+1);
            MO_CHECK_GL_THROW(
                glGetProgramInfoLog(prog_, blen, &slen, &compiler_log[0]) );
            log_ += "linker log:\n" + QString(&compiler_log[0]) + "\n";
        }
    }

    if (!linked)
    {
        //MO_DEBUG(source_->vertexSource() << "\n\n" << source_->fragmentSource() << "\n\n");
        return;
    }

    getUniforms_();
    getAttributes_();

    // keep copy of previous uniforms
    oldUniforms_ = uniforms_;

    ready_ = true;
}


void Shader::compileShader_(GLenum type, ProgramType pt,
                            const QString& typeName, const QString &source)
{
    if (source.isEmpty())
        return;

    int shadername;
    MO_CHECK_GL_THROW( shadername = glCreateShader(type) );
    if (glIsShader(shadername) == GL_FALSE)
    {
        addMessage_(pt, "error creating " + typeName + " ShaderObject");
        MO_GL_ERROR("Error creating " + typeName + " ShaderObject");
    }

    // get the latin1 char source
    // NOTE: QString::toStdString() is a temporary, we can't just
    // take the pointer to it because the pointed-at memory might
    // get deallocated before use.
    // This is the totally safe way.
    // (unless the driver tries a buffer overflow attack :)
    std::vector<GLchar> src(source.size()+1);
    memcpy(&src[0], source.toStdString().c_str(), source.size());
    const GLchar * psrc[1];
    psrc[0] = &src[0];

    // attach source
    MO_CHECK_GL_THROW( glShaderSource(shadername, 1, psrc, 0) );
    // compile
    MO_CHECK_GL_THROW( glCompileShader(shadername) );

    // check compile status
    bool compiled = false;
    GLint cc;
    MO_CHECK_GL_THROW( glGetShaderiv(shadername, GL_COMPILE_STATUS, &cc); )
    if (!cc)
    {
        log_ += typeName + " compile ERROR\n";
    }
    else
    {
        log_ += typeName + " compiled..\n";
        compiled = true;
    }

    // get compiler log
    GLint blen = 0;
    GLsizei slen = 0;
    MO_CHECK_GL_THROW( glGetShaderiv(shadername, GL_INFO_LOG_LENGTH, &blen) );
    if (blen > 1)
    {
        std::vector<GLchar> compiler_log(blen+1);
        MO_CHECK_GL_THROW(
            glGetShaderInfoLog(shadername, blen, &slen, &compiler_log[0]) );
        QString log = QString(&compiler_log[0]);
        parseLog_(log, pt);
        log_ += "compiler log:\n" + log + "\n";
        MO_DEBUG(log_);
    }

    if (!compiled)
        MO_GL_ERROR("Error compiling shader '" << name_ << "'\n"
                    << log_);

    // attach to programObject
    MO_CHECK_GL_THROW( glAttachShader(prog_, shadername) );
}

void Shader::addMessage_(ProgramType pt, const QString &msg)
{
    log_ += msg + '\n';
    msg_.append( CompileMessage(pt, 0, msg));
}

void Shader::parseLog_(const QString &log, ProgramType pt)
{
    auto lines = log.split('\n', QString::SkipEmptyParts);
    for (const QString& line : lines)
    {
        if (!line.contains("error", Qt::CaseInsensitive))
            continue;

        CompileMessage cm;
        cm.line = 0;
        cm.program = pt;

        // get line number
        int x1 = line.indexOf("("),
            x2 = line.indexOf(")", x1);
//        int x1 = line.indexOf(QRegExp("[0-9]")),
//            x2 = line.indexOf(QRegExp("(?![0-9])"), x1);
//        MO_PRINT("----------- " << x1 << " " << x2);
        if (x1 >= 0 && x2 > x1 + 1)
            cm.line = line.mid(x1+1, x2-x1-1).toInt();

        // get text
        x1 = line.indexOf(':');
        cm.text = line.mid(x1 + 1);

        msg_.append( cm );
    }
}

void Shader::activate()
{
    if (!isReady())
        return;

    MO_CHECK_GL_THROW( glUseProgram(prog_) );
    activated_ = true;
}

void Shader::deactivate()
{
    // TODO:
    // On OSX this gives GL_INVALID_OPERATION
    // although the spec says that's the way to do it
    MO_CHECK_GL_THROW( glUseProgram(0) );
    activated_ = false;
}


void Shader::getUniforms_()
{
    // get number of used uniforms
    GLint numu;
    MO_CHECK_GL_THROW( glGetProgramiv(prog_, GL_ACTIVE_UNIFORMS, &numu) );

    // get max length of variable names
    GLint labelLength;
    MO_CHECK_GL_THROW( glGetProgramiv(prog_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &labelLength) );

    // don't expose these to user
    QStringList specialUniforms;// XXX = appsettings()->getShaderUniforms();

    // get each uniform data
    for (int i=0; i<numu; ++i)
    {
        Uniform * u = new Uniform(this);

        // plain old char* strings always need a bit of extra code ..
        GLsizei length;
        std::vector<GLchar> name(labelLength);
        MO_CHECK_GL_THROW(
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

        if (u->type() == GL_FLOAT_MAT2
            || u->type() == GL_FLOAT_MAT3
            || u->type() == GL_FLOAT_MAT4)
            u->autoSend_ = false;

        // get location of uniform
        MO_CHECK_GL_THROW( u->location_ = glGetUniformLocation(prog_, &name[0]) );

        //MO_CHECK_GL_THROW( glGetUniformBlockIndex())

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
    MO_DEBUG_GL("Shader(" << name_ << ") uniforms:");
    for (auto u : uniformList_)
        MO_DEBUG_GL(u->location() << "\t " << u->name());
#endif
}

void Shader::getAttributes_()
{
    // get number of used uniforms
    GLint num;
    MO_CHECK_GL_THROW( glGetProgramiv(prog_, GL_ACTIVE_ATTRIBUTES, &num) );

    // get max length of variable names
    GLint labelLength;
    MO_CHECK_GL_THROW( glGetProgramiv(prog_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &labelLength) );

    // get each attributes
    for (int i=0; i<num; ++i)
    {
        Attribute * a = new Attribute();

        // plain old char* strings always need a bit of extra code ..
        GLsizei length;
        std::vector<GLchar> name(labelLength);
        MO_CHECK_GL_THROW(
            glGetActiveAttrib(prog_, i, name.size(), &length, &a->size_, &a->type_, &name[0])
            );
        name.resize(length);
        a->name_ = QString(&name[0]);

        // find location of attribute
        MO_CHECK_GL_THROW( a->location_ = glGetAttribLocation(prog_, &name[0]) );

        // keep in list
        attribs_.push_back(std::shared_ptr<Attribute>(a, privateAttributeDeleter));
        attributeList_.append(a);
    }


#ifdef MO_DO_DEBUG_GL
    MO_DEBUG_GL("Shader(" << name_ << ") attributes:");
    for (auto a : attributeList_)
        MO_DEBUG_GL(a->location() << "\t " << a->name());
#endif
}

void Shader::sendUniform(const Uniform * u)
{
#if 0
    MO_DEBUG_GL("Shader('" << name_ << ")::sendUniform("
                << u->name() << ", " << u->floats[0]
                << ", " << u->floats[1] << ", " << u->floats[2]
                << ", " << u->floats[3] << ")");
#endif
    switch (u->type())
    {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_INT:
            //MO_PRINT("INT " << u->name() << " " << u->location_ << " " << u->ints[0]);
            MO_CHECK_GL_THROW( glUniform1i(u->location_, u->ints[0]) );
        break;

        case GL_INT_VEC2:
            MO_CHECK_GL_THROW( glUniform2i(u->location_, u->ints[0], u->ints[1]) );
        break;
        case GL_INT_VEC3:
            MO_CHECK_GL_THROW( glUniform3i(u->location_, u->ints[0], u->ints[1], u->ints[2]) );
        break;
        case GL_INT_VEC4:
            MO_CHECK_GL_THROW( glUniform4i(u->location_, u->ints[0], u->ints[1], u->ints[2], u->ints[3]) );
        break;

        case GL_FLOAT:
            MO_CHECK_GL_THROW( glUniform1f(u->location_, u->floats[0]) );
        break;
        case GL_FLOAT_VEC2:
            MO_CHECK_GL_THROW( glUniform2f(u->location_, u->floats[0], u->floats[1]) );
        break;
        case GL_FLOAT_VEC3:
            MO_CHECK_GL_THROW( glUniform3f(u->location_, u->floats[0], u->floats[1], u->floats[2]) );
        break;
        case GL_FLOAT_VEC4:
            //MO_DEBUG_GL(activated_ << "," << u->location_ << "," << u->floats[0] << ","
            //                << u->floats[1] << "," << u->floats[2] << "," << u->floats[3]);
            MO_CHECK_GL_THROW( glUniform4f(u->location_, u->floats[0], u->floats[1], u->floats[2], u->floats[3]) );
        break;

        case GL_FLOAT_MAT2:
            MO_CHECK_GL_THROW( glUniformMatrix2fv(u->location_, 1, GL_FALSE, &u->floats[0]) );
        break;

        case GL_FLOAT_MAT3:
            MO_CHECK_GL_THROW( glUniformMatrix3fv(u->location_, 1, GL_FALSE, &u->floats[0]) );
        break;

        case GL_FLOAT_MAT4:
            //for (int i=0; i<16; ++i) MO_DEBUG(u->floats[i]);
            MO_CHECK_GL_THROW( glUniformMatrix4fv(u->location_, 1, GL_FALSE, &u->floats[0]) );
        break;

        default:
            //MO_GL_WARNING("unhandled uniform type '" << u->type_ << "' in Shader(" << name_ << ")");
        break;
    }
}

void Shader::sendUniforms()
{
    for (size_t i=0; i<numUniforms(); ++i)
    {
        auto u = getUniform(i);
        if (u->autoSend())
            sendUniform(u);
    }
}

void Shader::releaseGL()
{
    MO_CHECK_GL( glDeleteProgram(prog_) );
    ready_ = activated_ = false;
}


void Shader::dumpUniforms(std::ostream &out) const
{
    for (Uniform * u : uniformList_)
    {
        out << "[" << u->name() << "] @ " << u->location() << " autosend "
            << (u->autoSend() ? "on" : "off")
            << "\n  ints(" << u->ints[0] << ", " << u->ints[1] << ", " << u->ints[2] << ", " << u->ints[3] << ") "
            << "floats(" << u->floats[0] << ", " << u->floats[1] << ", " << u->floats[2] << ", " << u->floats[3] << ")"
            << std::endl;
    }
}

QString Shader::compileMessagesString() const
{
    QString s;

    // XXX make this nicer
    for (auto m : msg_)
        s += m.text + "\n";

    return s;
}

} // namespace GL
} // namespace MO
