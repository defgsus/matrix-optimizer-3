/** @file spinbox.cpp

    @brief Wrapper around QSpinBox to avoid unwanted valueChanged() signal

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWheelEvent>
#include <QToolButton>

#include "SpinBox.h"

namespace MO {
namespace GUI {

struct SpinBox::Private
{
    Private(SpinBox * p)
        : p             (p)
        , value         (0.)
        , isInputValid  (true)
        , ignoreChange  (false)
        , lineEdit      (nullptr)
        , label         (nullptr)
        , layout        (nullptr)
        , minimum       (0)
        , maximum       (100)
        , singleStep    (1)
    { }

    void createWidgets();
    void textToValue(const QString& text, bool send);
    void valueToText();
    void setValue(int, bool send);

    SpinBox* p;

    int value;
    bool isInputValid, ignoreChange;

    QLineEdit* lineEdit;
    QLabel* label;
    QHBoxLayout* layout;
    int minimum,
        maximum,
        singleStep;
    QString prefix, suffix;
};

SpinBox::SpinBox(QWidget *parent)
    : QWidget           (parent)
    , p_                (new Private(this))
{
    p_->createWidgets();
    p_->valueToText();
}

void SpinBox::Private::createWidgets()
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
            textToValue(lineEdit->text(), true);
        });

        int h = lineEdit->height() * .7,
            w = h * .8;
        auto but = new QToolButton(p);
        but->setFixedSize(w, h);
        but->setIcon(QIcon(":/icon/small_up.png"));
        layout->addWidget(but);
        connect(but, &QToolButton::clicked, [=](){ p->step(1, true); });

        but = new QToolButton(p);
        but->setFixedSize(w, h);
        but->setIcon(QIcon(":/icon/small_down.png"));
        layout->addWidget(but);
        connect(but, &QToolButton::clicked, [=](){ p->step(-1, true); });

}

QLineEdit* SpinBox::lineEdit() const { return p_->lineEdit; }
void SpinBox::setMinimum(int min) { p_->minimum = min; }
void SpinBox::setMaximum(int max) { p_->maximum = max; }
void SpinBox::setRange(int minimum, int maximum)
    { p_->minimum = minimum, p_->maximum = maximum; }
void SpinBox::setSingleStep(int val) { p_->singleStep = val; }
void SpinBox::setPrefix(const QString& prefix) { p_->prefix = prefix; }
void SpinBox::setSuffix(const QString& suffix) { p_->suffix = suffix; }

int  SpinBox::value() const { return p_->value; }
int  SpinBox::minimum() const { return p_->minimum; }
int  SpinBox::maximum() const { return p_->maximum; }
int  SpinBox::singleStep() const { return p_->singleStep; }
QString SpinBox::prefix() const { return p_->prefix; }
QString SpinBox::suffix() const { return p_->suffix; }


void SpinBox::setLabel(const QString& s)
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

void SpinBox::setValue(int v, bool sendSignal)
{
    p_->setValue(v, sendSignal);
    p_->valueToText();
}

void SpinBox::Private::setValue(int v, bool send)
{
    int old = value;
    value = std::max(minimum, std::min(maximum, v ));
    if (send && old != value)
        emit p->valueChanged(value);
}

void SpinBox::Private::textToValue(const QString &text, bool send)
{
    int v = text.toInt(&isInputValid);
    if (isInputValid)
        setValue(v, send);
}

void SpinBox::Private::valueToText()
{
    QString txt;
    txt = QString::number(value);

    ignoreChange = true;
    int cp = lineEdit->cursorPosition();
    lineEdit->setText(txt);
    lineEdit->setCursorPosition(cp);
    ignoreChange = false;
}

void SpinBox::wheelEvent(QWheelEvent* e)
{
    step(e->angleDelta().y(), true);
}

void SpinBox::step(int direction, bool send)
{
    if (!direction)
        return;
    direction = direction > 0 ? 1 : -1;

    setValue(value() + singleStep() * direction, send);
}

} // namespace GUI
} // namespace MO
