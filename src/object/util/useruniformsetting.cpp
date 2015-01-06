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
#include "object/param/parametertext.h"
#include "object/param/parameterselect.h"
#include "gl/shader.h"
#include "gl/opengl.h"
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


        u.p_name = params->createTextParameter(("uniformname%1_" + id_suffix).arg(i),
                                               tr("uniform%1 name").arg(i + 1),
                                               tr("The name of the uniform variable as it should go into the header of the shader"),
                                               TT_PLAIN_TEXT,
                                               "",
                                               true, false);

        u.p_type = params->createSelectParameter(
                                                ("uniformtype%1_" + id_suffix).arg(i),
                                                tr("uniform%1 type").arg(i + 1),
                                                tr("The type of the uniform variable "),
                                                { "none", "float", "vec2", "vec3", "vec4" },
                                                { tr("none"), "float", "vec2", "vec3", "vec4" },
                                                { tr("none"), "float", "vec2", "vec3", "vec4" },
                                                { 0, int(gl::GL_FLOAT), int(gl::GL_FLOAT_VEC2), int(gl::GL_FLOAT_VEC3), int(gl::GL_FLOAT_VEC4) },
                                                0,
                                                true, false);

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
        if (u.p_name == p)
            return true;
    }

    return false;
}

void UserUniformSetting::updateParameterVisibility()
{
    for (Uniform & u : uniforms_)
    {
        uint type = u.p_type->baseValue();

        uint num = 0;
        if (type == gl::GL_FLOAT)
            num = 1;
        if (type == gl::GL_FLOAT_VEC2)
            num = 2;
        if (type == gl::GL_FLOAT_VEC3)
            num = 3;
        if (type == gl::GL_FLOAT_VEC4)
            num = 4;

        for (uint i = 0; i<4; ++i)
            u.p_float[i]->setVisible(i < num);
        u.p_name->setVisible(num > 0);
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
        u.uniform = s->getUniform(u.p_name->value(), false);

        // check if type is compatible
        if (u.uniform && !(
                    u.uniform->type() == gl::GL_FLOAT
                 || u.uniform->type() == gl::GL_FLOAT_VEC2
                 || u.uniform->type() == gl::GL_FLOAT_VEC3
                 || u.uniform->type() == gl::GL_FLOAT_VEC4))
            u.uniform = 0;
    }
}

void UserUniformSetting::updateUniforms(Double time, uint thread)
{
    for (Uniform & u : uniforms_)
    if (u.uniform)
    {
        for (int i=0; i<4; ++i)
            u.uniform->floats[i] = u.p_float[i]->value(time, thread);
    }
}



} // namespace MO
