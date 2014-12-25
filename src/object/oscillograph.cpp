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
#include "param/parameterselect.h"
#include "param/parametertext.h"
#include "util/texturesetting.h"
#include "math/funcparser/parser.h"
#include "math/constants.h"


namespace MO {

MO_REGISTER_OBJECT(Oscillograph)

class Oscillograph::Private
{
    public:

    Private(Oscillograph * o)
        : obj               (o),
          textureSet        (new TextureSetting(obj)),
          doRecompile       (false),
          vaoUpdateTime     (-1.23456789232323)
    { }
    ~Private()
    {
        //delete textureSet;
    }

    enum OsciMode
    {
        O_LINEAR,
        O_EQUATION
    };

    enum DrawMode
    {
        D_LINES,
        D_POINTS
    };

    struct EquationObject
    {
        EquationObject()
            : equation (new PPP_NAMESPACE::Parser())
        {
            equation->variables().add("time", &time,
                tr("scene time in seconds").toStdString());
            equation->variables().add("rtime", &rtime,
                tr("radians of scene time").toStdString());
            equation->variables().add("t", &t,
                tr("the position on the oscillograph path [0,1]").toStdString());
            equation->variables().add("rt", &rt,
                tr("the position on the oscillograph path [0,TWO_PI]").toStdString());
            equation->variables().add("value", &value,
                tr("the value of the amplitude at current position").toStdString());
            equation->variables().add("x", &x,
                tr("the output value for the current oscillograph position").toStdString());
            equation->variables().add("y", &y,
                tr("the output value for the current oscillograph position").toStdString());
            equation->variables().add("z", &z,
                tr("the output value for the current oscillograph position").toStdString());
        }

        std::shared_ptr<PPP_NAMESPACE::Parser> equation;
        PPP_NAMESPACE::Float
                time, rtime, t, rt, value, x, y, z;
    };

    void updateEquations();
    void recompile();
    void calcVaoBuffer(Double time, uint thread);

    Oscillograph * obj;
    GL::Drawable * draw;
    TextureSetting * textureSet;

    ParameterFloat * paramR, *paramG, *paramB, *paramA, *paramBright,
                    * paramTimeSpan,
                    * paramValue,
                    * paramWidth,
                    * paramLineWidth,
                    * paramPointSize;
    ParameterInt * paramNumPoints;
    ParameterSelect * paramMode, * paramDrawMode, * paramLineSmooth;
    ParameterText * paramEquation;

    bool doRecompile;

