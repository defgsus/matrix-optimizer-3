/** @file helptextbrowser.cpp

    @brief QTextBrowser with reimplemented loadResource() to work with resource files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#include <QTextStream>
#include <QFile>
#include <QImage>
#include <QPainter>

#include "helptextbrowser.h"
#include "io/log.h"
#include "io/error.h"
#include "math/funcparser/parser.h"
#include "gui/painter/grid.h"
#include "gui/painter/valuecurve.h"

namespace MO {
namespace GUI {

namespace
{
    class EquationData : public PAINTER::ValueCurveData
    {
    public:
        mutable PPP_NAMESPACE::Parser p;
        mutable Double x;

        EquationData(const QString& equ)
        {
            p.variables().add("x", &x);
            p.parse(equ.toStdString());
        }
        Double value(Double time) const Q_DECL_OVERRIDE
        {
            x = time;
            return p.eval();
        }
    };
}


HelpTextBrowser::HelpTextBrowser(QWidget *parent) :
    QTextBrowser(parent)
{
    setSearchPaths(QStringList() << ":/helpimg" << ":/img" << ":/texture");
}

QVariant HelpTextBrowser::loadResource(int type, const QUrl &url)
{
    if (type == QTextDocument::HtmlResource)
    {
        const QString name = url.url();
        const QString fn = ":/help/"
            + (name.contains(".html")? name : name + ".html");

        QFile f(fn);
        if (!f.open(QFile::ReadOnly))
        {
            return tr("<h3>%1 is missing</h3>").arg(fn);
        }

        QTextStream s(&f);
        QString doc = s.readAll();

        return addRuntimeInfo_(doc, fn);
    }

    if (type == QTextDocument::ImageResource)
    {
        if (url.url().startsWith("_equ"))
            return getFunctionImage(url.url());
    }

    return QTextBrowser::loadResource(type, url);
}


QString HelpTextBrowser::addRuntimeInfo_(
        const QString &orgdoc, const QString &filename) const
{
    if (filename.contains("equation.html"))
    {
        QString str;

        PPP_NAMESPACE::Parser p;

        std::vector<PPP_NAMESPACE::Variable*> vars;
        p.variables().getVariables(vars, false);
        for (PPP_NAMESPACE::Variable * v : vars)
        {
            if (!v->isConst())
                continue;

            str += QString("<b>%1<&b> = %2<br/>\n")
                    .arg(QString::fromStdString(v->name())).arg(v->value());
        }

        QString doc(orgdoc);
        doc.replace("!CONSTANTS!", str);

        str = "<table border=\"0\">";
        auto funcs = p.functions().getFunctions();
        for (const PPP_NAMESPACE::Function * f : funcs)
        {
            if (f->type() != PPP_NAMESPACE::Function::FUNCTION
                || f->name() == "?")
                continue;

            const QString anchor = QString::fromStdString(f->name())
                                    + QString::number(f->num_param());

            str += "<tr><td><a name=\"" + anchor + "\"></a><b>"
                    + QString::fromStdString(f->name()) + "</b>(";
            for (int i=0; i<f->num_param(); ++i)
            {
                str += QChar('a'+i);
                if (i<f->num_param()-1)
                    str += ", ";
            }
            str += ")</td><td>" + getFunctionDescription_(f) + "<br/></td></tr>\n";
        }
        str += "</table>";

        doc.replace("!FUNCTIONS!", str);

        return doc;
    }


    return orgdoc;
}

QImage HelpTextBrowser::getFunctionImage(const QString &url) const
{
    //    MO_DEBUG("making image " << equation);

    PPP_NAMESPACE::Float
            xstart = url.section(QChar('#'), 1, 1).toDouble(),
            xend = url.section(QChar('#'), 2, 2).toDouble(),
            ymin = url.section(QChar('#'), 3, 3).toDouble(),
            ymax = url.section(QChar('#'), 4, 4).toDouble();
    const QString
            equation = url.section(QChar('#'), 5, 5),
            widthstr = url.section(QChar('#'), 6, 6),
            heightstr = url.section(QChar('#'), 7, 7);

    // create parser and test equation
    EquationData data(equation);
    if (!data.p.ok())
        return QImage();

    // determine width and height
    int width = 400, height = 100;
    if (!widthstr.isEmpty())
    {
        width = widthstr.toInt();
        if (heightstr.isEmpty())
            height = width;
        else
            height = heightstr.toInt();
    }

    // expand the y-view a little
    ymin -= (ymax-ymin) / 50;
    ymax += (ymax-ymin) / 50;

    // get image and painter
    QImage img(width,height,QImage::Format_RGB32);
    img.fill(Qt::black);

    QPainter paint(&img);
    paint.setRenderHint(QPainter::Antialiasing, true);

    UTIL::ViewSpace vs(xstart,ymin, xend-xstart, ymax-ymin);

    // draw background grid
    PAINTER::Grid grid;
    grid.setOptions(PAINTER::Grid::O_DrawAll);
    grid.setViewSpace(vs);
    grid.paint(paint);

    // -- value curve --

    PAINTER::ValueCurve curve;
    curve.setViewSpace(vs);
    curve.setCurveData(&data);
    curve.paint(paint);

    return img;
}

QString HelpTextBrowser::getFunctionDescription_(
        const PPP_NAMESPACE::Function * f) const
{
    if (f->name() == "abs")
        return tr("Returns the absolute value - that is the result is always positive");
    if (f->name() == "sign")
        return tr("Returns +1.0 if <i>a</i> is positive, -1.0 if <i>a</i> is negative and "
                  "0.0 if <i>a</i> is also zero");
    if (f->name() == "floor")
        return tr("Returns <i>a</i> without the fractional part, e.g. 2.7 becomes 2.0")
                  +"<br/><b>floor(x)</b>:<br/><img src=\"_equ#0#8#0#8#floor(x)\"/>";
    if (f->name() == "ceil")
        return tr("Returns <i>a</i> without the fractional part plus one, e.g 2.3 becomes 3.0")
                  +"<br/><b>ceil(x)</b>:<br/><img src=\"_equ#0#8#0#8#ceil(x)\"/>";
    if (f->name() == "round")
        return tr("Returns <i>a</i> rounded to the nearest integer, e.g. 2.3 becomes 2.0 and "
                  "2.7 becomes 3.0")
                  +"<br/><b>round(x)</b>:<br/><img src=\"_equ#0#8#0#8#round(x)\"/>";
    if (f->name() == "frac")
        return tr("Returns the fractional part (the digits right of the point)");

    if (f->name() == "min")
        return tr("Returns the smallest of the values <i>a</i> and <i>b</i>");
    if (f->name() == "max")
        return tr("Returns the largest of the values <i>a</i> and <i>b</i>");
    if (f->name() == "max")
        return tr("Limits the value <i>a</i> to minimally <i>b</i> and maximally <i>c</i>");
    if (f->name() == "quant")
        return tr("Quantizes <i>a</i> to the interval <i>b</i>")
                  +"<br/><b>quant(x,0.3)</b>:<br/><img src=\"_equ#0#2#0#2#quant(x,0.3)\"/>";
    if (f->name() == "mod")
        return tr("Returns <i>a modulo b</i>, that is, <i>a</i> will always be in the "
                  "range of [0,<i>b</i>)");
    if (f->name() == "smod")
        return tr("Returns <i>a modulo b</i> if <i>a</i> is positive and <i>b - (-a modulo b)</i> if a "
                  "is negative, that is, <i>a</i> will always be in the range of [0,<i>b</i>). "
                  "This function should be used for creating continious functions from a value that "
                  "might be negative as well.");

    if (f->name() == "sin")
        return tr("Returns the sinus of <i>a</i>. <i>a</i> is in radians, so a full interval "
                  "is [0,<i>TWO_PI</i>]")
                  +"<br/><b>sin(x)</b>:<br/><img src=\"_equ#0#6.283#-1#1#sin(x)\"/>";
    if (f->name() == "sinh")
        return tr("Returns the sinus hyperbolicus of <i>a</i>.")
                  +"<br/><b>sinh(x)</b>:<br/><img src=\"_equ#-3.1416#3.1416#-12#12#sinh(x)\"/>";
    if (f->name() == "asin")
        return tr("Returns the arcus sinus of <i>a</i>. This translates an angle back to it's sine "
                  "part. <i>a</i> is in the range of [-1,1], the result is in radians.")
                  +QString("<br/>asin(0.0) = %1, asin(1.0) = %2")
                        .arg(std::asin(0.0)).arg(std::asin(1.0))
                  +"<br/><b>asin(x)</b>:<br/><img src=\"_equ#-1#1#-1.58#1.58#asin(x)\"/>";
    if (f->name() == "cos")
        return tr("Returns the cosinus of <i>a</i>. <i>a</i> is in radians, so a full interval "
                  "is [0,<i>TWO_PI</i>]")
                  +"<br/><b>cos(x)</b>:<br/><img src=\"_equ#0#6.283#-1#1#cos(x)\"/>";
    if (f->name() == "cosh")
        return tr("Returns the cosinus hyperbolicus of <i>a</i>.")
                  +"<br/><b>cosh(x)</b>:<br/><img src=\"_equ#-3.1416#3.1416#0#12#cosh(x)\"/>";
    if (f->name() == "acos")
        return tr("Returns the arcus cosinus of <i>a</i>. This translates an angle back to it's "
                  "cosine part. <i>a</i> is in the range of [-1,1], the result is in radians.")
                  +QString("<br/>acos(0.0) = %1, acos(1.0) = %2")
                        .arg(std::acos(0.0)).arg(std::acos(1.0))
                  +"<br/><b>acos(x)</b>:<br/><img src=\"_equ#-1#1#-1.58#1.58#acos(x)\"/>";
    if (f->name() == "tan")
        return tr("Returns the tangens of <i>a</i>. <i>a</i> is in radians, so a full interval "
                  "is [0,<i>TWO_PI</i>]")
                  +"<br/><b>tan(x)</b>:<br/><img src=\"_equ#0#6.283#-4#4#tan(x)\"/>";
    if (f->name() == "tanh")
        return tr("Returns the tangens hyperbolicus of <i>a</i>.<br/>"
                  "This function has the nice property of fitting all input values into the "
                  "range [-1,1], so it can be usefull as a saturation function.")
                  +"<br/><b>tanh(x)</b>:<br/><img src=\"_equ#-4#4#-1#1#tanh(x)\"/>";
    if (f->name() == "atan" && f->num_param() == 1)
        return tr("Returns the arcus tangens of <i>a</i>. <i>a</i> is in the range of [-1,1], "
                  "the result is in radians.")
                  +"<br/><b>atan(x)</b>:<br/><img src=\"_equ#-1#1#-1.58#1.58#atan(x)\"/>";
    if (f->name() == "atan" && f->num_param() == 2)
        return tr("Returns the arcus tangens of <i>a</i>/<i>b</i>. The result is in radians "
                  "ranging from -<i>PI</i> to +<i>PI</i>.<br/>"
                  "This function is useful to translate a vector position into an angle. "
                  "For any point (except 0,0) <b>atan2(y,x)</b> returns the angle in radians "
                  "between the positive x-axis and the point x,y");
    if (f->name() == "sinc")
        return tr("Calculates <b>sin(<i>a</i>) / <i>a</i></b>, which gives a sine wave with "
                  "a maximum at zero, fading out in negative and positive directions.")
                  +"<br/><b>sinc(x)</b>:<br/><img src=\"_equ#-20#20#-1#1#sinc(x)\"/>";

    if (f->name() == "exp")
        return tr("The exponential function. This raises <i>e</i> (Euler's constant) to "
                  "the power of <i>a</i>");
    if (f->name() == "ln")
        return tr("The natural logarithm function returns the exponent to which <i>e</i> "
                  "(Euler's constant) must be raised to receive <i>a</i>");
    if (f->name() == "logistic")
        return tr("A function borrowed mainly from statistics. The result equals "
                  "<b>1 / (1 + exp(<i>a</i>))</b> and is always in the range of [0,1].")
                  +"<br/><b>logistic(x)</b>:<br/><img src=\"_equ#-5#5#0#1#logistic(x)\"/>";
    if (f->name() == "pow")
        return tr("Raises <i>a</i> to the power of <i>b</i>");
    if (f->name() == "sqrt")
        return tr("Returns the square root (or second root) of <i>a</i>");
    if (f->name() == "root")
        return tr("Returns the <i>b</i>th root of <i>a</i>");

    if (f->name() == "smstep")
        return tr("Smoothly fades between 0 and 1 for the value <i>c</i> between it's "
                  "boundaries <i>a</i> and <i>b</i>.")
                  +"<br/><b>smstep(1, 2, x)</b>:<br/><img src=\"_equ#0#4#0#1#smstep(1,2,x)\"/>";
    if (f->name() == "smstep2")
        return tr("Smoothly fades between 0 and 1 for the value <i>c</i> between it's "
                  "boundaries <i>a</i> and <i>b</i>. This version has slightly different "
                  "boundary derivatives as the <b>smstep</b> function. ")
                  +"<br/><b>smstep2(1, 2, x)</b>:<br/><img src=\"_equ#0#4#0#1#smstep2(1,2,x)\"/>";
    if (f->name() == "smquant")
        return tr("Works like <b>quant</b> but smoothly fades between each quantized value.")
                  +"<br/><b>smquant(x, 1)</b>:<br/><img src=\"_equ#0#4#0#4#smquant(x,1)\"/>";
    if (f->name() == "smquant2")
        return tr("Sames as <b>smquant</b> but slightly other derivatives of the smoothing function.")
                  +"<br/><b>smquant2(x, 1)</b>:<br/><img src=\"_equ#0#4#0#4#smquant2(x,1)\"/>";

    if (f->name() == "beta" && f->num_param() == 1)
        return tr("Calculates a circle's surface for input <i>a</i> in the range [-1,1]. "
                  "For example <i>a</i> could be the x coordinate and the result would be "
                  "the y coordinate of the circumference of a circle of radius 1 centered at "
                  "the origin.<br/>The underlying equation is: <b>beta(x) = sqrt(1-x*x)</b>")
                  +"<br/><b>beta(x)</b>:<br/><img src=\"_equ#-1#1#0#1#beta(x)#200#100\"/>";
    if (f->name() == "beta" && f->num_param() == 2)
        return tr("Calculates a sphere's surface for input range [-1,1]. "
                  "For example <i>a</i> and <i>b</i> could be the x and y coordinates and the "
                  "result would be the z coordinate of the periphery of a sphere of radius 1 "
                  "centered at the origin."
                  "<br/>The underlying equation is: <b>beta(x,y) = sqrt(1-x*x-y*y)</b>");
    if (f->name() == "beta" && f->num_param() == 3)
        return tr("Calculates a 4-dimensional sphere's surface for input range [-1,1]."
                  "<br/>The underlying equation is: <b>beta(x,y,z) = sqrt(1-x*x-y*y-z*z)</b>");
    if (f->name() == "beta" && f->num_param() == 4)
        return tr("Calculates a 5-dimensional sphere's surface for input range [-1,1]."
                  "<br/>The underlying equation is: <b>beta(x,y,z,w) = sqrt(1-x*x-y*y-z*z-w*w)</b>");

    if (f->name() == "mag" && f->num_param() == 2)
        return tr("Returns the magnitude or length of the 2d-vector (<i>a</i>, <i>b</i>)");
    if (f->name() == "mag" && f->num_param() == 3)
        return tr("Returns the magnitude or length of the 3d-vector (<i>a</i>, <i>b</i>, <i>c</i>)");
    if (f->name() == "mag" && f->num_param() == 4)
        return tr("Returns the magnitude or length of the 4d-vector "
                  "(<i>a</i>, <i>b</i>, <i>c</i>, <i>d</i>)");
    if (f->name() == "dist")
        return tr("Returns the distance between the two 2d-coordinates"
                  "(<i>a</i>, <i>b</i>) and (<i>c</i>, <i>d</i>)");

    if (f->name() == "rotate")
        return tr("Rotates the 2d-coordinate (<i>a</i>, <i>b</i>) around the origin by the angle "
                  "<i>c</i> in radians and returns the first component of the rotated vector."
                  "A common rotation would be:<br/>")
                  +"<b>x' = rotate(x, y, degree)<br/>"
                      "y' = rotate(y, x, -degree)</b>";
    if (f->name() == "rotater")
        return tr("Rotates the 2d-coordinate (<i>a</i>, <i>b</i>) around the origin by the angle "
                  "<i>c</i> in radians and returns the first component of the rotated vector."
                  "A common rotation can be expressed as:")
                  +"<b>x' = rotater(x, y, degree / 180 * PI)<br/>"
                      "y' = rotater(y, x, -degree / 180 * PI)</b>";

    if (f->name() == "ramp")
        return tr("A ramp oscillator, output range [0,1]")
                  +"<br/><b>ramp(x)</b>:<br/><img src=\"_equ#-1#1#-1#1#ramp(x)\"/>";
    if (f->name() == "saw")
        return tr("A sawtooth oscillator, output range [-1,1]")
                  +"<br/><b>saw(x)</b>:<br/><img src=\"_equ#-1#1#-1#1#saw(x)\"/>";
    if (f->name() == "square" && f->num_param() == 1)
        return tr("A square-wave oscillator, output range [-1,1]")
                  +"<br/><b>square(x)</b>:<br/><img src=\"_equ#-1#1#-1#1#square(x)\"/>";
    if (f->name() == "square" && f->num_param() == 2)
        return tr("A square-wave oscillator with pulse-width control <i>b</i>, output range [-1,1]")
                  +"<br/><b>square(x, 0.1)</b>:<br/><img src=\"_equ#-1#1#-1#1#square(x,0.1)\"/>";
    if (f->name() == "tri" && f->num_param() == 1)
        return tr("A triangle-wave oscillator, output range [-1,1]")
                  +"<br/><b>tri(x)</b>:<br/><img src=\"_equ#-1#1#-1#1#tri(x)\"/>";
    if (f->name() == "tri" && f->num_param() == 2)
        return tr("A triangle-wave oscillator with pulse-width control <i>b</i>, output range [-1,1]")
                  +"<br/><b>tri(x, 0.1)</b>:<br/><img src=\"_equ#-1#1#-1#1#tri(x,0.1)\"/>";

    if (f->name() == "rnd")
        return tr("Returns a pseudo-random number in the range [0,1]. "
                  "<br/>A new random number is generated on each call of the function. "
                  "Generally it's highly experimental to use functions like this in objects "
                  "that support equations. If you need \"predictable\" randomness, "
                  "use <b>noise</b> instead.");
    if (f->name() == "noise" && f->num_param() == 1)
        return tr("Returns a 1-dimensional smoothed pseudo-random number in the range [-1,1].")
                  +"<br/><b>noise(x)</b>:<br/><img src=\"_equ#-4#4#-1#1#noise(x)\"/>";
    if (f->name() == "noise" && f->num_param() == 2)
        return tr("Returns a 2-dimensional smoothed pseudo-random number in the range [-1,1].")
                  +"<br/><b>noise(x, 0)</b>:<br/><img src=\"_equ#-4#4#-1#1#noise(x,0)\"/>";
    if (f->name() == "noise" && f->num_param() == 3)
        return tr("Returns a 3-dimensional smoothed pseudo-random number in the range [-1,1].")
                  +"<br/><b>noise(x, 0, 0)</b>:<br/><img src=\"_equ#-4#4#-1#1#noise(x,0,0)\"/>";
    if (f->name() == "noiseoct" && f->num_param() == 2)
        return tr("Returns a 1-dimensional smoothed pseudo-random number in the range about [-2,2]."
                  "The noise function is octaved, meaning there are <i>b</i> number of "
                  "noises mixed together, each with double the frequency and half the amplitude. "
                  "The number of octaves is limited to 10.")
                  +"<br/><b>noise(x, 4)</b>:<br/><img src=\"_equ#-4#4#-1#1#noiseoct(x,4)\"/>";
    if (f->name() == "noiseoct" && f->num_param() == 3)
        return tr("Returns a 2-dimensional smoothed pseudo-random number in the range about [-2,2]."
                  "The noise function is octaved, meaning there are <i>c</i> number of "
                  "noises mixed together, each with double the frequency and half the amplitude. "
                  "The number of octaves is limited to 10.")
                  +"<br/><b>noise(x, 0, 4)</b>:<br/><img src=\"_equ#-4#4#-1#1#noiseoct(x,0,4)\"/>";
    if (f->name() == "noiseoct" && f->num_param() == 4)
        return tr("Returns a 3-dimensional smoothed pseudo-random number in the range about [-2,2]."
                  "The noise function is octaved, meaning there are <i>d</i> number of "
                  "noises mixed together, each with double the frequency and half the amplitude. "
                  "The number of octaves is limited to 10.")
                  +"<br/><b>noise(x, 0, 0, 4)</b>:<br/><img src=\"_equ#-4#4#-1#1#noiseoct(x,0,0,4)\"/>";

    if (f->name() == "fac")
        return tr("Returns the factorial of <i>a</i>. E.g. the factorial of 7 is calculated as:")
                  +" 1 * 2 * 3 * 4 * 5 * 6 * 7";
    if (f->name() == "fib")
        return tr("Returns the <i>a</i>th fibonacci number. Fibonacci numbers are defined as:")
                  + "<br/>fib(n) = fib(n-2) + fib(n-1); where fib(0) and fib(1) are 1";

    if (f->name() == "zeta" && f->num_param() == 1)
        return tr("Approximates the Riemann zeta function of <i>a</i>");
    if (f->name() == "zeta" && f->num_param() == 2)
        return tr("Approximates the Riemann zeta function of <i>a</i> until the change per "
                  "iteration is equal or lower than <i>b</i>, or the number of iterations "
                  "reaches 200000");

    if (f->name() == "harmo" && f->num_param() == 2)
        return tr("Returns the harmonic quotient of <i>a</i>/<i>b</i> or <i>b</i>/<i>a</i>. "
                  "If neither of the terms results in an integer, the result is 0.");

    if (f->name() == "harmo" && f->num_param() == 3)
        return tr("Returns the harmonic quotient of <i>a</i>/<i>b</i>/<i>c</i> or "
                  "any permutation of the order. "
                  "If neither of the terms results in an integer, the result is 0.");

    if (f->name() == "prime")
        return tr("Returns 1 if <i>a</i> is a prime number and 0 otherwise. "
                  "The first about million numbers have a look-up table for efficient "
                  "execution.")
                + "<br/><b>prime(x)</b>:<br/><img src=\"_equ#0#100#0#1#prime(x)\"/>"; ;

    if (f->name() == "quer")
        return tr("Returns the cross sum of the integer number <i>a</i>. The cross sum "
                  "is the sum of all digits of a number, in this case in the base 10 system. "
                  "When the sum is calculated, the cross sum is taken again until only one "
                  "digit remains.")
                + "<br/><b>quer(x)</b>:<br/><img src=\"_equ#0#100#0#20#quer(x)\"/>"; ;

    if (f->name() == "uspiral" && f->num_param() == 2)
        return tr("Returns the ulam spiral number for the integer coordinate (<i>a</i>, <i>b</i>)."
                  "The ulam spiral lives in the integer grid, starts at 1 and extends "
                  "counter-clockwise in square spirals.")
                + "<br/><img src=\"ulam_spiral.png\"/>";

    if (f->name() == "uspiral" && f->num_param() == 3)
        return tr("Returns the ulam spiral number for the integer coordinate (<i>a</i>, <i>b</i>) "
                  "with additional width parameter <i>c</i>.");

    if (f->name() == "tspiral" && f->num_param() == 2)
        return tr("Returns the triangle spiral number for the integer coordinate (<i>a</i>, <i>b</i>)."
                  "The triangle spiral lives in the integer grid, starts at 1 and extends "
                  "counter-clockwise in triangles around itself. "
                  "<br/>Note that not all positions are defined for the triangle spiral. These "
                  "undefined positions return 0.");

    if (f->name() == "ndiv")
        return tr("Returns the number of divisors of the integer <i>a</i>."
                  "The first about million numbers have a look-up table for efficient "
                  "execution.")
                + "<br/><b>ndiv(x)</b>:<br/><img src=\"_equ#0#100#0#20#ndiv(x)\"/>";
    if (f->name() == "divisor")
        return tr("Returns the <i>b</i>th divisor of the integer <i>a</i>."
                  "The first about 260,000 numbers have a look-up table for efficient "
                  "execution.")
                + "<br/><b>divisor(x,3)</b>:<br/><img src=\"_equ#0#100#0#20#divisor(x,3)\"/>";
    if (f->name() == "sumdiv")
        return tr("Returns the sum of all the divisors of the integer <i>a</i>."
                  "The first about 260,000 numbers have a look-up table for efficient "
                  "execution.")
                + "<br/><b>sumdiv(x)</b>:<br/><img src=\"_equ#0#100#0#100#sumdiv(x)\"/>";
    if (f->name() == "proddiv")
        return tr("Returns the product of all the divisors of the integer <i>a</i>."
                  "The first about 260,000 numbers have a look-up table for efficient "
                  "execution.")
                + "<br/><b>proddiv(x)</b>:<br/><img src=\"_equ#0#100#0#10000#proddiv(x)\"/>";
    if (f->name() == "nextdiv")
        return tr("Returns the next devisor of the integer <i>a</i>, starting at <i>b</i>."
                  "It does not matter if <i>b</i> is actually a divisor of <i>a</i>."
                  "There is no look-up table and this function might be relatively slow.");
    if (f->name() == "gcd")
        return tr("Returns the greatest common divisor of the integers <i>a</i> and <i>b</i>."
                  "There is no look-up table and this function might be relatively slow.");
    if (f->name() == "cong")
        return tr("Returns 1 if the integers <i>a</i> and <i>b</i> or cogruent in the "
                  "modulo space of <i>c</i> and 0 otherwise. The calculation is: "
                  "<br/><b>cong(a,b,m) = ((b-a) modulo m) equals 0</b>");
    if (f->name() == "digits")
        return tr("Returns the number of digits of the integer <i>a</i> in the base 10 system.");

    if (f->name() == "ndiv_s")
        return tr("This is the smoothed version of <a href=\"#ndiv1\">ndiv</a>")
                + "<br/><b>ndiv_s(x)</b>:<br/><img src=\"_equ#0#100#0#20#ndiv_s(x)\"/>";
    if (f->name() == "divisor_s")
        return tr("This is the smoothed version of <a href=\"#divisor2\">divisor</a>")
                + "<br/><b>divisor_s(x,3)</b>:<br/><img src=\"_equ#0#100#0#20#divisor_s(x,3)\"/>";
    if (f->name() == "sumdiv_s")
        return tr("This is the smoothed version of <a href=\"#sumdiv1\">sumdiv</a>")
                + "<br/><b>sumdiv_s(x)</b>:<br/><img src=\"_equ#0#100#0#100#sumdiv_s(x)\"/>";
    if (f->name() == "proddiv_s")
        return tr("This is the smoothed version of <a href=\"#proddiv1\">proddiv</a>")
                + "<br/><b>proddiv_s(x)</b>:<br/><img src=\"_equ#0#100#0#10000#proddiv_s(x)\"/>";
    if (f->name() == "nextdiv_s")
        return tr("This is the smoothed version of <a href=\"#nextdiv2\">nextdiv</a>");
    if (f->name() == "gcd_s")
        return tr("This is the smoothed version of <a href=\"#gcd2\">gcd</a>");
    if (f->name() == "cong_s")
        return tr("This is the smoothed version of <a href=\"#cong3\">cong</a>");
    if (f->name() == "digits_s")
        return tr("This is the smoothed version of <a href=\"#digits1\">digits</a>");

    if (f->name() == "mandel" && f->num_param() == 2)
        return tr("Returns the approximation of the Mandelbrot function. "
                  "The input variables are the real (<i>a</i>) and imaginary (<i>b</i>) part "
                  "of the equation: <b>z(n+1) = z(n) ^ 2 + c</b>; where z and c are complex numbers. "
                  "z is initially zero and c is initialized to the input variables. The "
                  "result of the function is the real part of z after a maximum of 1000 iterations.")
                + "<br/><b>mandel(x,1)</b>:<br/><img src=\"_equ#-2#2#0#6#mandel(x,1)\"/>";

    if (f->name() == "mandel" && f->num_param() == 3)
        return tr("Sames as <a href=\"#mandel2\">mandel(a,b)</a> but with the number of "
                  "maximum iterations given in <i>c</i>");

    if (f->name() == "mandeli" && f->num_param() == 2)
        return tr("Returns the number of iterations after which z in the mandelbrot function "
                  "exceeds the limit of sqrt(-2). The maximum iteration is fixed to 1000.")
                + "<br/><b>mandeli(x,1)</b>:<br/><img src=\"_equ#-2#2#0#200#mandeli(x,1)\"/>";
    if (f->name() == "mandeli" && f->num_param() == 3)
        return tr("Sames as <a href=\"#mandeli2\">mandeli(a,b)</a> but with the number of "
                  "maximum iterations given in <i>c</i>");

    return "";
}


} // namespace GUI
} // namespace MO
