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
#include "object/param/parameters.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/util/texturesetting.h"
#include "math/funcparser/parser.h"
#include "math/constants.h"
#include "math/fft.h"

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

    enum SourceMode
    {
        S_AMPLITUDE,
        S_SPECTRUM,
        S_SPECTRUM_PHASE
    };

    enum OsciMode
    {
        O_1D,
        O_2D,
        O_3D,
        O_EQUATION
    };

    enum DrawMode
    {
        D_LINES,
        D_BARS,
        D_POINTS
    };

    struct EquationObject
    {
        EquationObject()
            : equation  (new PPP_NAMESPACE::Parser()),
              values    (3)
        {
            equation->variables().add("time", &time,
                tr("scene time in seconds").toStdString());
            equation->variables().add("rtime", &rtime,
                tr("radians of scene time").toStdString());
            equation->variables().add("t", &t,
                tr("the position on the oscillograph path [0,1]").toStdString());
            equation->variables().add("rt", &rt,
                tr("the position on the oscillograph path [0,TWO_PI]").toStdString());
            equation->variables().add("value", &values[0],
                tr("the 1st input value at current position on the path (same as value1)").toStdString());
            equation->variables().add("value1", &values[0],
                tr("the 1st input value at current position on the path").toStdString());
            equation->variables().add("value2", &values[1],
                tr("the 2nd input value at current position on the path").toStdString());
            equation->variables().add("value3", &values[2],
                tr("the 3rd input value at current position on the path").toStdString());
            equation->variables().add("x", &x,
                tr("the output value for the current oscillograph position").toStdString());
            equation->variables().add("y", &y,
                tr("the output value for the current oscillograph position").toStdString());
            equation->variables().add("z", &z,
                tr("the output value for the current oscillograph position").toStdString());
        }

        std::shared_ptr<PPP_NAMESPACE::Parser> equation;
        PPP_NAMESPACE::Float
                time, rtime, t, rt, x, y, z;
        std::vector<PPP_NAMESPACE::Float> values;
    };

    SourceMode sourceMode() const { return (SourceMode)paramSourceMode->baseValue(); }
    DrawMode drawMode() const { return (DrawMode)paramDrawMode->baseValue(); }
    OsciMode osciMode() const { return (OsciMode)paramMode->baseValue(); }
    void updateEquations();
    void recompile();
    void calcValueBuffer(const RenderTime & time);
    void calcVaoBuffer(const RenderTime & time);

    Oscillograph * obj;
    GL::Drawable * draw;
    TextureSetting * textureSet;

    ParameterFloat * paramR, *paramG, *paramB, *paramA, *paramBright,
                    * paramTimeSpan,
                    * paramAmp,
                    * paramWidth,
                    * paramLineWidth,
                    * paramPointSize,
                    * paramBlurWidth;
    ParameterInt * paramNumPoints,
                 * paramBlurFrames;
    ParameterSelect * paramMode, * paramSourceMode,
                    * paramDrawMode, * paramLineSmooth,
                    * paramFftSize;
    ParameterText * paramEquation;

    std::vector<ParameterFloat*> paramValue;

    bool doRecompile;

    uint numChannels, numPoints;
    Double vaoUpdateTime;
    std::vector<gl::GLfloat> vaoBuffer, valueBuffer;
    std::vector<EquationObject> equs;
    std::vector<MATH::Fft<gl::GLfloat>> ffts;

};