    Double vaoUpdateTime;
    std::vector<gl::GLfloat> vaoBuffer;
    std::vector<EquationObject> equs;
};


Oscillograph::Oscillograph(QObject * parent)
    : ObjectGl      (parent),
      p_            (new Private(this))
{
    setName("Oscillograph");
}

Oscillograph::~Oscillograph()
{
    delete p_;
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

        p_->paramBright = params()->createFloatParameter("bright", "bright", tr("Overall brightness of the color"), 1.0, 0.1);
        p_->paramR = params()->createFloatParameter("red", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        p_->paramG = params()->createFloatParameter("green", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        p_->paramB = params()->createFloatParameter("blue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        p_->paramA = params()->createFloatParameter("alpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);

    params()->endParameterGroup();

    params()->beginParameterGroup("oscgraph", tr("oscillograph"));

        p_->paramValue = params()->createFloatParameter("oscvalue", tr("value"),
                                        tr("The input value for the oscillograph/scope"),
                                        0.0,
                                        0.01, true, true);

        p_->paramNumPoints = params()->createIntParameter("numverts", tr("number points"),
                                            tr("The number of points on the oscillograph path"),
                                            100, 2, 999999,
                                            1, true, false);

        p_->paramTimeSpan = params()->createFloatParameter("timeshift", tr("time span"),
                                            tr("The time to move during one period of the oscillograph in seconds"),
                                            -0.1,
                                            0.01, true, true);

// scope mode

        p_->paramMode = params()->createSelectParameter("oscmode", tr("scope type"),
                                            tr("Selects the scope type"),
                                            { "lin", "equ" },
                                            { tr("linear"), tr("equation") },
                                            { tr("Linear right-to-left display on x/y-plane"),
                                              tr("An equation controlls the position of each oscillograph point") },
                                            { Private::O_LINEAR, Private::O_EQUATION },
                                            Private::O_LINEAR,
                                            true, false);
    // equation
        p_->paramEquation = params()->createTextParameter("oscgequ", tr("equation"),
                    tr("An equation mapping the input samples to positions"),
                    TT_EQUATION,
                    "x = (t - 0.5) * 2;\n"
                    "y = value\n"
                    "\n// " + tr("you can also use 'time', 'rtime' and 'rt'") +
                    "\n// " + tr("'t' is the position on the path") + "[0,1]"
                    , true, false);

        Private::EquationObject tmpequ;
        p_->paramEquation->setVariableNames(tmpequ.equation->variables().variableNames());
        p_->paramEquation->setVariableDescriptions(tmpequ.equation->variables().variableDescriptions());

    // width for linear
        p_->paramWidth = params()->createFloatParameter("scalex", tr("width"),
                                            tr("The width or scale on x-axis"),
                                            1.0,
                                            0.1, true, true);

// draw mode

        p_->paramDrawMode = params()->createSelectParameter("oscdrawmode", tr("draw mode"),
                                            tr("Selects how the points will be drawn"),
                                            { "oneline", "points" },
                                            { tr("lines"), tr("points") },
                                            { tr("All points are connected to neighbours by lines"),
                                              tr("Each point is a sprite (using GL_POINTS)") },
                                            { Private::D_LINES, Private::D_POINTS },
                                            Private::D_LINES,
                                            true, false);

        p_->paramLineSmooth = params()->createBooleanParameter(
                    "linesmooth", tr("antialiased lines"),
                    tr("Should lines be drawn with smoothed edges"),
                    tr("The lines are drawn edgy"),
                    tr("The lines are drawn smoothly (maximum line width might change)"),
                    true,
                    true, false);

        p_->paramLineWidth = params()->createFloatParameter("linewidth", tr("line width"),
                                            tr("The width of the line - currently in pixels - your driver supports maximally %1 and %2 (anti-aliased)")
                                                            // XXX Not initialized before first gl context
                                                            .arg(GL::Properties::maxLineWidth())
                                                            .arg(GL::Properties::maxLineWidthSmooth()),
                                            2, 1, 10000,
                                            0.01, true, true);

        p_->paramPointSize = params()->createFloatParameter("pointsize", tr("point size"),
                                            tr("The size of the points in pixels"),
                                            10.0,
                                            1, true, true);

    params()->endParameterGroup();

    params()->beginParameterGroup("texture", "texture");

        p_->textureSet->createParameters("_col", TextureSetting::TT_NONE, true);

    params()->endParameterGroup();
}

void Oscillograph::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == p_->paramNumPoints)
        requestReinitGl();

    if (p_->textureSet->needsReinit(p))
        requestReinitGl();

    if (p == p_->paramEquation)
        p_->updateEquations();
}

void Oscillograph::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    auto mode = Private::OsciMode(p_->paramMode->baseValue());
    p_->paramWidth->setVisible(mode == Private::O_LINEAR);
    p_->paramEquation->setVisible(mode == Private::O_EQUATION);

    auto dmode = Private::DrawMode(p_->paramDrawMode->baseValue());
    p_->paramPointSize->setVisible(dmode == Private::D_POINTS);
    p_->paramLineWidth->setVisible(dmode == Private::D_LINES);
}

void Oscillograph::setNumberThreads(uint num)
{
    ObjectGl::setNumberThreads(num);

    p_->equs.resize(num);
    p_->updateEquations();
}


void Oscillograph::getNeededFiles(IO::FileList &files)
{
    ObjectGl::getNeededFiles(files);

    p_->textureSet->getNeededFiles(files, IO::FT_TEXTURE);
}

void Oscillograph::Private::updateEquations()
{
    const std::string text = paramEquation->baseValue().toStdString();
    for (auto & e : equs)
    {
        if (!e.equation.get()->parse(text))
            MO_WARNING("parsing failed for equation in Oscillograph '" << obj->name() << "'"
                       " (text = '" << text << "')");
    }
}

const GEOM::Geometry* Oscillograph::geometry() const
{
    return p_->draw ? p_->draw->geometry() : 0;
}

Vec4 Oscillograph::modelColor(Double time, uint thread) const
{
    const auto b = p_->paramBright->value(time, thread);
    return Vec4(
        p_->paramR->value(time, thread) * b,
        p_->paramG->value(time, thread) * b,
        p_->paramB->value(time, thread) * b,
        p_->paramA->value(time, thread));
}

void Oscillograph::initGl(uint /*thread*/)
{
    p_->textureSet->initGl();

    p_->draw = new GL::Drawable(idName());

    // create a line
    auto g = p_->draw->geometry();

    g->setColor(1,1,1,1);

    int num = std::max(2, p_->paramNumPoints->baseValue());
    for (int i=0; i<num; ++i)
    {
        g->addVertexAlways(i, 0, 0);
        if (i > 0)
            g->addLine(i - 1, i);
    }
    // force p_->calcVaoBuffer()
    p_->vaoUpdateTime = -1000000.123456;

    p_->recompile();
}

