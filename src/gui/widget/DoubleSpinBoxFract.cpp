/** @file doublespinboxclean.cpp

    @brief QDoubleSpinBox without the zero fill-ins

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#include <QLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QWheelEvent>

#include "DoubleSpinBoxFract.h"
#include "math/functions.h"

#if 0
#   include "io/log.h"
#   define MO__D(arg__) MO_PRINT(::MO::applicationTimeString() \
        << ": DoubleSpinBox(" << this << "): " << arg__)
#else
#   define MO__D(unused__) { }
#endif

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
    void updateToolTips();
    void setFractional(bool enable);
    void setValid(bool valid);
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
    QToolButton *butUp, *butDown;
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
    setStatusTip(tr("Edit with keyboard, scroll with mouse-wheel or "
                    "use the up/down buttons"));
    p_->updateToolTips();
}

void DoubleSpinBoxFract::Private::createWidgets()
{
    layout = new QHBoxLayout(p);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

        lineEdit = new QLineEdit(p);
        lineEdit->setMaximumWidth(140);
        layout->addWidget(lineEdit);
        p->setFocusProxy(lineEdit);

        connect(lineEdit, &QLineEdit::textChanged, [=]()
        {
            if (ignoreChange)
                return;
            MO__D("textChanged(" << lineEdit->text() << ")");
            textToValue(lineEdit->text(), true);
        });

        int h = lineEdit->height() * .7,
            w = h * .8;
        auto but = butUp = new QToolButton(p);
        but->setFixedSize(w, h);
        but->setIcon(QIcon(":/icon/small_up.png"));
        layout->addWidget(but);
        connect(but, &QToolButton::clicked, [=](){ p->step(1, true); });

        but = butDown = new QToolButton(p);
        but->setFixedSize(w, h);
        but->setIcon(QIcon(":/icon/small_down.png"));
        layout->addWidget(but);
        connect(but, &QToolButton::clicked, [=](){ p->step(-1, true); });

}

bool DoubleSpinBoxFract::isFractional() const { return p_->isFractional; }

void DoubleSpinBoxFract::Private::setFractional(bool enable)
{
    MO__D("setFractional(" << enable << ")");
    isFractional = enable;
    updateToolTips();
}

void DoubleSpinBoxFract::Private::setValid(bool valid)
{
    MO__D("setValid(" << valid << ")");
    isInputValid = valid;

    auto f = lineEdit->font();
    f.setUnderline(!isInputValid);
    lineEdit->setFont(f);
    if (!isInputValid)
        lineEdit->setToolTip(tr("invalid input"));
}

QLineEdit* DoubleSpinBoxFract::lineEdit() const { return p_->lineEdit; }
void DoubleSpinBoxFract::setMinimum(double min) { p_->minimum = min; }
void DoubleSpinBoxFract::setMaximum(double max) { p_->maximum = max; }
void DoubleSpinBoxFract::setRange(double minimum, double maximum)
    { p_->minimum = minimum, p_->maximum = maximum; }
void DoubleSpinBoxFract::setDecimals(int prec) { p_->decimals = prec; }
void DoubleSpinBoxFract::setSingleStep(double val)
    { p_->singleStep = val; p_->updateToolTips(); }
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

void DoubleSpinBoxFract::Private::updateToolTips()
{
    MO__D("updateToolTips() valid=" << isInputValid << ", frac=" << isFractional);

    if (isInputValid)
        lineEdit->setToolTip(QString::number(value));

    if (isFractional)
    {
        butUp->setToolTip(tr("increase denominator"));
        butDown->setToolTip(tr("decrease denominator"));
    }
    else
    {
        butUp->setToolTip(tr("increase by %1").arg(singleStep));
        butDown->setToolTip(tr("decrease by %1").arg(singleStep));
    }
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
    MO__D("value = " << value);
    setFractional(false);
    if (send && old != value)
        emit p->valueChanged(value);
}

void DoubleSpinBoxFract::Private::setValue(int64_t n, int64_t d, bool send)
{
    fraction.nom = n;
    fraction.denom = std::abs(d);
    double v = d ? double(n) / d : 0., old = value;
    value = std::max(minimum, std::min(maximum, v ));
    MO__D("value = " << value << " <- " << n << "/" << d);
    setFractional(true);
    if (send && old != value)
        emit p->valueChanged(value);
}

void DoubleSpinBoxFract::Private::textToValue(const QString &text2, bool send)
{
    MO__D("textToValue(" << text2 << ")");

    QString text = text2;
    if (!prefix.isEmpty() && text.startsWith(prefix))
        text.remove(prefix + " ");
    if (!suffix.isEmpty() && text.endsWith(suffix))
        text.remove(" " + suffix);

    if (!text.contains("/"))
    {
        double v = text.toDouble(&isInputValid);
        setValid(isInputValid);
        if (isInputValid)
            setValue(v, send);
        return;
    }

    auto s = text.split('/', QString::SkipEmptyParts);
    if (s.size() < 1)
    {
        setValid(false);
        return;
    }

    int n = s[0].toInt(&isInputValid);
    if (!isInputValid)
    {
        setValid(false);
        return;
    }

    if (s.size() != 2)
    {
        setValid(false);
        return;
    }

    int d = s[1].toInt(&isInputValid);
    if (!isInputValid)
    {
        setValid(false);
        return;
    }

    setValid(true);
    setValue(n, d, send);
}

void DoubleSpinBoxFract::Private::valueToText()
{

    QString txt;
    if (!isFractional)
        txt = QString::number(value);
    else
        txt = fraction.toString();

    if (!prefix.isEmpty())
        txt = prefix + " " + txt;
    if (!suffix.isEmpty())
        txt += " " + suffix;

    setValid(true);
    updateToolTips();

    MO__D("valueToText() -> '" << txt << "'");
    ignoreChange = true;
    int cp = lineEdit->cursorPosition();
    lineEdit->setText(txt);
    lineEdit->setCursorPosition(cp);
    ignoreChange = false;
}

void DoubleSpinBoxFract::wheelEvent(QWheelEvent* e)
{
    step(e->angleDelta().y(), true);
}

void DoubleSpinBoxFract::step(int direction, bool send)
{
    if (!direction)
        return;
    direction = direction > 0 ? 1 : -1;

    if (!isFractional())
        setValue(value() + singleStep() * direction, send);
    else
        setValue(MATH::Fraction(p_->fraction.nom, p_->fraction.denom + direction),
                 send);
}


} // namespace GUI
} // namespace MO