Oscillograph::Oscillograph()
    : ObjectGl      (),
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
    initParameterGroupExpanded("color");

        p_->paramBright = params()->createFloatParameter("bright", "bright", tr("Overall brightness of the color"), 1.0, 0.1);
        p_->paramR = params()->createFloatParameter("red", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        p_->paramG = params()->createFloatParameter("green", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        p_->paramB = params()->createFloatParameter("blue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        p_->paramA = params()->createFloatParameter("alpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);
        p_->paramA->setDefaultEvolvable(false);

    params()->endParameterGroup();

    params()->beginParameterGroup("oscgraph", tr("oscillograph"));

        p_->paramValue.resize(3);
        p_->paramValue[0] = params()->createFloatParameter("oscvalue", tr("value 1"),
                                        tr("The first input value for the oscillograph/scope"),
                                        0.0,
                                        0.01, true, true);
        p_->paramValue[1] = params()->createFloatParameter("oscvalue2", tr("value 2"),
                                        tr("The 2nd input value for the oscillograph/scope"),
                                        0.0,
                                        0.01, true, true);
        p_->paramValue[2] = params()->createFloatParameter("oscvalue3", tr("value 3"),
                                        tr("The 3rd input value for the oscillograph/scope"),
                                        0.0,
                                        0.01, true, true);
        for (int i=0; i<3; ++i)
            p_->paramValue[i]->setVisibleGraph(true);

        p_->paramAmp = params()->createFloatParameter("oscamp", tr("amplitude"),
                                        tr("The amplitude of the oscillograph/scope"),
                                        1.0,
                                        0.1, true, true);

        p_->paramNumPoints = params()->createIntParameter("numverts", tr("number points"),
                                            tr("The number of points on the oscillograph path"),
                                            100, 2, 999999,
                                            1, true, false);

        p_->paramTimeSpan = params()->createFloatParameter("timeshift", tr("time span"),
                                            tr("The time to move during one period of the oscillograph in seconds"),
                                            -0.1,
                                            0.01, true, true);

// scope mode

        p_->paramSourceMode = params()->createSelectParameter("oscsrcmode", tr("source type"),
                                            tr("Selects the scope type"),
                                            { "amp", "fft", "fftph" },
                                            { tr("amplitude"), tr("spectral amplitude"), tr("spectral phase") },
                                            { tr("The amplitude of the signal over the number of samples"),
                                              tr("A frequency spectrum over the number of samples"),
                                              tr("The phase spectrum over the number of samples") },
                                            { Private::S_AMPLITUDE, Private::S_SPECTRUM, Private::S_SPECTRUM_PHASE },
                                            Private::S_AMPLITUDE,
                                            true, false);

        p_->paramFftSize = params()->createSelectParameter("osc_fftsize", tr("fft size"),
                    tr("The size of the fourier window in samples"),
                    { "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" },
                    { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
                    { "", "", "", "", "", "", "", "", "", "", "", "", "" },
                    { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
                    512, true, false);

        p_->paramMode = params()->createSelectParameter("oscmode", tr("scope type"),
                                            tr("Selects the scope type"),
                                            { "1d", "2d", "3d", "equ" },
                                            { tr("1-dimensional"), tr("2-dimensional"), tr("3-dimensional"), tr("equation") },
                                            { tr("Linear right-to-left display on x/y-plane"),
                                              tr("Two channels on the x/y-plane, like e.g. a phase difference scope"),
                                              tr("Each channel is mapped to an axis in 3d space"),
                                              tr("An equation controlls the position of each oscillograph point") },
                                            { Private::O_1D, Private::O_2D, Private::O_3D, Private::O_EQUATION },
                                            Private::O_1D,
                                            true, false);
    // equation
        p_->paramEquation = params()->createTextParameter("oscgequ", tr("equation"),
                    tr("An equation mapping the input samples to positions"),
                    TT_EQUATION,
                    "x = (t - 0.5) * 2;\n"
                    "y = value\n"
                    "\n// " + tr("you can also use 'time', 'rtime' and 'rt'") +
                    "\n// " + tr("'t' is the position on the path") + " [0,1]"
                    "\n// " + tr("input values are") + " value or value1, value2 and value3"
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
                                            { "oneline", "bar", "points" },
                                            { tr("line"), tr("solid"), tr("points") },
                                            { tr("All points are connected to neighbours by lines"),
                                              tr("The envlope is drawn as a solid shape"),
                                              tr("Each point is a sprite (using GL_POINTS)") },
                                            { Private::D_LINES, Private::D_BARS, Private::D_POINTS },
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
                                                            .arg(GL::Properties::staticInstance().lineWidth[0])
                                                            .arg(GL::Properties::staticInstance().lineWidth[1]),
                                            2, 1, 10000,
                                            0.01, true, true);

        p_->paramPointSize = params()->createFloatParameter("pointsize", tr("point size"),
                                            tr("The size of the points in pixels"),
                                            10.0,
                                            1, true, true);

// motion blur

        p_->paramBlurFrames = params()->createIntParameter("blurframes", tr("motion blur frames"),
                                            tr("The number of frames for motion blur multi-sampling"),
                                            0, 0, 1000,
                                            1, true, true);

        p_->paramBlurWidth = params()->createFloatParameter("blurwidth", tr("aperture time"),
                                            tr("The time range in seconds in which to blur the graphic"),
                                            0.01, 0.0, 1000.0,
                                            0.01, true, true);

    params()->endParameterGroup();

    params()->beginParameterGroup("texture", "texture");

        p_->textureSet->createParameters(
                    "_col", tr("color texture"), ParameterTexture::IT_WHITE);

    params()->endParameterGroup();
}

void Oscillograph::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == p_->paramSourceMode
        || p == p_->paramFftSize
        || p == p_->paramNumPoints
        || p == p_->paramDrawMode
        || p == p_->paramMode)
        requestReinitGl();

    if (p == p_->paramEquation)
        p_->updateEquations();
}

void Oscillograph::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    p_->textureSet->updateParameterVisibility();

    auto mode = Private::OsciMode(p_->paramMode->baseValue());
    p_->paramWidth->setVisible(mode == Private::O_1D);
    p_->paramEquation->setVisible(mode == Private::O_EQUATION);

    auto dmode = Private::DrawMode(p_->paramDrawMode->baseValue());
    p_->paramPointSize->setVisible(dmode == Private::D_POINTS);
    p_->paramLineWidth->setVisible(dmode == Private::D_LINES);
    p_->paramLineSmooth->setVisible(dmode == Private::D_LINES);

    auto smode = p_->sourceMode();
    bool spec = smode == Private::S_SPECTRUM || smode == Private::S_SPECTRUM_PHASE;
    p_->paramFftSize->setVisible(spec);
    p_->paramNumPoints->setVisible(!spec);
    p_->paramTimeSpan->setVisible(!spec);

    bool manyInputs = mode == Private::O_EQUATION;
    p_->paramValue[1]->setVisible(manyInputs || mode == Private::O_2D || mode == Private::O_3D);
    p_->paramValue[2]->setVisible(manyInputs || mode == Private::O_3D);
}

