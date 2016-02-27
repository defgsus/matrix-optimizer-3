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
#include "object/util/objecteditor.h"
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
    { }

    void createParameters();
    QString createEvoShader(KaliSetEvolution*) const;
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);

    KaliSetTO * to;

    KaliSetEvolution * evo;

    ParameterFloat
            *p_paramX, *p_paramY, *p_paramZ, *p_paramW,
            *p_offsetX, *p_offsetY, *p_offsetZ, *p_offsetW,
            *p_scale, *p_scaleX, *p_scaleY,
            *p_bright, *p_exponent, *p_freq;
    ParameterInt
            *p_numIter, *p_AntiAlias_;
    ParameterSelect
            *p_numDim, *p_calcMode, *p_colMode, *p_outMode, *p_mono;
    GL::Uniform
            * u_kali_param,
            * u_offset,
            * u_scale,
            * u_bright,
            * u_freq;
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
        p_->evo->releaseRef();
    delete p_;
}

void KaliSetTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texkali", 2);

    // v2
    if (p_->evo)
        io << true << p_->evo->toJsonString();
    else
        io << false;
}

void KaliSetTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    const int ver = io.readHeader("texkali", 2);

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
            p_->evo->releaseRef();
        p_->evo = evo;
    }
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
        { "basic", "evolved" },
        { tr("basic"), tr("evolved") },
        { tr("Basic flexible kali renderer"),
          tr("Renderer based on interactive evolution") },
        { 0, 1 },
                    0, true, false);
        p_calcMode->setDefaultEvolvable(false);

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
}

void KaliSetTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_calcMode
        || p == p_->p_colMode
        || p == p_->p_numIter
        || p == p_->p_numDim
        || p == p_->p_outMode
        || p == p_->p_mono
        || p == p_->p_AntiAlias_)
        requestReinitGl();
}

void KaliSetTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void KaliSetTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    bool evo = p_->p_calcMode->baseValue() == 1;

    int dim = evo ? 3 : p_->p_numDim->baseValue();

    p_->p_paramZ->setVisible(dim >= 3);
    p_->p_paramW->setVisible(dim >= 4);
    p_->p_offsetZ->setVisible(dim >= 3);
    p_->p_offsetW->setVisible(dim >= 4);

    p_->p_freq->setVisible( p_->p_outMode->baseValue() != 0);

    p_->p_numDim->setVisible(!evo);
    p_->p_numIter->setVisible(!evo);
    p_->p_colMode->setVisible(!evo);
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
        "\tvec3 po = vec3(uv * u_scale, 0.) + u_offset.xyz;\n"
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

void KaliSetTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/kaliset.frag");
        src.pasteDefaultIncludes();

        src.addDefine(QString("#define CALC_MODE %1").arg(p_calcMode->baseValue()), false);
        src.addDefine(QString("#define NUM_DIM %1").arg(p_numDim->baseValue()), false);
        src.addDefine(QString("#define SINE_OUT %1").arg(p_outMode->baseValue()), false);
        src.addDefine(QString("#define MONOCHROME %1").arg(p_mono->baseValue()), false);
        src.addDefine(QString("#define AA %1").arg(p_AntiAlias_->baseValue()), false);
        if (p_calcMode->baseValue() != 1)
        {
            src.addDefine(QString("#define NUM_ITER %1").arg(p_numIter->baseValue()), false);
            src.addDefine(QString("#define COL_MODE %1").arg(p_colMode->baseValue()), false);
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


    }
    catch (Exception& e)
    {
        to->setErrorMessage(e.what());
        throw;
    }

    auto shader = to->createShaderQuad(
                src, { "u_tex" })->shader();

    // uniforms

    u_kali_param = shader->getUniform("u_kali_param", false);
    u_offset = shader->getUniform("u_offset", false);
    u_scale = shader->getUniform("u_scale", false);
    u_bright = shader->getUniform("u_bright", false);
    u_freq = shader->getUniform("u_freq", false);
}

void KaliSetTO::Private::releaseGl()
{

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
                    p_scaleY->value(time) * s);
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
        p_->evo->releaseRef();
    auto k = dynamic_cast<const KaliSetEvolution*>(evo);
    p_->evo = k ? k->createClone() : nullptr;

    if (p_->p_calcMode->baseValue() == 1 && k)
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
