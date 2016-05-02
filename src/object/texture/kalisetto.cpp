/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/9/2015</p>
*/

#include <QTextStream>

#include "kalisetto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/util/objecteditor.h"
#include "object/util/useruniformsetting.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "math/kalisetevolution.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(KaliSetTO)

struct KaliSetTO::Private
{
    Private(KaliSetTO * to)
        : to            (to)
        , evo           (0)
        , p_uniforms    (new UserUniformSetting(to))
    { }

    ~Private()
    {
        delete p_uniforms;
    }

    void createParameters();
    QString createEvoShader(KaliSetEvolution*) const;
    GL::ShaderSource createSource();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    KaliSetTO * to;

    KaliSetEvolution * evo;

    ParameterFloat
            *p_paramX, *p_paramY, *p_paramZ, *p_paramW,
            *p_offsetX, *p_offsetY, *p_offsetZ, *p_offsetW,
            *p_scale, *p_scaleX, *p_scaleY, *p_scaleZ,
            *p_bright, *p_exponent, *p_freq,
            *p_fisheye_ang;
    ParameterInt
            *p_numIter, *p_AntiAlias_;
    ParameterSelect
            *p_numDim, *p_calcMode, *p_posMode, *p_colMode,
            *p_rgbMode, *p_outMode, *p_mono;
    ParameterText
            *p_param_glsl;
    UserUniformSetting* p_uniforms;
    GL::Uniform
            * u_kali_param,
            * u_offset,
            * u_scale,
            * u_bright,
            * u_freq,
            * u_fisheye_ang;
};


KaliSetTO::KaliSetTO()
    : TextureObjectBase ()
    , p_                (new Private(this))
{
    setName("KaliSet");
    initMaximumTextureInputs(0);

    addEvolutionKey(tr("Kali-Set"));
}

KaliSetTO::~KaliSetTO()
{
    if (p_->evo)
        p_->evo->releaseRef("KaliSetTO destroy");
    delete p_;
}

void KaliSetTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texkali", 3);

    // v2
    if (p_->evo)
        io << true << p_->evo->toJsonString();
    else
        io << false;
    // v3
    p_->p_uniforms->serialize(io);
}

void KaliSetTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    const int ver = io.readHeader("texkali", 3);

    if (ver >= 2)
    {
        KaliSetEvolution* evo = 0;
        bool isEvo;
        io >> isEvo;
        if (isEvo)
        {
            QString s;
            io >> s;
            evo = dynamic_cast<KaliSetEvolution*>( EvolutionBase::fromJsonString(s) );
        }
        if (p_->evo)
            p_->evo->releaseRef("KaliSetTO deserialize relprev");
        p_->evo = evo;
    }

    if (ver >= 3)
        p_->p_uniforms->deserialize(io);

}

void KaliSetTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void KaliSetTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void KaliSetTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void KaliSetTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
}