void Oscillograph::setNumberThreads(uint num)
{
    ObjectGl::setNumberThreads(num);

    p_->equs.resize(num);
    p_->updateEquations();
    p_->ffts.resize(num);
}


void Oscillograph::getNeededFiles(IO::FileList &files)
{
    ObjectGl::getNeededFiles(files);

    p_->textureSet->getNeededFiles(files);
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

Vec4 Oscillograph::modelColor(const RenderTime& time) const
{
    const auto b = p_->paramBright->value(time);
    return Vec4(
        p_->paramR->value(time) * b,
        p_->paramG->value(time) * b,
        p_->paramB->value(time) * b,
        p_->paramA->value(time));
}

void Oscillograph::initGl(uint thread)
{
    p_->draw = new GL::Drawable(idName());

    // get number of input channels
    p_->numChannels = 1;
    if (p_->osciMode() == Private::O_2D)
        p_->numChannels = 2;
    if (p_->osciMode() == Private::O_3D
        || p_->osciMode() == Private::O_EQUATION)
        p_->numChannels = 3;

    // choose number of points on path
    p_->numPoints = std::max(2, p_->paramNumPoints->baseValue());
    if (p_->sourceMode() == Private::S_SPECTRUM)
    {
        p_->numPoints = p_->paramFftSize->baseValue();
        p_->ffts[thread].setSize(p_->numPoints);
        p_->numPoints /= 2;
    }
    p_->valueBuffer.resize(p_->numPoints * p_->numChannels);

    // build geometry
    auto g = p_->draw->geometry();
    g->setColor(1,1,1,1);

    switch (p_->drawMode())
    {
        // create a line
        case Private::D_LINES:
        case Private::D_POINTS:
            for (uint i=0; i<p_->numPoints; ++i)
            {
                g->addVertexAlways(i, 0, 0);
                if (i > 0)
                    g->addLine(i - 1, i);
            }
        break;

        case Private::D_BARS:
        {
            // top
            for (uint i=0; i<p_->numPoints; ++i)
            {
                g->setTexCoord(float(i) / (p_->numPoints-1), 1);
                g->addVertexAlways(i, 1, 0);
            }
            // bottom
            for (uint i=0; i<p_->numPoints; ++i)
            {
                g->setTexCoord(float(i) / (p_->numPoints-1), 0);
                g->addVertexAlways(i, 0, 0);
            }
            // tris
            for (uint i=0; i<p_->numPoints-1; ++i)
            {
                g->addTriangle(p_->numPoints + i, i + 1, i);
                g->addTriangle(p_->numPoints + i, p_->numPoints + i + 1, i + 1);
            }
        }
        break;
    }

    // space to upload new position data
    p_->vaoBuffer.resize(g->numVertices() * 3);

    // force p_->calcVaoBuffer()
    p_->vaoUpdateTime = -1000000.123456;

    p_->recompile();
}

void Oscillograph::Private::recompile()
{
    GL::ShaderSource * src = new GL::ShaderSource();

    src->loadDefaultSource();

    //if (textureSet->isEnabled())
    {
        src->addDefine("#define MO_ENABLE_TEXTURE");
        if (drawMode() == D_POINTS)
            src->addDefine("#define MO_USE_POINT_COORD");
    }
    if (textureSet->isCube())
        src->addDefine("#define MO_TEXTURE_IS_FULLDOME_CUBE");

    draw->setShaderSource(src);
    draw->createOpenGl();

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

void Oscillograph::Private::calcValueBuffer(const RenderTime& time)
{
    auto writep = &valueBuffer[0];

    if (sourceMode() == Private::S_AMPLITUDE)
    {
        const Double
                timeSpan = paramTimeSpan->value(time),
                fac = 1.0 / (valueBuffer.size() - 1);

        for (uint j = 0; j < numChannels; ++j)
        for (uint i = 0; i < numPoints; ++i)
        {
            const Double t = Double(i) * fac;
            *writep++ = paramValue[j]->value(time + (1.0-t) * timeSpan);
        }
    }
    else
    {
        MATH::Fft<Float> * fft = &ffts[time.thread()];

        for (uint j = 0; j < numChannels; ++j)
        {
            // fill fft buffer
            for (uint i = 0; i < fft->size(); ++i)
                fft->buffer()[i] =
                        paramValue[j]->value(time - SamplePos(i));

            // perform it
            fft->fft();
            fft->getAmplitudeAndPhase();

            // copy
            uint offset = (sourceMode() == Private::S_SPECTRUM_PHASE)
                    ? (fft->size() / 2) : 0;

            for (uint i=0; i<numPoints; ++i)
                *writep++ = fft->buffer()[i + offset];
        }
    }
}

void Oscillograph::Private::calcVaoBuffer(const RenderTime& time)
{
    const Double
            fac = 1.0 / (numPoints - 1),
            amp = paramAmp->value(time);

    // calculate osci positions
    switch (osciMode())
    {
        case O_1D:
        {
            const gl::GLfloat scalex = paramWidth->value(time);

            gl::GLfloat * pos = &vaoBuffer[0];
            for (uint i = 0; i < numPoints; ++i)
            {
                const Double t = Double(i) * fac;

                *pos++ = (t - 0.5) * scalex;
                *pos++ = valueBuffer[i] * amp;
                *pos++ = 0.0f;
            }

            // bars have a second line of points at the bottom
            if (drawMode() == D_BARS)
            for (uint i = 0; i <numPoints; ++i)
            {
                const Double t = Double(i) * fac;

                *pos++ = (t - 0.5) * scalex;
                *pos++ = 0.0f;
                *pos++ = 0.0f;
            }
        }
        break;

        case O_2D:
        {
            gl::GLfloat * pos = &vaoBuffer[0];
            for (uint i = 0; i < numPoints; ++i)
            {
                *pos++ = valueBuffer[i] * amp;
                *pos++ = valueBuffer[i + numPoints] * amp;
                *pos++ = 0.0f;
            }

            // bars have a second line of points at the bottom
            if (drawMode() == D_BARS)
            for (uint i = 0; i <numPoints; ++i)
            {
                *pos++ = 0.0f;
                *pos++ = 0.0f;
                *pos++ = 0.0f;
            }
        }
        break;

        case O_3D:
        {
            gl::GLfloat * pos = &vaoBuffer[0];
            for (uint i = 0; i < numPoints; ++i)
            {
                *pos++ = valueBuffer[i] * amp;
                *pos++ = valueBuffer[i + numPoints] * amp;
                *pos++ = valueBuffer[i + (numPoints<<1)] * amp;
            }

            // bars have a second line of points at the bottom
            if (drawMode() == D_BARS)
            for (uint i = 0; i <numPoints; ++i)
            {
                *pos++ = 0.0f;
                *pos++ = 0.0f;
                *pos++ = 0.0f;
            }
        }
        break;

        case O_EQUATION:
        {
            Private::EquationObject * equ = &equs[time.thread()];

            equ->time = time.second();
            equ->rtime = time.second() * TWO_PI;

            gl::GLfloat * pos = &vaoBuffer[0];
            for (uint i = 0; i < numPoints; ++i)
            {
                const Double t = Double(i) * fac;

                // update equation variables
                equ->t = t;
                equ->rt = t * TWO_PI;
                for (uint j=0; j<numChannels; ++j)
                    equ->values[j] = valueBuffer[i + j * numPoints] * amp;
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

            // bars have a second line of points at the bottom
            // XXX hacky.
            for (uint j=0; j<numChannels; ++j)
                equ->values[j] = 0;
            if (drawMode() == D_BARS)
            for (uint i = 0; i <numPoints; ++i)
            {
                const Double t = Double(i) * fac;

                // update equation variables
                equ->t = t;
                equ->rt = t * TWO_PI;
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
        break;
    }
}




void Oscillograph::renderGl(const GL::RenderSettings& rs, const RenderTime& rtime)
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
        const int mbframes = p_->paramBlurFrames->value(rtime) + 1;
        const Double mbtimefac = 1.0 / Double(std::max(1, mbframes - 1))
                * p_->paramBlurWidth->value(rtime);

        p_->vaoUpdateTime = rtime.second() + 1.;
        for (int mb = 0; mb < mbframes; ++mb)
        {
            RenderTime time(rtime);
            time -= Double(mb) * mbtimefac;

            // update geometry
            if (p_->vaoUpdateTime != time.second())
            {
                p_->vaoUpdateTime = time.second();

                GL::BufferObject * buf = p_->draw->vao()->getAttributeBufferObject(
                            GL::VertexArrayObject::A_POSITION);
                if (buf && p_->vaoBuffer.size() > 3)
                {
                    p_->calcValueBuffer(time);
                    p_->calcVaoBuffer(time);

                    // move to device
                    buf->bind();
                    buf->upload(&p_->vaoBuffer[0], gl::GL_DYNAMIC_DRAW);
                }
            }


            // update uniforms
            const auto bright = p_->paramBright->value(time);
            p_->draw->setAmbientColor(
                        p_->paramR->value(time) * bright,
                        p_->paramG->value(time) * bright,
                        p_->paramB->value(time) * bright,
                        p_->paramA->value(time));

            // -- render the thing --

            p_->textureSet->bind(time);

            switch (Private::DrawMode(p_->paramDrawMode->baseValue()))
            {
                case Private::D_LINES:
                    p_->draw->setDrawType(gl::GL_LINE_STRIP);
                    GL::Properties::staticInstance().setLineSmooth(p_->paramLineSmooth->value(time) != 0);
                    GL::Properties::staticInstance().setLineWidth(p_->paramLineWidth->value(time));
                break;

                case Private::D_BARS:
                    p_->draw->setDrawType(gl::GL_TRIANGLES);
                break;

                case Private::D_POINTS:
                    p_->draw->setDrawType(gl::GL_POINTS);
                    GL::Properties::staticInstance().setPointSize(p_->paramPointSize->value(time));
                break;
            }

            p_->draw->renderShader(rs.cameraSpace().projectionMatrix(),
                                cubeViewTrans, viewTrans, trans, &rs.lightSettings());
        }
    }
}





} // namespace MO
