/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/20/2015</p>
*/

#ifndef MO_DISABLE_EXP

#include "distancemapto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "gl/screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "io/datastream.h"
#include "io/log.h"


namespace MO {

MO_REGISTER_OBJECT(DistanceMapTO)

struct DistanceMapTO::Private
{
    Private(DistanceMapTO * to)
        : to            (to)
    { }

    void createParameters();
    void initGl();
    void releaseGl();
    void renderGl(const GL::RenderSettings&rset, uint thread, Double time);

    DistanceMapTO * to;

    ParameterFloat
            * p_r, * p_g, * p_b, * p_a,
            * p_thresh;
    ParameterInt * p_distance;
    ParameterSelect
            * p_invert;
    GL::Uniform
            * u_color,
            * u_settings;
};


DistanceMapTO::DistanceMapTO(QObject *parent)
    : TextureObjectBase (parent)
    , p_                (new Private(this))
{
    setName("DistanceMap");
    initMaximumTextureInputs(1);
}

DistanceMapTO::~DistanceMapTO()
{
    delete p_;
}

void DistanceMapTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("texdm", 1);
}

void DistanceMapTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("texdm", 1);
}

void DistanceMapTO::createParameters()
{
    TextureObjectBase::createParameters();
    p_->createParameters();
}

void DistanceMapTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
    p_->initGl();
}

void DistanceMapTO::releaseGl(uint thread)
{
    p_->releaseGl();
    TextureObjectBase::releaseGl(thread);
}

void DistanceMapTO::renderGl(const GL::RenderSettings& rset, uint thread, Double time)
{
    p_->renderGl(rset, thread, time);
}


void DistanceMapTO::Private::createParameters()
{
    to->params()->beginParameterGroup("distance_map", tr("distance map"));
    to->initParameterGroupExpanded("distance_map");

        p_thresh = to->params()->createFloatParameter(
                    "thresh", tr("threshold"),
                    tr("Threshold value above which the color is interpreted as ON [0,1]"),
                    0.5, 0.05);

        p_r = to->params()->createFloatParameter("red", tr("red"), tr("Red contribution to threshold value"), 1.0, 0.1);
        p_g = to->params()->createFloatParameter("green", tr("green"), tr("Green contribution to threshold value"), 1.0, 0.1);
        p_b = to->params()->createFloatParameter("blue", tr("blue"), tr("Blue contribution to threshold value"), 1.0, 0.1);
        p_a = to->params()->createFloatParameter("alpha", tr("alpha"), tr("Alpha contribution to threshold value"), 0.0, 0.1);

        p_invert = to->params()->createBooleanParameter("invert", tr("invert"),
                                tr("Invert the selection"),
                                tr("No inversion"),
                                tr("Invert selection"),
                                false, true, false);

        p_distance = to->params()->createIntParameter(
                    "distance", tr("max. distance"),
                    tr("Maximum distance to messure (in pixels)"), 4,  1, 1000,  1, true, false);

    to->params()->endParameterGroup();
}

void DistanceMapTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == p_->p_distance
        || p == p_->p_invert)
        requestReinitGl();

}

void DistanceMapTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void DistanceMapTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();


}




void DistanceMapTO::Private::initGl()
{
    // --- threshold shader-quad ---

    {
        GL::ShaderSource src;
        try
        {
            src.loadVertexSource(":/shader/to/default.vert");
            src.loadFragmentSource(":/shader/to/threshold.frag");
            src.pasteDefaultIncludes();
            src.addDefine(QString("#define INVERT %1")
                          .arg(p_invert->baseValue() ? "1" : "0"));
        }
        catch (const Exception& e)
        {
            to->setError(e.what());
            throw;
        }

        auto shader = to->createShaderQuad(
                    src, { "u_tex" })->shader();

        // uniforms

        u_color = shader->getUniform("u_color", false);
        u_settings = shader->getUniform("u_settings", false);
    }


    // ---- distance shader-quad ---
    {
        GL::ShaderSource src;
        try
        {
            src.loadVertexSource(":/shader/to/default.vert");
            src.loadFragmentSource(":/shader/to/distancemap.frag");
            src.pasteDefaultIncludes();
            src.addDefine(QString("#define DISTANCE %1")
                          .arg(p_distance->baseValue()));
            src.addDefine(QString("#define DISTANCE_DIAG %1")
                          .arg((int)std::ceil(p_distance->baseValue() * std::sqrt(2.) )));
        }
        catch (const Exception& e)
        {
            to->setError(e.what());
            throw;
        }

        /*auto shader =*/ to->createShaderQuad(
                    src, { "u_tex" })->shader();

        // uniforms

    }
}

void DistanceMapTO::Private::releaseGl()
{

}

void DistanceMapTO::Private::renderGl(const GL::RenderSettings& , uint thread, Double time)
{
    // update uniforms

    const Float
            r = p_r->value(time),
            g = p_g->value(time),
            b = p_b->value(time),
            a = p_a->value(time);

    if (u_color)
        u_color->setFloats( r, g, b, a );

    if (u_settings)
        u_settings->setFloats(
                            p_thresh->value(time) * (r + g + b + a)
                            );

    uint texSlot = 0;
    to->renderShaderQuad(0, time, thread, texSlot);
    to->renderShaderQuad(1, time, thread, texSlot);
}


} // namespace MO

#endif // MO_DISABLE_EXP
