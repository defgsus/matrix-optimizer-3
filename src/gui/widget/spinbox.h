/** @file spinbox.h

    @brief Wrapper around QSpinBox to avoid unwanted valueChanged() signal

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_SPINBOX_H
#define MOSRC_GUI_WIDGET_SPINBOX_H

#include <QSpinBox>

#include <QDoubleSpinBox>

class QLabel;
class QHBoxLayout;

namespace MO {
namespace GUI {

/** Wrapper around QSpinBox.
    Most of all, to have better control over the valueChanged() signal */
class SpinBox : public QWidget
{
    Q_OBJECT
public:
    explicit SpinBox(QWidget *parent = 0);

    QSpinBox * spinBox() const { return spin_; }

    void setMinimum(int min) { spin_->setMinimum(min); }
    void setMaximum(int max) { spin_->setMaximum(max); }
    void setRange(int minimum, int maximum) { spin_->setRange(minimum, maximum); }
    void setSingleStep(int val) { spin_->setSingleStep(val); }
    void setPrefix(const QString& prefix) { spin_->setPrefix(prefix); }
    void setSuffix(const QString& suffix) { spin_->setSuffix(suffix); }

    int value() const { return spin_->value(); }
    int minimum() const { return spin_->minimum(); }
    int maximum() const { return spin_->maximum(); }
    int singleStep() const { return spin_->singleStep(); }
    QString prefix() const { return spin_->prefix(); }
    QString suffix() const { return spin_->suffix(); }

    QString cleanText() const { return spin_->cleanText(); }

signals:

    void valueChanged(int);

public slots:

    void setValue(int val, bool send_signal = false);
    /** Attaches a label to the spinbox */
    void setLabel(const QString&);

private slots:
    void internValueChanged_(int);

private:
    QSpinBox * spin_;
    QLabel * label_;
    QHBoxLayout * layout_;
    bool ignoreSignal_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_SPINBOX_H
