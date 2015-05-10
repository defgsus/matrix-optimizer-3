/** @file mixto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.05.2015</p>
*/

#include "mixto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertexture.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "math/functions.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(MixTO)

struct MixTO::Private
{
    Private(MixTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

    MixTO * to;

    ParameterSelect
            * p_func;
};


MixTO::MixTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("Mixer");
    initMaximumTextureInputs(8);
}

MixTO::~MixTO()
{
    delete p_;
}

void MixTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("tomix", 1);
}

void MixTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("tomix", 1);
}

void MixTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void MixTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void MixTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void MixTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
}


void MixTO::Private::createParameters()
{
    to->params()->beginParameterGroup("key", tr("key"));
    to->initParameterGroupExpanded("key");

        p_func = to->params()->createSelectParameter("mode", tr("combination"),
                                    tr("The type of function used to combine the channels"),
        { "add", "sub", "mul", "div", "min", "max" },
        { tr("add"), tr("subtract"), tr("multiply"), tr("divide"), tr("minimum"), tr("maximum") },
        { tr("The channels are added"), tr("The channels are subtracted"),
          tr("The channels are multiplied"), tr("The channels are divided"),
          tr("The minimum in all channels is used"), tr("The maximum in all channels is used") },
        { CF_ADD, CF_SUB, CF_MUL, CF_DIV, CF_MIN, CF_MAX },
        CF_ADD,
        true, false);

    to->params()->endParameterGroup();
}

void MixTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_func)
        requestReinitGl();

    for (auto par : textureParams())
        if (p == par) { requestReinitGl(); break; }

}

void MixTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void MixTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();
}




void MixTO::Private::initGl()
{
    // shader-quad

    GL::ShaderSource src;
    try
    {
        src.loadVertexSource(":/shader/to/default.vert");
        src.loadFragmentSource(":/shader/to/mix.frag");
        src.pasteDefaultIncludes();
        src.addDefine(QString("#define NUM_TEX %1").arg(to->numberTextureInputs()));

        // paste combine function
        QString source;
        switch ((CombineFunc)p_func->baseValue())
        {
            case CF_ADD: source = "return a + b;"; break;
            case CF_SUB: source = "return a - b;"; break;
            case CF_MUL: source = "return a * b;"; break;
            case CF_DIV: source = "return a / b;"; break;
            case CF_MIN: source = "return min(a, b);"; break;
            case CF_MAX: source = "return max(a, b);"; break;
        }
        src.replace("//%combine_func%", "\t" + source, true);

        // create calls for each additional texture input
        source.clear();
        for (uint i=1; i<to->numberTextureInputs(); ++i)
            if (to->hasTextureInput(i))
                source += QString("\tPROC_LAYER(%1);\n").arg(i);
        src.replace("//%call_func%", source);

    }
    catch (Exception& )
    {
        // XXX Signal to gui
        throw;
    }

    QStringList texNames;
    for (uint i=0; i<to->numberTextureInputs(); ++i)
        texNames << QString("u_tex_%1").arg(i);

    auto shader = to->createShaderQuad(src, texNames)->shader();

    // uniforms


}

void MixTO::Private::releaseGl()
{

}

void MixTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    // update uniforms

    to->renderShaderQuad(time, thread);
}


} // namespace MO
