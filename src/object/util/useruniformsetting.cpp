/** @file useruniformsetting.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.01.2015</p>
*/

#include "useruniformsetting.h"
#include "object/object.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parametertext.h"
#include "object/param/parametertexture.h"
#include "object/param/parameterselect.h"
#include "gl/opengl.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {


bool UserUniformSetting::Uniform::isUsed() const
{
    return p_type->baseValue() != 0;
}

bool UserUniformSetting::Uniform::isBufferTexture() const
{
    int ut = p_type->baseValue();
    return ut == UT_T1 || ut == UT_T2 || ut == UT_T3 || ut == UT_T4;
}

bool UserUniformSetting::Uniform::isTextureInput() const
{
    int ut = p_type->baseValue();
    return ut == UT_TEX || ut == UT_TEX_C;
}

UserUniformSetting::UserUniformSetting(Object * object, uint maxUnis)
    : QObject       (object)
    , object_       (object)
    , num_          (maxUnis)
    , uploadTime_   (-1.1234)
{
}

UserUniformSetting::~UserUniformSetting()
{
}

void UserUniformSetting::serialize(IO::DataStream &io) const
{
    io.writeHeader("uu", 1);
}

void UserUniformSetting::deserialize(IO::DataStream &io)
{
    io.readHeader("uu", 1);
}

void UserUniformSetting::createParameters(const QString &id_suffix)
{
    auto params = object_->params();

    for (uint i=0; i<num_; ++i)
    {
        Uniform u;
        u.uniform = 0;
        u.texture = 0;


        u.p_type = params->createSelectParameter(
            ("uniformtype%1_" + id_suffix).arg(i),
            tr("uniform%1 type").arg(i + 1),
            tr("The type of the uniform variable"),
            { "none", "float", "vec2", "vec3", "vec4",
              "texture", "texturecube", "texture1D", "texture1D2", "texture1D3", "texture1D4" },
            { tr("none"), "float", "vec2", "vec3", "vec4",
              "texture", "texture cube", "float texture1D", "vec2 texture1D", "vec3 texture1D", "vec4 texture1D" },
            { tr("none"), "float", "vec2", "vec3", "vec4",
              "texture", "texture cube", "float texture1D", "vec2 texture1D", "vec3 texture1D", "vec4 texture1D" },
            { UT_NONE, UT_F1, UT_F2, UT_F3, UT_F4, UT_TEX, UT_TEX_C, UT_T1, UT_T2, UT_T3, UT_T4 },
            0,
            true, false);

        u.p_name = params->createTextParameter(("uniformname%1_" + id_suffix).arg(i),
                                               tr("uniform%1 name").arg(i + 1),
                                               tr("The name of the uniform variable as it should go into the header of the shader"),
                                               TT_PLAIN_TEXT,
                                               "",
                                               true, false);

        u.p_length = params->createIntParameter(("uniformtexlen%1_" + id_suffix).arg(i),
                                                tr("uniform%1 length").arg(i + 1),
                                                tr("The length of the array / texture width"),
                                                1024, 16, 16384,
                                                16, true, false);

        u.p_timerange = params->createFloatParameter(
                                                ("uniformtextime%1_" + id_suffix).arg(i),
                                                tr("uniform%1 time range").arg(i + 1),
                                                tr("The time range in seconds to fill the texture"),
                                                1., -10000., 10000.,
                                                .1, true, true);

        u.p_tex = params->createTextureParameter(("uniformtex%1_" + id_suffix).arg(i),
                                                tr("uniform%1 texture").arg(i + 1),
                                                tr("Connects to a texture from somewhere else"));

        static QString compName[] = { "x", "y", "z", "w" };
        for (int j=0; j<4; ++j)
        {
            ParameterFloat * pf = params->createFloatParameter(
                                                    ("uniformdecl%1_%2_" + id_suffix).arg(i).arg(j),
                                                    tr("uniform%2 %1").arg(compName[j]).arg(i+1),
                                                    tr("The %1 value of the %2th uniform").arg(compName[j]).arg(i+1),
                                                    0.0, 0.1,
                                                    true, true);
            u.p_float.push_back(pf);
        }

        uniforms_.push_back(u);
    }
}

bool UserUniformSetting::needsReinit(Parameter *p) const
{
    for (const Uniform & u : uniforms_)
    {
        if (u.p_name == p || u.p_type == p || u.p_length == p)
            return true;
    }

    return false;
}

void UserUniformSetting::updateParameterVisibility()
{
    for (Uniform & u : uniforms_)
    {
        uint type = u.p_type->baseValue();

        u.num_floats = 0;

        if (type == UT_F1 || type == UT_T1)
            u.num_floats = 1;
        if (type == UT_F2 || type == UT_T2)
            u.num_floats = 2;
        if (type == UT_F3 || type == UT_T3)
            u.num_floats = 3;
        if (type == UT_F4 || type == UT_T4)
            u.num_floats = 4;

        if (u.isBufferTexture())
        {
            u.p_length->setVisible(true);
            u.p_timerange->setVisible(true);
        }
        else
        {
            u.p_length->setVisible(false);
            u.p_timerange->setVisible(false);
        }

        for (uint i = 0; i<4; ++i)
            u.p_float[i]->setVisible(i < u.num_floats);

        u.p_name->setVisible(u.num_floats > 0 || u.isTextureInput());

        u.p_tex->setVisible(u.isTextureInput());
    }

    uploadTime_ = -1.12341212; // be sure to upload uniforms next time
}