void KaliSetTO::Private::createParameters()
{
    to->params()->beginParameterGroup("kali", tr("kali set"));
    to->initParameterGroupExpanded("kali");

        p_calcMode = to->params()->createSelectParameter(
                    "calc_mode", tr("mode"),
                    tr("The calculation mode"),
        { "basic", "user_func", "evolved" },
        { tr("basic"), tr("user function"), tr("evolved") },
        { tr("Basic flexible kali renderer"),
          tr("Same as basic but with a user function defining the kaliset parameter"),
          tr("Renderer based on interactive evolution") },
        { 0, 1, 2 },
                    0, true, false);
        p_calcMode->setDefaultEvolvable(false);

        p_param_glsl = to->params()->createTextParameter(
                    "param_func", tr("parameter function"),
                    tr("A freely definable glsl function that returns the "
                       "magic kali parameter"),
                    TT_GLSL,
        "// uv is [-1,1]\n"
        "// cur is value of current iteration\n"
        "// i is current iteration number\n"
        "vec4 kali_user_param(in vec2 uv, in VEC cur, in int i)\n"
        "{\n"
        "\treturn u_kali_param;\n"
        "}\n", true, false);
        p_param_glsl->setDefaultEvolvable(false);

        p_posMode = to->params()->createSelectParameter(
                    "pos_mode", tr("slice"),
                    tr("The shape of the slice through kali-set space"),
                    { "planar", "fisheye", "cylinder_y", "cylinder_x" },
                    { tr("planar"), tr("fisheye"), tr("cylinder Y"), tr("cylinder X") },
                    { tr("Planar slice"),
                      tr("Spherical slice (only visible with >2 dimensions)"),
                      tr("Cylindrical slice (only visible with >2 dimensions)"),
                      tr("Cylindrical slice (only visible with >2 dimensions)") },
                    { 0, 1, 2, 3 },
                    0, true, false);
        p_posMode->setDefaultEvolvable(false);

        p_fisheye_ang = to->params()->createFloatParameter(
                    "slice_angle", tr("slice angle"),
                    tr("The angle of the fisheye or cylinder in degree"), 180., 1.);

        p_numDim = to->params()->createSelectParameter(
                    "num_dim", tr("dimensions"),
                    tr("The number of dimensions to use in the formula"),
        { "2", "3", "4" },
        { tr("two"), tr("three"), tr("four") },
        { tr("Two-dimensional"), tr("Three-dimensional"), tr("Four-dimensional") },
        { 2, 3, 4 },
                    2, true, false);

        p_numIter = to->params()->createIntParameter(
                    "num_iter", tr("iterations"),
                    tr("The number of iterations on the kali set formula"),
                    15, true, false);
        p_numIter->setMinValue(1);

        p_paramX = to->params()->createFloatParameter(
                    "param_x", tr("parameter x"),
                    tr("The magic kali parameter, typical range is [-0.1, 1.5]"), 0.5, 0.01);

        p_paramY = to->params()->createFloatParameter(
                    "param_y", tr("parameter y"),
                    tr("The magic kali parameter, typical range is [-0.1, 1.5]"), 0.5, 0.01);

        p_paramZ = to->params()->createFloatParameter(
                    "param_z", tr("parameter z"),
                    tr("The magic kali parameter, typical range is [-0.1, 1.5]"), 0.5, 0.01);

        p_paramW = to->params()->createFloatParameter(
                    "param_w", tr("parameter w"),
                    tr("The magic kali parameter, typical range is [-0.1, 1.5]"), 0.5, 0.01);


        p_offsetX = to->params()->createFloatParameter(
                    "offset_x", tr("offset x"),
                    tr("Offset/position on x-axis"), 0., 0.01);

        p_offsetY = to->params()->createFloatParameter(
                    "offset_y", tr("offset y"),
                    tr("Offset/position on y-axis"), 0., 0.01);

        p_offsetZ = to->params()->createFloatParameter(
                    "offset_z", tr("offset z"),
                    tr("Offset/position on z-axis"), 0., 0.01);

        p_offsetW = to->params()->createFloatParameter(
                    "offset_w", tr("offset w"),
                    tr("Offset/position on w-axis"), 0., 0.01);


        p_scale = to->params()->createFloatParameter(
                    "scale", tr("scale"),
                    tr("Overall scale"), 1., 0.01);

        p_scaleX = to->params()->createFloatParameter(
                    "scale_x", tr("scale x"),
                    tr("Scale on x-axis"), 1., 0.01);

        p_scaleY = to->params()->createFloatParameter(
                    "scale_y", tr("scale y"),
                    tr("Scale on y-axis"), 1., 0.01);

        p_scaleZ = to->params()->createFloatParameter(
                    "scale_z", tr("scale z"),
                    tr("Scale on z-axis"), 1., 0.01);

    to->params()->endParameterGroup();


    to->params()->beginParameterGroup("color", tr("color"));

        p_AntiAlias_ = to->params()->createIntParameter(
                    "aa", tr("antialiasing"),
                    tr("Square root of the number of multi-samples"),
                    0, true, false);
        p_AntiAlias_->setMinValue(0);
        p_AntiAlias_->setDefaultEvolvable(false);

        p_colMode = to->params()->createSelectParameter(
            "color_mode", tr("color mode"),
            tr("Defines the kind of value to use for the output color"),
            { "final", "average", "max", "min" },
            { tr("final"), tr("average"), tr("max"), tr("min") },
            { tr("The value at the last iteration step"),
              tr("The average of all values"),
              tr("The maximum of all values"),
              tr("The minimum of all values") },
            { 0, 1, 2, 3 },
            0, true, false);

        p_rgbMode = to->params()->createSelectParameter(
            "rgb_mode", tr("rgb values"),
            tr("How the kaliset values are intepreted to rgb"),
            { "rgb", "hsv" },
            { tr("as-is"), tr("hsv") },
            { tr("The values are interpreted as RGB"),
              tr("The values are interpreted as HSV") },
            { 0, 1 },
            0, true, false);

        p_mono = to->params()->createBooleanParameter("mono", tr("monochrome"),
                                tr("Strictly monochrome colors"),
                                tr("Off"),
                                tr("On"),
                                false, true, false);

        p_bright = to->params()->createFloatParameter(
                    "brightness", tr("brightness"),
                    tr("Output brightness"), 1., 0.05);

        p_exponent = to->params()->createFloatParameter(
                    "exponent", tr("exponent"),
                    tr("Output exponent"), 1., 0.05);
        p_exponent->setMinValue(0.0);

        p_outMode = to->params()->createSelectParameter(
            "output_mode", tr("output"),
            tr("The function applied to the value"),
            { "linear", "sine", },
            { tr("linear"), tr("sine") },
            { tr("Unchanged linear"),
              tr("Sine transform of value") },
            { 0, 1 },
            0, true, false);

        p_freq = to->params()->createFloatParameter(
                    "freq", tr("frequency"),
                    tr("Frequency of the sine transform"), 10., 0.1);

    to->params()->endParameterGroup();

    to->params()->beginParameterGroup("useruniforms", tr("user uniforms"));

        p_uniforms->createParameters("g");

    to->params()->endParameterGroup();

}

void KaliSetTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_calcMode
        || p == p_->p_colMode
        || p == p_->p_posMode
        || p == p_->p_rgbMode
        || p == p_->p_numIter
        || p == p_->p_numDim
        || p == p_->p_outMode
        || p == p_->p_mono
        || p == p_->p_AntiAlias_
        || p == p_->p_param_glsl
        || p_->p_uniforms->needsReinit(p))
        requestReinitGl();
}

void KaliSetTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void KaliSetTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    bool evo = p_->p_calcMode->baseValue() == 2,
         userfunc = p_->p_calcMode->baseValue() == 1;

    int dim = evo ? 3 : p_->p_numDim->baseValue();
    int posMode = p_->p_posMode->baseValue();

    p_->p_paramZ->setVisible(dim >= 3);
    p_->p_paramW->setVisible(dim >= 4);
    p_->p_offsetZ->setVisible(dim >= 3);
    p_->p_offsetW->setVisible(dim >= 4);
    p_->p_scaleZ->setVisible(dim >= 3 && posMode != 0);

    p_->p_freq->setVisible( p_->p_outMode->baseValue() == 1);

    p_->p_numDim->setVisible(!evo);
    p_->p_numIter->setVisible(!evo);
    p_->p_colMode->setVisible(!evo);

    p_->p_param_glsl->setVisible(userfunc);

    //p_->p_posMode->setVisible(!evo);
    p_->p_fisheye_ang->setVisible(posMode != 1);

    p_->p_uniforms->updateParameterVisibility();
}