void Oscillograph::Private::recompile()
{
    GL::ShaderSource * src = new GL::ShaderSource();

    src->loadDefaultSource();

    if (textureSet->isEnabled())
    {
        src->addDefine("#define MO_ENABLE_TEXTURE");
        src->addDefine("#define MO_USE_POINT_COORD");
    }
    if (textureSet->isCube())
        src->addDefine("#define MO_TEXTURE_IS_FULLDOME_CUBE");

    draw->setShaderSource(src);
    draw->createOpenGl();

    vaoBuffer.resize(draw->geometry()->numVertices() * 3);

}

void Oscillograph::releaseGl(uint /*thread*/)
{
    p_->textureSet->releaseGl();

    if (p_->draw->isReady())
        p_->draw->releaseOpenGl();

    delete p_->draw;
    p_->draw = 0;
}

/*void Oscillograph::numberLightSourcesChanged(uint thread)
{
    doRecompile_ = true;
}*/

void Oscillograph::Private::calcVaoBuffer(Double time, uint thread)
{
    const uint numPoints = vaoBuffer.size() / 3;

    const Double
            timeSpan = paramTimeSpan->value(time, thread),
            fac = 1.0 / (numPoints - 1);

    // calculate osci positions
    switch (OsciMode(paramMode->value(time, thread)))
    {
        case O_LINEAR:
        {
            const gl::GLfloat scalex = paramWidth->value(time, thread);

            gl::GLfloat * pos = &vaoBuffer[0];
            for (uint i = 0; i < numPoints; ++i)
            {
                const Double t = Double(i) * fac;

                *pos++ = (t - 0.5) * scalex;
                *pos++ = paramValue->value(time + (1.0-t) * timeSpan, thread);
                *pos++ = 0.0f;
            }
        }
        break;

        case O_EQUATION:
        {
            Private::EquationObject * equ = &equs[thread];

            equ->time = time;
            equ->rtime = time * TWO_PI;

            gl::GLfloat * pos = &vaoBuffer[0];
            for (uint i = 0; i < numPoints; ++i)
            {
                const Double t = Double(i) * fac;

                // update equation variables
                equ->t = t;
                equ->rt = t * TWO_PI;
                equ->value = paramValue->value(time + (1.0-t) * timeSpan, thread);
                // don't allow physics ;)
                // would drift on clients
                equ->x = equ->y = equ->z = 0.0;

                // calc
                equ->equation.get()->eval();

                // get output
                *pos++ = equ->x;
                *pos++ = equ->y;
                *pos++ = equ->z;
            }
        }
    }

}

void Oscillograph::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    const Mat4& trans = transformation();
    const Mat4  cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
    const Mat4  viewTrans = rs.cameraSpace().viewMatrix() * trans;

    if (p_->doRecompile)
    {
        p_->doRecompile = false;
        p_->recompile();
    }

    if (p_->draw->isReady())
    {
        // update geometry
        if (p_->vaoUpdateTime != time)
        {
            p_->vaoUpdateTime = time;

            GL::BufferObject * buf = p_->draw->vao()->getBufferObject(
                        GL::VertexArrayObject::A_POSITION);
            if (buf && p_->vaoBuffer.size() > 3)
            {
                p_->calcVaoBuffer(time, thread);

                // move to device
                buf->bind();
                buf->upload(&p_->vaoBuffer[0], gl::GL_DYNAMIC_DRAW);
            }
        }


        // update uniforms
        const auto bright = p_->paramBright->value(time, thread);
        p_->draw->setAmbientColor(
                    p_->paramR->value(time, thread) * bright,
                    p_->paramG->value(time, thread) * bright,
                    p_->paramB->value(time, thread) * bright,
                    p_->paramA->value(time, thread));

        // -- render the thing --

        p_->textureSet->bind();

        switch (Private::DrawMode(p_->paramDrawMode->baseValue()))
        {
            case Private::D_LINES:
                p_->draw->setDrawType(gl::GL_LINE_STRIP);
                GL::setLineSmooth(p_->paramLineSmooth->value(time, thread) != 0);
                GL::setLineWidth(p_->paramLineWidth->value(time, thread));
            break;

            case Private::D_POINTS:
                p_->draw->setDrawType(gl::GL_POINTS);
                MO_CHECK_GL( gl::glPointSize(p_->paramPointSize->value(time, thread)) );
            break;
        }

        p_->draw->renderShader(rs.cameraSpace().projectionMatrix(),
                            cubeViewTrans, viewTrans, trans, &rs.lightSettings());
    }
}





} // namespace MO
