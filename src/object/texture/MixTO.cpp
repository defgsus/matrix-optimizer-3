/** @file mixto.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.05.2015</p>
*/

#include "MixTO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterTexture.h"
#include "gl/ScreenQuad.h"
#include "gl/Shader.h"
#include "gl/ShaderSource.h"
#include "math/functions.h"
#include "io/DataStream.h"
#include "io/log.h"

#undef CF_MAX // windows..

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
    void renderGl(const GL::RenderSettings&rset, const RenderTime& time);
    QString getModSyntax(ModFunc);

    MixTO * to;

    ParameterSelect
            * p_func,
            * p_mod1,
            * p_mod2,
            * p_keep_alpha;
};


MixTO::MixTO()
    : TextureObjectBase ()
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

void MixTO::renderGl(const GL::RenderSettings& rset, const RenderTime& time)
{
    p_->renderGl(rset, time);
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

        p_mod1 = to->params()->createSelectParameter("mod1", tr("first argument"),
        tr("Modifies the first argument to the combination function"),
        { "asis", "invert", "invertall" },
        { tr("as is"), tr("invert colors"), tr("invert all") },
        { tr("No change"), tr("Invert the color channels"), tr("Invert all channels including alpha") },
        { MF_NONE, MF_INVERT, MF_INVERT_ALL },
        MF_NONE,
        true, false);

        p_mod2 = to->params()->createSelectParameter("mod2", tr("second argument"),
        tr("Modifies the second argument to the combination function"),
        { "asis", "invert", "invertall" },
        { tr("as is"), tr("invert colors"), tr("invert all") },
        { tr("No change"), tr("Invert the color channels"), tr("Invert all channels including alpha") },
        { MF_NONE, MF_INVERT, MF_INVERT_ALL },
        MF_NONE,
        true, false);

        p_keep_alpha = to->params()->createBooleanParameter("keep_alpha", tr("keep alpha"),
        tr("When selected, the alpha channel of the first input will stay unchanged"),
        { tr("The alpha channel is combined just like the other channels") },
        { tr("Alpha stays unchanged") },
        true,
        true, false);

    to->params()->endParameterGroup();
}

void MixTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_func
        || p == p_->p_mod1
        || p == p_->p_mod2
        || p == p_->p_keep_alpha)
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


QString MixTO::Private::getModSyntax(ModFunc t)
{
    QString source;
    switch (t)
    {
        case MF_NONE: break;
        case MF_INVERT: source = "X.rgb = 1. - X.rgb;"; break;
        case MF_INVERT_ALL: source = "X = 1. - X;"; break;
    }
    return source;
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

        // phase mod functions
        QString source =  getModSyntax( (ModFunc)p_mod1->baseValue() ),
                source2 = getModSyntax( (ModFunc)p_mod2->baseValue() );;
        source.replace("X", "a");
        source2.replace("X", "b");
        src.replace("//%mod_func%", "\t" + source + "\n\t" + source2, true);

        // paste combine function
        switch ((CombineFunc)p_func->baseValue())
        {
            case CF_ADD: source = "c = a + b;"; break;
            case CF_SUB: source = "c = a - b;"; break;
            case CF_MUL: source = "c = a * b;"; break;
            case CF_DIV: source = "c = a / b;"; break;
            case CF_MIN: source = "c = min(a, b);"; break;
            case CF_MAX: source = "c = max(a, b);"; break;
        }
        src.replace("//%combine_func%", "\t" + source, true);

        if (p_keep_alpha->baseValue())
            src.replace("//%restore_func%", "\tc.a = pa.a;\n");

        // create calls for each additional texture input
        source.clear();
        for (uint i=1; i<to->numberTextureInputs(); ++i)
            if (to->hasTextureInput(i))
                source += QString("\tPROC_LAYER(%1);\n").arg(i);
        src.replace("//%call_func%", source);

        //MO_PRINT(src.fragmentSource());
    }
    catch (Exception& )
    {
        // XXX Signal to gui
        throw;
    }

    QStringList texNames;
    for (uint i=0; i<to->numberTextureInputs(); ++i)
        texNames << QString("u_tex_%1").arg(i);

    //auto shader =
            to->createShaderQuad(src, texNames)->shader();

    // uniforms


}

void MixTO::Private::releaseGl()
{

}

void MixTO::Private::renderGl(const GL::RenderSettings& , const RenderTime& time)
{
    // update uniforms

    to->renderShaderQuad(time);
}


} // namespace MO
