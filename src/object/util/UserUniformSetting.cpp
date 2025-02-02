/** @file useruniformsetting.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.01.2015</p>
*/

#include "UserUniformSetting.h"
#include "object/Object.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterText.h"
#include "object/param/ParameterTexture.h"
#include "object/param/ParameterTransformation.h"
#include "object/param/ParameterSelect.h"
#include "object/util/TextureSetting.h"
#include "gl/opengl.h"
#include "gl/Shader.h"
#include "gl/Texture.h"
#include "io/DataStream.h"
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

bool UserUniformSetting::Uniform::isTransformation() const
{
    return p_type->baseValue() == UT_TRANS;
}

bool UserUniformSetting::Uniform::isTextureInput() const
{
    int ut = p_type->baseValue();
    return ut == UT_TEX || ut == UT_TEX_C || ut == UT_TEX_3D;
}

UserUniformSetting::UserUniformSetting(Object * object, uint maxUnis)
    : object_       (object)
    , num_          (maxUnis)
    , uploadTime_   (-1.1234)
{
}

UserUniformSetting::~UserUniformSetting()
{
    for (Uniform& u : uniforms_)
        delete u.p_texSet;
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
        u.ownTexture = 0;


        u.p_type = params->createSelectParameter(
            ("uniformtype%1_" + id_suffix).arg(i),
            tr("uniform%1 type").arg(i + 1),
            tr("The type of the uniform variable"),
            { "none", "float", "vec2", "vec3", "vec4",
              "transform",
              "texture", "texturecube", "texture3D", "texture1D", "texture1D2", "texture1D3", "texture1D4" },
            { tr("none"), "float", "vec2", "vec3", "vec4",
              tr("mat4"),
              tr("texture"), tr("texture cube"), tr("texture 3D"),
              tr("float texture1D"), tr("vec2 texture1D"), tr("vec3 texture1D"),
              tr("vec4 texture1D") },
            { tr("none"), "float", "vec2", "vec3", "vec4",
              tr("mat4 transformation"),
              tr("texture"), tr("texture cube"), tr("texture 3D"),
              tr("float texture1D"), tr("vec2 texture1D"), tr("vec3 texture1D"),
              tr("vec4 texture1D") },
            { UT_NONE, UT_F1, UT_F2, UT_F3, UT_F4,
              UT_TRANS,
              UT_TEX, UT_TEX_C, UT_TEX_3D, UT_T1, UT_T2, UT_T3, UT_T4 },
            UT_NONE,
            true, false);
        u.p_type->setDefaultEvolvable(false);

        u.p_name = params->createTextParameter(
                    ("uniformname%1_" + id_suffix).arg(i),
               tr("uniform%1 name").arg(i + 1),
               tr("The name of the uniform variable as it should "
                  "go into the header of the shader"),
               TT_PLAIN_TEXT,
               QString("uu_%1").arg(i),
               true, false);
        u.p_name->setDefaultEvolvable(false);

        u.p_length = params->createIntParameter(
                    ("uniformtexlen%1_" + id_suffix).arg(i),
                    tr("uniform%1 length").arg(i + 1),
                    tr("The length of the array / texture width"),
                    1024, 16, 16384,
                    16, true, false);
        u.p_length->setDefaultEvolvable(false);

        u.p_timerange = params->createFloatParameter(
                ("uniformtextime%1_" + id_suffix).arg(i),
                tr("uniform%1 time range").arg(i + 1),
                tr("The time range in seconds to fill the texture"),
                1., -10000., 10000.,
                .1, true, true);

        u.p_texSet = new TextureSetting(object_);
        u.p_texSet->createParameters(("uniformtex%1_" + id_suffix).arg(i),
                                     tr("uniform%1 texture").arg(i + 1),
                                     ParameterTexture::IT_INPUT);
        u.p_texSet->textureParam()->setVisibleGraph(true);

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

        u.p_trans = params->createTransformationParameter(
                    ("uniform_transform%1_" + id_suffix).arg(i),
                    tr("uniform%1 mat4").arg(i+1),
                    tr("A 4-by-4 transformation matrix"));

        uniforms_.push_back(u);
    }
}