QString UserUniformSetting::getDeclarations() const
{
    QString decl;
    /* ?? who understands this?
    layout(std140) uniform MyArray
     {
      float myDataArray[size];
     };*/
    for (const Uniform & u : uniforms_)
    if (u.isUsed() && !u.p_name->value().isEmpty())
    {
        QString typestr;
        switch ((UniformType)u.p_type->baseValue())
        {
            case UT_F1: typestr = "float"; break;
            case UT_F2: typestr = "vec2"; break;
            case UT_F3: typestr = "vec3"; break;
            case UT_F4: typestr = "vec4"; break;
            case UT_T1:
            case UT_T2:
            case UT_T3:
            case UT_T4: typestr = "sampler1D"; break;
            case UT_TEX: typestr = "sampler2D"; break;
            case UT_TEX_C: typestr = "samplerCube"; break;
            case UT_NONE: break;
        }

        decl += "uniform " + typestr + " " + u.p_name->value() + ";\n";
    }

    return decl;
}

void UserUniformSetting::tieToShader(GL::Shader * s)
{
    for (Uniform & u : uniforms_)
    if (u.isUsed() && !u.p_name->value().isEmpty())
    {
        if (!u.isBufferTexture())
        {
            u.uniform = s->getUniform(u.p_name->value(), false);

            // check if type is compatible
            if (u.uniform && !(
                        u.uniform->type() == gl::GL_FLOAT
                     || u.uniform->type() == gl::GL_FLOAT_VEC2
                     || u.uniform->type() == gl::GL_FLOAT_VEC3
                     || u.uniform->type() == gl::GL_FLOAT_VEC4
                     || u.uniform->type() == gl::GL_SAMPLER_2D
                     || u.uniform->type() == gl::GL_SAMPLER_CUBE))
                u.uniform = 0;
        }
        // create a texture
        else
        {
            // to set texture slot
            u.uniform = s->getUniform(u.p_name->value(), false);
            // update texture
            if (!u.texture)
            {
                u.texture = new GL::Texture();
                u.texture->setName("float_input");
            }
            if ((int)u.texture->width() != u.p_length->baseValue())
            {
                // find input format
                gl::GLenum informat = gl::GL_RED;
                int ut = u.p_type->baseValue();
                if (ut == UT_T2)
                    informat = gl::GL_RG;
                if (ut == UT_T3)
                    informat = gl::GL_RGB;
                if (ut == UT_T4)
                    informat = gl::GL_RGBA;
                u.texture->create(u.p_length->baseValue(), gl::GL_RGB32F, informat, gl::GL_FLOAT, 0);
            }
        }
    }

    uploadTime_ = -1.12341212;
}

void UserUniformSetting::updateUniforms(Double time, uint thread, uint & texSlot)
{
#if 0 // does not account for changes to incoming parameters
    if (uploadTime_ == time)
        return;
    uploadTime_ = time;
#endif

    for (Uniform & u : uniforms_)
    if (u.uniform)
    {
        if (u.isTextureInput())
        {
            if (const GL::Texture * tex = u.p_tex->value(time, thread))
            {
                //MO_PRINT(object_->name() << ": bind " << tex->name() << " to slot " << texSlot);
                MO_CHECK_GL( gl::glActiveTexture(gl::GL_TEXTURE0 + texSlot) );
                tex->bind();
                u.uniform->ints[0] = texSlot;
                ++texSlot;
                /// @todo generalize texture read parameters for texture inputs
                if (tex->isCube())
                {
                    tex->setTexParameter(gl::GL_TEXTURE_WRAP_S, gl::GLint(gl::GL_CLAMP_TO_EDGE));
                    tex->setTexParameter(gl::GL_TEXTURE_WRAP_S, gl::GLint(gl::GL_CLAMP_TO_EDGE));
                }
            }
        }
        else
        if (!u.isBufferTexture())
        {
            // copy single float value
            for (uint i=0; i<u.num_floats; ++i)
                u.uniform->floats[i] = u.p_float[i]->value(time, thread);
        }
        else
        {
            uint len = u.texture->width(),
                 bsize = len * u.num_floats;
            // update buffer size
            if (u.texBuf.size() != bsize)
                u.texBuf.resize(bsize);

            // sample inputs
            const Double range = u.p_timerange->value(time, thread) / std::max(1, int(len)-1);
            for (uint j=0; j<len; ++j)
            {
                const Double ti = time - range * j;
                for (uint i=0; i<u.num_floats; ++i)
                    u.texBuf[j * u.num_floats + i] = u.p_float[i]->value(ti, thread);
            }

            MO_CHECK_GL( gl::glActiveTexture(gl::GL_TEXTURE0 + texSlot) );
            u.texture->bind();
            u.uniform->ints[0] = texSlot;
            ++texSlot;

            u.texture->upload(&u.texBuf[0]);
        }
    }
}


void UserUniformSetting::releaseGl()
{
    for (Uniform & u : uniforms_)
    {
        if (u.texture)
        {
            if (u.texture->isAllocated())
                u.texture->release();
            delete u.texture;
            u.texture = 0;
        }
    }
}

} // namespace MO
