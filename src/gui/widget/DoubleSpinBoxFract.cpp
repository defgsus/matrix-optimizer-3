/** @file doublespinboxclean.cpp

    @brief QDoubleSpinBox without the zero fill-ins

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#include <QLayout>
#include <QLineEdit>
#include <QLabel>
#include <QResizeEvent>

#include "DoubleSpinBoxFract.h"
#include "math/functions.h"

#include "io/log.h"
#define MO__D(arg__) MO_PRINT(::MO::applicationTimeString() \
    << ": DoubleSpinBox(" << this << "): " << arg__)

namespace MO {
namespace GUI {

struct DoubleSpinBoxFract::Private
{
    Private(DoubleSpinBoxFract * p)
        : p             (p)
        , value         (0.)
        , isFractional  (false)
        , isInputValid  (true)
        , ignoreChange  (false)
        , decimals      (4)
        , lineEdit      (nullptr)
        , label         (nullptr)
        , layout        (nullptr)
        , minimum       (0)
        , maximum       (100)
        , singleStep    (1)
    { }

    void createWidgets();
    bool hasFocus() const { return p->hasFocus(); }
    void textToValue(const QString& text, bool send);
    void valueToText();
    void setValue(double, bool send);
    void setValue(int64_t n, int64_t d, bool send);

    DoubleSpinBoxFract* p;

    double value;
    bool isFractional, isInputValid, ignoreChange;
    MATH::Fraction fraction;
    int decimals;

    QLineEdit* lineEdit;
    QLabel* label;
    QHBoxLayout* layout;
    double minimum,
           maximum,
           singleStep;
    QString prefix, suffix;
};

DoubleSpinBoxFract::DoubleSpinBoxFract(QWidget *parent)
    : QWidget           (parent)
    , p_                (new Private(this))
{
    p_->createWidgets();
    p_->valueToText();
}

void DoubleSpinBoxFract::Private::createWidgets()
{
    layout = new QHBoxLayout(p);
    layout->setMargin(0);

        lineEdit = new QLineEdit(p);
        layout->addWidget(lineEdit);
        p->setFocusProxy(lineEdit);

        connect(lineEdit, &QLineEdit::textChanged, [=]()
        {
            if (ignoreChange)
                return;
            MO__D("textChanged(" << lineEdit->text() << ")");
            textToValue(lineEdit->text(), true);
        });
}

bool DoubleSpinBoxFract::isFractional() const { return p_->isFractional; }

void DoubleSpinBoxFract::setFractional(bool enable)
{
    MO__D("setFractional(" << enable << ")");
    if (enable == p_->isFractional)
        return;
    p_->isFractional = enable;
}

QLineEdit* DoubleSpinBoxFract::lineEdit() const { return p_->lineEdit; }
void DoubleSpinBoxFract::setMinimum(double min) { p_->minimum = min; }
void DoubleSpinBoxFract::setMaximum(double max) { p_->maximum = max; }
void DoubleSpinBoxFract::setRange(double minimum, double maximum)
    { p_->minimum = minimum, p_->maximum = maximum; }
void DoubleSpinBoxFract::setDecimals(int prec) { p_->decimals = prec; }
void DoubleSpinBoxFract::setSingleStep(double val) { p_->singleStep = val; }
void DoubleSpinBoxFract::setPrefix(const QString& prefix) { p_->prefix = prefix; }
void DoubleSpinBoxFract::setSuffix(const QString& suffix) { p_->suffix = suffix; }

double  DoubleSpinBoxFract::minimum() const { return p_->minimum; }
double  DoubleSpinBoxFract::maximum() const { return p_->maximum; }
int     DoubleSpinBoxFract::decimals() const { return p_->decimals; }
double  DoubleSpinBoxFract::singleStep() const { return p_->singleStep; }
QString DoubleSpinBoxFract::prefix() const { return p_->prefix; }
QString DoubleSpinBoxFract::suffix() const { return p_->suffix; }
const MATH::Fraction& DoubleSpinBoxFract::fraction() const { return p_->fraction; }

double  DoubleSpinBoxFract::value() const
{
    return isFractional() ? p_->fraction.value() : p_->value;
}

void DoubleSpinBoxFract::setLabel(const QString& s)
{
    if (!p_->label)
    {
        if (s.isEmpty())
            return;
        p_->label = new QLabel(this);
        p_->layout->insertWidget(0, p_->label);
    }
    p_->label->setText(s);
}

void DoubleSpinBoxFract::setValue(double v, bool sendSignal)
{
    MO__D("setValue(" << v << ")");
    p_->setValue(v, sendSignal);
    p_->valueToText();
}

void DoubleSpinBoxFract::setValue(const MATH::Fraction& f, bool sendSignal)
{
    MO__D("setValue(" << f.toString() << ")");

    p_->setValue(f.nom, f.denom, sendSignal);
    p_->valueToText();
}

void DoubleSpinBoxFract::Private::setValue(double v, bool send)
{
    double old = value;
    value = std::max(minimum, std::min(maximum, v ));
    isFractional = false;
    MO__D("value = " << value);
    if (send && old != value)
        emit p->valueChanged(value);
}

void DoubleSpinBoxFract::Private::setValue(int64_t n, int64_t d, bool send)
{
    fraction.nom = n;
    fraction.denom = d;
    isFractional = true;
    double v = d ? double(n) / d : 0., old = value;
    value = std::max(minimum, std::min(maximum, v ));
    MO__D("value = " << value << " <- " << n << "/" << d);
    if (send && old != value)
        emit p->valueChanged(value);
}

void DoubleSpinBoxFract::Private::textToValue(const QString &text, bool send)
{
    MO__D("textToValue(" << text << ")");

    if (!text.contains("/"))
    {
        double v = text.toDouble(&isInputValid);
        if (isInputValid)
            setValue(v, send);
        return;
    }

    auto s = text.split('/', QString::SkipEmptyParts);
    if (s.size() < 1)
    {
        isInputValid = false;
        return;
    }

    int n = s[0].toInt(&isInputValid);
    if (!isInputValid)
        return;

    if (s.size() < 2)
        return;

    int d = s[1].toInt(&isInputValid);
    if (!isInputValid)
        return;

    setValue(n, d, send);
}

void DoubleSpinBoxFract::Private::valueToText()
{

    QString txt;
    if (!isFractional)
        txt = QString::number(value);
    else
        txt = fraction.toString();

    MO__D("valueToText() -> '" << txt << "'");
    ignoreChange = true;
    lineEdit->setText(txt);
    ignoreChange = false;
/*
    double i = std::floor(val), f = val - i;
    if (std::abs(f) < 1e-10)
        return QString::number(val);
    double n = val / f,
           d = 1. / f;
    int cn = 0;
    while (cn++ < 5)
    {
        f = MATH::fract(d);
        if (f < 1e-10)
            break;
        n *= f, d *= f;
    }
    return QString("%1/%2")
            .arg(val / f)
            .arg(1. / f);
*/
}