bool UserUniformSetting::needsReinit(Parameter *p) const
{
    bool r = false;
    for (const Uniform & u : uniforms_)
        r |= u.p_texSet->onParameterChange(p);

    for (const Uniform & u : uniforms_)
    {
        if (u.p_name == p || u.p_type == p || u.p_length == p)
            return true;
    }

    return r;
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
        if (type == UT_TRANS)
            u.num_floats = 16;

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
            u.p_float[i]->setVisible(i < u.num_floats
                                     && !u.isTransformation());

        u.p_name->setVisible(u.num_floats > 0 || u.isTextureInput());

        u.p_texSet->setVisible(u.isTextureInput());
        u.p_trans->setVisible(u.isTransformation());
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
            case UT_TRANS: typestr = "mat4"; break;
            case UT_T1:
            case UT_T2:
            case UT_T3:
            case UT_T4: typestr = "sampler1D"; break;
            case UT_TEX: typestr = "sampler2D"; break;
            case UT_TEX_C: typestr = "samplerCube"; break;
            case UT_TEX_3D: typestr = "sampler3D"; break;
            case UT_NONE: break;
        }

        decl += "uniform " + typestr + " " + u.p_name->value() + ";\n";
    }

    return decl;
}

void UserUniformSetting::tieToShader(GL::Shader * shader)
{
    for (Uniform & u : uniforms_)
    if (u.isUsed() && !u.p_name->value().isEmpty())
    {
        u.uniform = shader->getUniform(u.p_name->value(), false);
        if (!u.uniform)
            continue;

        if (!u.isBufferTexture())
        {
            // check if type is compatible
            if (u.uniform && !(
                        u.uniform->type() == gl::GL_FLOAT
                     || u.uniform->type() == gl::GL_FLOAT_VEC2
                     || u.uniform->type() == gl::GL_FLOAT_VEC3
                     || u.uniform->type() == gl::GL_FLOAT_VEC4
                     || u.uniform->type() == gl::GL_SAMPLER_2D
                     || u.uniform->type() == gl::GL_SAMPLER_3D
                     || u.uniform->type() == gl::GL_SAMPLER_CUBE
                     || u.uniform->type() == gl::GL_FLOAT_MAT4
                        ))
                u.uniform = 0;
        }
        // create a texture
        else
        {
            // update texture
            if (!u.ownTexture)
            {
                u.ownTexture = new GL::Texture();
                u.ownTexture->setName("float_input");
            }
            if ((int)u.ownTexture->width() != u.p_length->baseValue())
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
                u.ownTexture->create(u.p_length->baseValue(),
                                     gl::GL_RGBA32F, informat, gl::GL_FLOAT, 0);
            }
        }
    }

    uploadTime_ = -1.12341212;
}

void UserUniformSetting::updateUniforms(const RenderTime& time, uint* texSlot)
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
            u.uniform->ints[0] = *texSlot;
            u.p_texSet->bind(time, texSlot);
        }
        else if (u.isTransformation())
        {
            u.uniform->set( u.p_trans->value(time) );
        }
        else
        if (!u.isBufferTexture())
        {
            // copy single float value
            for (uint i=0; i<u.num_floats; ++i)
                u.uniform->floats[i] = u.p_float[i]->value(time);
        }
        else
        {
            uint len = u.ownTexture->width(),
                 bsize = len * u.num_floats;
            // update buffer size
            if (u.texBuf.size() != bsize)
                u.texBuf.resize(bsize);

            // sample inputs
            const Double rangeStep = u.p_timerange->value(time)
                                     / std::max(1, int(len)-1);
            RenderTime ti(time);
            for (uint j=0; j<len; ++j)
            {
                for (uint i=0; i<u.num_floats; ++i)
                    u.texBuf[j * u.num_floats + i] = u.p_float[i]->value(ti);
                ti -= rangeStep;
            }

            GL::Texture::setActiveTexture(*texSlot);
            u.ownTexture->bind();
            u.uniform->ints[0] = *texSlot;
            //MO_PRINT("upload " << u.uniform->name() << " " << *texSlot);
            ++(*texSlot);

            u.ownTexture->upload(&u.texBuf[0]);
        }
    }
}


void UserUniformSetting::releaseGl()
{
    for (Uniform & u : uniforms_)
    {
        if (u.ownTexture)
        {
            if (u.ownTexture->isAllocated())
                u.ownTexture->release();
            delete u.ownTexture;
            u.ownTexture = 0;
        }

        u.p_texSet->releaseGl();
    }
}

} // namespace MO


