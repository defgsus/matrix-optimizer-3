/** @file doublespinbox.h

    @brief Wrapper around QDoubleSpinBox to avoid unwanted valueChanged signal

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_DOUBLESPINBOX_H
#define MOSRC_GUI_DOUBLESPINBOX_H

#include <QDoubleSpinBox>

namespace MO {
namespace GUI {

class DoubleSpinBox : public QWidget
{
    Q_OBJECT
public:
    explicit DoubleSpinBox(QWidget *parent = 0);

    void setMinimum(double min) { spin_->setMinimum(min); }
    void setMaximum(double max) { spin_->setMaximum(max); }
    void setRange(double minimum, double maximum) { spin_->setRange(minimum, maximum); }
    void setDecimals(int prec) { spin_->setDecimals(prec); }
    void setSingleStep(double val) { spin_->setSingleStep(val); }
    void setPrefix(const QString& prefix) { spin_->setPrefix(prefix); }
    void setSuffix(const QString& suffix) { spin_->setSuffix(suffix); }

    double value() const { return spin_->value(); }
    double minimum() const { return spin_->minimum(); }
    double maximum() const { return spin_->maximum(); }
    int decimals() const { return spin_->decimals(); }
    double singleStep() const { return spin_->singleStep(); }
    QString prefix() const { return spin_->prefix(); }
    QString suffix() const { return spin_->suffix(); }

    QString cleanText() const { return spin_->cleanText(); }
    QString textFromValue(double value) const { return spin_->textFromValue(value); }
    double valueFromText(const QString& text) const { return spin_->valueFromText(text); }
    void fixup(QString & text) const { return spin_->fixup(text); }

signals:

    void valueChanged(double);

public slots:

    void setValue(double val, bool send_signal = false);

private slots:
    void internValueChanged_(double);

private:
    QDoubleSpinBox * spin_;
    bool ignoreSignal_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_DOUBLESPINBOX_H
