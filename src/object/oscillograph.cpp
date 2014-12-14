/** @file oscillograph.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#include "oscillograph.h"
#include "io/datastream.h"
#include "gl/drawable.h"
#include "geom/geometry.h"
#include "gl/shadersource.h"
#include "gl/cameraspace.h"
#include "gl/shader.h"
#include "gl/rendersettings.h"
#include "gl/vertexarrayobject.h"
#include "gl/bufferobject.h"
#include "gl/compatibility.h"
#include "param/parameters.h"
#include "param/parameterint.h"
#include "param/parameterfloat.h"

namespace MO {

MO_REGISTER_OBJECT(Oscillograph)

Oscillograph::Oscillograph(QObject * parent)
    : ObjectGl      (parent),
      nextGeometry_ (0),
      doRecompile_  (false)
{
    setName("Oscillograph");
}

void Oscillograph::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);

    io.writeHeader("oscg", 1);

}

void Oscillograph::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);

    //const int ver =
            io.readHeader("oscg", 1);
}

void Oscillograph::createParameters()
{
    ObjectGl::createParameters();

    params()->beginParameterGroup("color", tr("color"));

        paramBright_ = params()->createFloatParameter("bright", "bright", tr("Overall brightness of the color"), 1.0, 0.1);
        paramR_ = params()->createFloatParameter("red", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        paramG_ = params()->createFloatParameter("green", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        paramB_ = params()->createFloatParameter("blue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        paramA_ = params()->createFloatParameter("alpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);

    params()->endParameterGroup();

    params()->beginParameterGroup("oscgraph", tr("oscillograph"));

        paramValue_ = params()->createFloatParameter("oscvalue", tr("value"),
                                        tr("The input value for the oscillograph/scope"),
                                        0.0,
                                        0.01, true, true);

        paramNumPoints_ = params()->createIntParameter("numverts", tr("number points"),
                                            tr("The number of points on the oscillograph"),
                                            100, 2, 999999,
                                            1, true, false);

        paramTimeSpan_ = params()->createFloatParameter("timeshift", tr("time span"),
                                            tr("The time-span of the oscillograph in seconds"),
                                            -0.1,
                                            0.01, true, true);

        paramWidth_ = params()->createFloatParameter("scalex", tr("width"),
                                            tr("The width or scale on x-axis"),
                                            1.0,
                                            0.1, true, true);

        paramLineWidth_ = params()->createFloatParameter("linewidth", tr("line width"),
                                            tr("The width of the line - currently in pixels"),
                                            2,
                                            0.01, true, true);
        paramLineWidth_->setMinValue(1);

    params()->endParameterGroup();
}

void Oscillograph::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == paramNumPoints_)
        requestReinitGl();
}

void Oscillograph::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    // nothing
}

/*
void Oscillograph::getNeededFiles(IO::FileList &files)
{

}*/

const GEOM::Geometry* Oscillograph::geometry() const
{
    return draw_ ? draw_->geometry() : 0;
}

Vec4 Oscillograph::modelColor(Double time, uint thread) const
{
    const auto b = paramBright_->value(time, thread);
    return Vec4(
        paramR_->value(time, thread) * b,
        paramG_->value(time, thread) * b,
        paramB_->value(time, thread) * b,
        paramA_->value(time, thread));
}

void Oscillograph::initGl(uint /*thread*/)
{
    draw_ = new GL::Drawable(idName());

    // create a line
    auto g = draw_->geometry();

    g->setColor(1,1,1,1);

    int num = std::max(2, paramNumPoints_->baseValue());
    for (int i=0; i<num; ++i)
    {
        g->addVertexAlways(i, 0, 0);
        if (i > 0)
            g->addLine(i - 1, i);
    }

    setupDrawable_();
}

void Oscillograph::releaseGl(uint /*thread*/)
{
    if (draw_->isReady())
        draw_->releaseOpenGl();

    delete draw_;
    draw_ = 0;
}

/*void Oscillograph::numberLightSourcesChanged(uint thread)
{
    doRecompile_ = true;
}*/

void Oscillograph::setupDrawable_()
{
    GL::ShaderSource * src = new GL::ShaderSource();

    src->loadDefaultSource();

    draw_->setShaderSource(src);

    draw_->createOpenGl();

    vaoBuffer_.resize(draw_->geometry()->numVertices() * 3);

    // get uniforms
}

void Oscillograph::calcVaoBuffer_(Double time, uint thread)
{
    const uint numPoints = vaoBuffer_.size() / 3;

    const Double
            timeSpan = paramTimeSpan_->value(time, thread),
            scalex = paramWidth_->value(time, thread),

            fac = 1.0 / (numPoints - 1);

    // calculate osci positions
    gl::GLfloat * pos = &vaoBuffer_[0];
    for (uint i = 0; i < numPoints; ++i)
    {
        const Double t = Double(i) * fac;

        *pos++ = (0.5f - t) * scalex;
        *pos++ = paramValue_->value(time + t * timeSpan, thread);
        *pos++ = 0.0f;
    }

}

void Oscillograph::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    const Mat4& trans = transformation();
    const Mat4  cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
    const Mat4  viewTrans = rs.cameraSpace().viewMatrix() * trans;

    if (doRecompile_)
    {
        doRecompile_ = false;
        setupDrawable_();
    }

    if (draw_->isReady())
    {
        // update geometry

        GL::BufferObject * buf = draw_->vao()->getBufferObject(
                    GL::VertexArrayObject::A_POSITION);
        if (buf && vaoBuffer_.size() > 3)
        {
            calcVaoBuffer_(time, thread);

            // move to device
            buf->bind();
            buf->upload(&vaoBuffer_[0]);
        }


        // update uniforms
        const auto bright = paramBright_->value(time, thread);
        draw_->setAmbientColor(
                    paramR_->value(time, thread) * bright,
                    paramG_->value(time, thread) * bright,
                    paramB_->value(time, thread) * bright,
                    paramA_->value(time, thread));

        // render the thing
        GL::setLineWidth(paramLineWidth_->value(time, thread));
        draw_->renderShader(rs.cameraSpace().projectionMatrix(),
                            cubeViewTrans, viewTrans, trans, &rs.lightSettings());
    }
}





} // namespace MO