QString KaliSetTO::Private::createEvoShader(KaliSetEvolution* evo) const
{
    QString str;
    QTextStream s(&str);

    s <<"/* uv is in [-1, 1] */\n"
        "vec3 evolvedKaliSet(in vec2 uv)\n"
        "{\n"
        "\tconst vec3 colAcc = vec3("
            << evo->pColAccX() << ", " << evo->pColAccY() << ", " << evo->pColAccZ() << ");\n"
        "\tconst vec3 minAcc = vec3("
            << evo->pMinAccX() << ", " << evo->pMinAccY() << ", " << evo->pMinAccZ() << ");\n"
        "\tconst vec3 minCol = vec3("
            << evo->pMinAmtX() << ", " << evo->pMinAmtY() << ", " << evo->pMinAmtZ() << ");\n"
        "\tconst float minExp = " << evo->pMinExp() << ";\n"
        "\n\t// start pos + random scale and offset\n"
        "\tvec3 po = kali_slice_pos(uv) * u_scale + u_offset.xyz;\n"
        "\n\tvec3 col = vec3(0.);\n"
        "\tfloat md = 1000.;\n"
        "\tconst int numIter = " << evo->pNumIter() << ";\n"
        "\n\tfor (int i=0; i<numIter; ++i)\n"
        "\t{\n"
        "\t\t// kali set (first half)\n"
        "\t\tpo = abs(po.xyz) / dot(po, po);\n"
        "\n"
        "\t\t// accumulate some values\n"
        "\t\tcol += colAcc * po;\n"
        "\t\tmd = min(md, abs(dot(minAcc, po)));\n"
        "\n"
        "\t\t// kali set (second half)\n"
        "\t\tpo -= u_kali_param.xyz;\n";
    for (int i=0; i<evo->pNumIter()-1; ++i)
    {
        s << "\t\tif (i == " << i << ") po -= vec3("
          << evo->pMagicX(i) << ", " << evo->pMagicY(i) << ", " << evo->pMagicZ(i) << ");\n";
    }
    s <<"\t}\n\t// average color\n"
        "\tcol = abs(col) / float(numIter);\n"
        "\n\t// 'min-distance stripes' or 'orbit traps'\n"
        "\n"
        "\t// mix-in color from last iteration step\n"
        "\tcol += po * vec3("
             << evo->pAmtX() << ", " << evo->pAmtY() << ", " << evo->pAmtZ() << ");\n"
        "\n"
        "\tmd = pow(1. - md, minExp);\n"
        "\tcol += md * minCol;\n"
        "\n\treturn col;\n"
        "}\n";
    return str;
}

GL::ShaderSource KaliSetTO::Private::createSource()
{
    GL::ShaderSource src;

    src.loadVertexSource(":/shader/to/default.vert");
    src.loadFragmentSource(":/shader/to/kaliset.frag");
    src.pasteDefaultIncludes();

    src.addDefine(QString("#define CALC_MODE %1").arg(p_calcMode->baseValue()), false);
    src.addDefine(QString("#define POS_MODE %1").arg(p_posMode->baseValue()), false);
    src.addDefine(QString("#define RGB_MODE %1").arg(p_rgbMode->baseValue()), false);
    src.addDefine(QString("#define NUM_DIM %1").arg(p_numDim->baseValue()), false);
    src.addDefine(QString("#define SINE_OUT %1").arg(p_outMode->baseValue()), false);
    src.addDefine(QString("#define MONOCHROME %1").arg(p_mono->baseValue()), false);
    src.addDefine(QString("#define AA %1").arg(p_AntiAlias_->baseValue()), false);
    if (p_calcMode->baseValue() != 2)
    {
        src.addDefine(QString("#define NUM_ITER %1").arg(p_numIter->baseValue()), false);
        src.addDefine(QString("#define COL_MODE %1").arg(p_colMode->baseValue()), false);
        src.replace("//%mo_user_uniforms%", p_uniforms->getDeclarations());
        if (p_calcMode->baseValue() == 1)
            src.replace("//%kali_user_param%", "#line 1\n" + p_param_glsl->baseValue(), true);
    }
    else
    {
        if (!evo)
        {
            evo = new KaliSetEvolution();
            evo->randomize();
        }
        src.replace("//%KaliSet%", createEvoShader(evo));
    }

    return src;
}

