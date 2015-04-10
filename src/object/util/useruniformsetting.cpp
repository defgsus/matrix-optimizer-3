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

UserUniformSetting::UserUniformSetting(Object * object, uint maxUnis)
    : QObject   (object),
      object_   (object),
      num_      (maxUnis)
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


        u.p_name = params->createTextParameter(("uniformname%1_" + id_suffix).arg(i),
                                               tr("uniform%1 name").arg(i + 1),
                                               tr("The name of the uniform variable as it should go into the header of the shader"),
                                               TT_PLAIN_TEXT,
                                               "",
                                               true, false);

        u.p_type = params->createSelectParameter(
                                                ("uniformtype%1_" + id_suffix).arg(i),
                                                tr("uniform%1 type").arg(i + 1),
                                                tr("The type of the uniform variable"),
                                                { "none", "float", "vec2", "vec3", "vec4", "texture1D" },
                                                { tr("none"), "float", "vec2", "vec3", "vec4", "texture1D" },
                                                { tr("none"), "float", "vec2", "vec3", "vec4", "texture1D" },
                                                { 0, int(gl::GL_FLOAT), int(gl::GL_FLOAT_VEC2), int(gl::GL_FLOAT_VEC3), int(gl::GL_FLOAT_VEC4),
                                                    int(gl::GL_TEXTURE_1D) },
                                                0,
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
        if (type == gl::GL_FLOAT)
            u.num_floats = 1;
        if (type == gl::GL_FLOAT_VEC2)
            u.num_floats = 2;
        if (type == gl::GL_FLOAT_VEC3)
            u.num_floats = 3;
        if (type == gl::GL_FLOAT_VEC4)
            u.num_floats = 4;

        if (type == gl::GL_TEXTURE_1D)
        {
            u.p_length->setVisible(true);
            u.p_timerange->setVisible(true);
            u.num_floats = 1;
        }
        else
        {
            u.p_length->setVisible(false);
            u.p_timerange->setVisible(false);
        }

        for (uint i = 0; i<4; ++i)
            u.p_float[i]->setVisible(i < u.num_floats);

        u.p_name->setVisible(u.num_floats > 0);
    }
}

QString UserUniformSetting::getDeclarations() const
{
    QString decl;
    /*
    layout(std140) uniform MyArray
     {
      float myDataArray[size];
     };*/
    for (const Uniform & u : uniforms_)
    if (u.isUsed() && !u.p_name->value().isEmpty())
    {
        QString typestr;
        switch (u.p_type->baseValue())
        {
            case int(gl::GL_FLOAT): typestr = "float"; break;
            case int(gl::GL_FLOAT_VEC2): typestr = "vec2"; break;
            case int(gl::GL_FLOAT_VEC3): typestr = "vec3"; break;
            case int(gl::GL_FLOAT_VEC4): typestr = "vec4"; break;
            case int(gl::GL_TEXTURE_1D): typestr = "sampler1D"; break;
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
        if (u.p_type->baseValue() != gl::GL_TEXTURE_1D)
        {
            u.uniform = s->getUniform(u.p_name->value(), false);

            // check if type is compatible
            if (u.uniform && !(
                        u.uniform->type() == gl::GL_FLOAT
                     || u.uniform->type() == gl::GL_FLOAT_VEC2
                     || u.uniform->type() == gl::GL_FLOAT_VEC3
                     || u.uniform->type() == gl::GL_FLOAT_VEC4))
                u.uniform = 0;
        }
        // create a texture
        else
        {
            // to set texture slot
            u.uniform = s->getUniform(u.p_name->value(), false);
            if (!u.texture)
                u.texture = new GL::Texture();
            if ((int)u.texture->width() != u.p_length->baseValue())
            {
                //u.texture->create(u.p_length->baseValue(), gl::GL_RED, gl::GL_RED, gl::GL_FLOAT, 0);
                u.texture->create(u.p_length->baseValue(), gl::GL_RGB32F, gl::GL_RED, gl::GL_FLOAT, 0);
            }
        }
    }
}

void UserUniformSetting::updateUniforms(Double time, uint thread)
{
    for (Uniform & u : uniforms_)
    if (u.uniform)
    {
        if (!u.texture)
        {
            for (uint i=0; i<u.num_floats; ++i)
                u.uniform->floats[i] = u.p_float[i]->value(time, thread);
        }
        else
        {
            //if (!u.texture->isCreated())
            //    u.texture->create();
            uint len = u.texture->width();
            gl::GLfloat data[len];
            const Double range = u.p_timerange->value(time, thread) / std::max(1, int(len)-1);
            for (uint j=0; j<len; ++j)
            {
                const Double ti = time - range * j;
                for (uint i=0; i<u.num_floats; ++i)
                    data[j] = u.p_float[i]->value(ti, thread);
            }
            /** @todo Need to incorporate texture slots!! */
            u.texture->bind();
            u.texture->upload(data);
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