/*
void DoubleSpinBoxFract::keyPressEvent(QKeyEvent* event)
{
    MO__D("keyPress()");
    QDoubleSpinBox::keyPressEvent(event);

    QString txt = text();
    MO__D("text now '" << txt << "'");
    bool hasDot = txt.contains("."),
         hasSlash = txt.contains("/");
    if (hasDot && !hasSlash)
        setFractional(false);
    if (hasSlash && !hasDot)
        setFractional(true);
}

QString DoubleSpinBoxFract::textFromValue(double val) const
{
    MO__D("textFromValue(" << val << ")");

    if (!isFractional() && !text().contains("/"))
        return QString::number(val);

    double i = std::floor(val), f = val - i;
    if (std::abs(f) < 1e-10)
        return QString::number(val);
    double n = val / f,
           d = 1. / f;
    int cn = 0;
    while (cn++ < 5)
    {
        f = MATH::fract(d);
        if (f < 1e-10)
            break;
        n *= f, d *= f;
    }
    return QString("%1/%2")
            .arg(val / f)
            .arg(1. / f);
}


QValidator::State DoubleSpinBoxFract::validate(QString &input, int &pos) const
{
    MO__D("validate(" << input << ", " << pos << ")");

    auto s = QAbstractSpinBox::validate(input, pos);
    if (s == QValidator::Acceptable)
        return s;

    return QValidator::Acceptable;
}
*/

} // namespace GUI
} // namespace MO