void KaliSetTO::Private::initGl()
{
    // shader-quad

    GL::Shader* shader;
    GL::ShaderSource src = createSource();

    shader = to->createShaderQuad(
                src, { "u_tex" })->shader();

    // send errors to editor widget
    for (const GL::Shader::CompileMessage& msg : to->compileMessages())
    {
        if (msg.program == GL::Shader::P_FRAGMENT
            || msg.program == GL::Shader::P_LINKER)
        {
            p_param_glsl->addErrorMessage(msg.line, msg.text);
        }
    }


    // uniforms

    u_kali_param = shader->getUniform("u_kali_param", false);
    u_offset = shader->getUniform("u_offset", false);
    u_scale = shader->getUniform("u_scale", false);
    u_bright = shader->getUniform("u_bright", false);
    u_freq = shader->getUniform("u_freq", false);
    u_fisheye_ang = shader->getUniform("u_fisheye_ang", false);

    p_uniforms->tieToShader(shader);
}

void KaliSetTO::Private::releaseGl()
{
    p_uniforms->releaseGl();
}

GL::ShaderSource KaliSetTO::valueShaderSource(uint channel) const
{
    GL::ShaderSource src;
    if (channel == 0)
        src = p_->createSource();
    return src;
}


void KaliSetTO::Private::renderGl(const GL::RenderSettings& , const RenderTime& time)
{
    // update uniforms

    if (u_kali_param)
    {
        u_kali_param->setFloats(
                    p_paramX->value(time),
                    p_paramY->value(time),
                    p_paramZ->value(time),
                    p_paramW->value(time));
    }

    if (u_offset)
    {
        u_offset->setFloats(
                    p_offsetX->value(time),
                    p_offsetY->value(time),
                    p_offsetZ->value(time),
                    p_offsetW->value(time));
    }

    if (u_scale)
    {
        Float s = p_scale->value(time);
        u_scale->setFloats(
                    p_scaleX->value(time) * s,
                    p_scaleY->value(time) * s,
                    p_scaleZ->value(time) * s);
    }

    if (u_bright)
    {
        u_bright->setFloats(
                    p_bright->value(time),
                    p_exponent->value(time));
    }

    if (u_freq)
    {
        u_freq->floats[0] = p_freq->value(time);
    }

    if (u_fisheye_ang)
        u_fisheye_ang->floats[0] = p_fisheye_ang->value(time);

    uint texSlot=0;
    p_uniforms->updateUniforms(time, &texSlot);

    to->renderShaderQuad(time);
}


const EvolutionBase* KaliSetTO::getEvolution(const QString& key) const
{
    if (key != tr("Kali-Set") || !p_->evo)
        return Object::getEvolution(key);
    p_->evo->setPScale(p_->p_scale->baseValue());
    p_->evo->setPPosX(p_->p_offsetX->baseValue());
    p_->evo->setPPosY(p_->p_offsetY->baseValue());
    p_->evo->setPPosZ(p_->p_offsetZ->baseValue());
    return p_->evo;
}

void KaliSetTO::setEvolution(const QString& key, const EvolutionBase* evo)
{
    if (key != tr("Kali-Set"))
    {
        Object::setEvolution(key, evo);
        return;
    }

    if (p_->evo)
        p_->evo->releaseRef("KaliSetTO setEvolution relprev");
    auto k = dynamic_cast<const KaliSetEvolution*>(evo);
    p_->evo = k ? k->createClone() : nullptr;

    if (p_->p_calcMode->baseValue() == 2 && k)
    {
        p_->p_paramZ->setValue(0.);
        p_->p_offsetX->setValue(k->pPosX());
        p_->p_offsetY->setValue(k->pPosY());
        p_->p_offsetZ->setValue(k->pPosZ());
        p_->p_scale->setValue(k->pScale());
        p_->p_scaleX->setValue(1.);
        p_->p_scaleY->setValue(1.);

        if (editor())
            emit editor()->parametersChanged(this);

        requestReinitGl();
    }
}


} // namespace MO
