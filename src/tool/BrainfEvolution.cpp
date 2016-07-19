/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/19/2016</p>
*/

#include <QImage>
#include <QPainter>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>

#include "BrainfEvolution.h"
#include "tool/Brainf.h"
#include "types/Properties.h"
#include "math/random.h"
#include "io/error.h"

namespace MO {

MO_REGISTER_EVOLUTION(BrainfEvolution)

BrainfEvolution::BrainfEvolution(size_t cs, size_t is, size_t mcs, size_t mis)
    : EvolutionBase     ()
    , p_codeSize_       (cs)
    , p_inputSize_      (is)
    , p_maxCodeSize_    (mcs)
    , p_maxInputSize_   (mis)
    , p_isInputLoop_    (false)
{
    properties().set("num_cycles",
          QObject::tr("max. cycles"),
          QObject::tr("Maximum number of cycles spend on code"),
          10000u, 100u);

    properties().set("init_code_length",
          QObject::tr("init code length"),
          QObject::tr("The length of the code on creation"),
          (unsigned)p_codeSize_);
    properties().setMin("init_code_length", (unsigned)1);
    if (p_maxCodeSize_ > 0)
        properties().setMax("init_code_length", (unsigned)p_maxCodeSize_);

    properties().set("init_input_length",
          QObject::tr("init input length"),
          QObject::tr("The length of the input on creation"),
          (unsigned)p_inputSize_);
    properties().setMin("init_input_length", (unsigned)1);
    if (p_maxInputSize_ > 0)
        properties().setMax("init_input_length", (unsigned)p_maxInputSize_);

    properties().set("mutation_prob_code",
              QObject::tr("mutation probability (code)"),
              QObject::tr("Probability of a random change to the code"),
              0.1, 0., 1., 0.025);

    properties().set("mutation_prob_input",
              QObject::tr("mutation probability (input)"),
              QObject::tr("Probability of a random change to the input"),
              0.1, 0., 1., 0.025);

    properties().set("grow_prob_code",
          QObject::tr("grow probability (code)"),
          QObject::tr("Probability of a random insert to the code"),
          0.01, 0., 1., 0.01);
    properties().set("shrink_prob_code",
          QObject::tr("shrink probability (code)"),
          QObject::tr("Probability of a random deletion from code"),
          0.01, 0., 1., 0.01);

    properties().set("grow_prob_input",
          QObject::tr("grow probability (input)"),
          QObject::tr("Probability of a random insert to the input"),
          0.01, 0., 1., 0.01);
    properties().set("shrink_prob_input",
          QObject::tr("shrink probability (input)"),
          QObject::tr("Probability of a random deletion from input"),
          0.01, 0., 1., 0.01);

}

void BrainfEvolution::copyFrom(const EvolutionBase* o)
{
    EvolutionBase::copyFrom(o);
    if (auto e = dynamic_cast<const BrainfEvolution*>(o))
    {
        p_code_ = e->p_code_;
        p_input_ = e->p_input_;
        p_codeSize_ = e->p_codeSize_;
        p_inputSize_ = e->p_inputSize_;
        p_maxCodeSize_ = e->p_maxCodeSize_;
        p_maxInputSize_ = e->p_maxInputSize_;
        p_isInputLoop_ = e->p_isInputLoop_;
    }
}

void BrainfEvolution::serialize(QJsonObject& o) const
{
    o.insert("version", 1);
    QJsonArray acode, ainput;
    for (auto v : code())
        acode.append( (int)v );
    for (auto v : input())
        ainput.append( (int)v );
    o.insert("brainf", acode);
    o.insert("input", ainput);
    o.insert("do_loop_input", p_isInputLoop_);
}

void BrainfEvolution::deserialize(const QJsonObject& o)
{
    auto acode = o.find("brainf"),
         ainput = o.find("input");
    if (acode == o.end())
        MO_IO_ERROR(VERSION_MISMATCH, "Missing 'brainf' in json evolution");
    if (!acode->isArray())
        MO_IO_ERROR(VERSION_MISMATCH, "'brainf' of wrong type in json evolution");
    if (ainput == o.end())
        MO_IO_ERROR(VERSION_MISMATCH, "Missing 'input' in json evolution");
    if (!ainput->isArray())
        MO_IO_ERROR(VERSION_MISMATCH, "'input' of wrong type in json evolution");
    {
        auto a = acode->toArray();
        code().resize(a.size());
        for (size_t i=0; i<code().size(); ++i)
            code(i) = Brainf_traits<Int>::toOpcode( a[i].toInt() );
    }
    {
        auto a = ainput->toArray();
        input().resize(a.size());
        for (size_t i=0; i<input().size(); ++i)
            input(i) = a[i].toInt();
    }
    p_isInputLoop_ = o.value("do_loop_input").toBool();
}


void BrainfEvolution::getImage(QImage &img) const
{
    Brainf_uint8 brainf(16, 16);

    brainf.setCode(code());
    brainf.setInput(input());

    brainf.run(properties().get("num_cycles").toUInt());


    QPainter p;
    p.begin(&img);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.setPen(Qt::NoPen);
    p.fillRect(img.rect(), Qt::black);

    const int resX = std::max(1, (int)std::sqrt(brainf.output().size())),
              resY = resX;

    double w = std::max(1., double(img.width()) / resX),
           h = std::max(1., double(img.height()) / resY);
    for (int j=0; j<resY; ++j)
    for (int i=0; i<resX; ++i)
    if (j*resX+i < (int)brainf.output().size())
    {
        uint8_t v = brainf.output()[j * resX + i];
        p.setBrush(v == 0 ? QColor(0, 0, 0) : QColor(
                                int(255*(.5+.5*sin(.1*v))),
                                int(255*(.5+.5*cos(.13*v))),
                                int(255*(.5-.5*sin(.161*v)))
                                ));
        p.drawRect(QRectF(double(i)/resX * img.width() ,
                          double(j)/resY * img.height() , w, h));
    }

    p.end();
}


QString BrainfEvolution::toString() const
{
    Brainf<Int> brainf;
    brainf.setCode(code());
    brainf.setInput(input());
    brainf.run(properties().get("num_cycles").toUInt());
    return "code: " + QString::fromStdString( brainf.codeString() )
         + "\ninput: " + QString::fromStdString( brainf.inputStringNum() )
            + (p_isInputLoop_ ? " (loop)" : "")
         + "\noutput: " + QString::fromStdString( brainf.outputStringNum() );
}


BrainfOpcode BrainfEvolution::randomOpcode(MATH::Twister& rnd)
{
    return (BrainfOpcode)(rnd.getUInt32() % 8 + 1);
}

void BrainfEvolution::makeSaneCode(std::vector<BrainfOpcode>& code) const
{
    if (code.empty())
        return;

    // remove leading tape moves
    int i=0;
    while (i < (int)code.size() &&
           (code[i] == BFO_LEFT || code[i] == BFO_RIGHT))
        ++i;
    if (i>0)
        code.erase(code.begin(), code.begin() + i);

    if (code.size() < 2)
        return;

    // keep everything up to last '.'
    i = code.size() - 1;
    for (; i > 0; --i)
        if (code[i] == BFO_OUT)
            break;
    if (i > 0)
    {
        // but include trailing ']'
        int j=i;
        for (; j<int(code.size()); ++j)
            if (code[j] == BFO_END)
                i = j;

        code.resize(std::min(code.size(), size_t(i + 1)));
    }

    if (code.size() < 2)
        return;

    // sanitise brackets
    // ..[...]]..[..]]
    int numOpen = 0, numClose = 0;
    for (size_t i = 0; i < code.size(); )
    {
        if (code[i] == BFO_END)
        {
            if (numOpen <= numClose)
                { code.erase(code.begin()+i); continue; }

            ++numClose;
        }
        if (code[i] == BFO_BEGIN)
        {
            ++numOpen;
        }
        ++i;
    }
    while (numClose < numOpen)
    {
        ++numClose;
        code.push_back(BFO_END);
    }

    if (code.size() < 2)
        return;

    // remove obvious two-char noops
    for (size_t i = 0; i < code.size(); )
    {
        if (i < code.size() - 1)
        {
            auto nextop = code[i+1];
            if ((code[i] == BFO_INC && nextop == BFO_DEC)
             || (code[i] == BFO_DEC && nextop == BFO_INC)
             || (code[i] == BFO_RIGHT && nextop == BFO_LEFT)
             || (code[i] == BFO_LEFT && nextop == BFO_RIGHT)
             || (code[i] == BFO_BEGIN && nextop == BFO_END)
                )
            {
                code.erase(code.begin() + i, code.begin() + i + 2);
                continue;
            }
        }

        ++i;
    }
}

void BrainfEvolution::randomize()
{
    MATH::Twister rnd(properties().get("seed").toUInt());
/*    double  mean = properties().get("init_mean").toDouble(),
            var = properties().get("init_var").toDouble(),
            dev = 1./properties().get("init_dev").toDouble();
*/
    p_isInputLoop_ = rnd() < .5;

    code().resize(properties().get("init_code_length").toUInt());
    for (auto& c : code())
        c = randomOpcode(rnd);
    makeSaneCode(code());

    input().resize(properties().get("init_input_length").toUInt());
    for (auto& i : input())
        i = rnd.getUInt32();
}

void BrainfEvolution::mutate()
{
    MATH::Twister rnd(properties().get("seed").toUInt());
    double  prob_code  = properties().get("mutation_prob_code").toDouble(),
            prob_input = properties().get("mutation_prob_input").toDouble(),
            grow_code = properties().get("grow_prob_code").toDouble(),
            shrink_code = properties().get("shrink_prob_code").toDouble(),
            grow_input = properties().get("grow_prob_input").toDouble(),
            shrink_input = properties().get("shrink_prob_input").toDouble();

    for (auto& v : code())
        if (rnd() < prob_code)
            v = randomOpcode(rnd);

    for (auto& v : input())
        if (rnd() < prob_input)
            v = rnd.getUInt32();

    if (rnd() < prob_input)
        p_isInputLoop_ = (rnd.getUInt32() >> 7) & 1;

    // grow code
    for (size_t i=0; i<code().size()
                     && (p_maxCodeSize_ == 0 || code().size() < p_maxCodeSize_);
         ++i)
    if (rnd() < grow_code)
    {
        code().insert(code().begin() + i, randomOpcode(rnd));
    }
    // shrink
    for (size_t i=0; i<code().size() && code().size() > 1; ++i)
    if (rnd() < shrink_code)
        code().erase(code().begin() + i);
    makeSaneCode(code());

    // grow input
    for (size_t i=0; i<input().size()
                     && (p_maxInputSize_ == 0 || input().size() < p_maxInputSize_);
         ++i)
    if (rnd() < grow_input)
    {
        input().insert(input().begin() + i, rnd.getUInt32());
    }
    // shrink
    for (size_t i=0; i<input().size() && input().size() > 1; ++i)
    if (rnd() < shrink_input)
        input().erase(input().begin() + i);
}

bool BrainfEvolution::isCompatible(const EvolutionBase* o) const
{
    return dynamic_cast<const BrainfEvolution*>(o);
}

void BrainfEvolution::mate(const EvolutionBase* otherBase)
{
    /*
    auto other = dynamic_cast<const BrainfEvolution*>(otherBase);
    if (!other)
        return;

    MATH::Twister rnd(properties().get("seed").toUInt());
    double range = std::pow(rnd(0.01, 1.), 1.36) * 2. * vector().size(),
           phase = rnd(0., 6.28),
           amp = rnd() * rnd();
    size_t num = std::min(vector().size(), other->vector().size());

    for (size_t i=0; i<num; ++i)
    {
        double v1 = vector(i),
               v2 = other->vector(i),
               mx = amp * std::cos(double(i)/num * 3.14159265 * range + phase);

        p_vec_[i] = v1 + mx * (v2 - v1);
    }
    */
}



} // namespace MO

