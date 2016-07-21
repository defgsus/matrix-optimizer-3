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

/** A mock of QSpinBox, to have the same visual style as DoubleSpinBoxFract */
class SpinBox : public QWidget
{
    Q_OBJECT
public:
    explicit SpinBox(QWidget *parent = 0);

    QLineEdit* lineEdit() const;

    void setLabel(const QString&);

    void setValue(int, bool sendSignal = false);
    void step(int direction, bool sendSignal = false);

    void setMinimum(int min);
    void setMaximum(int max);
    void setRange(int minimum, int maximum);
    void setSingleStep(int val);
    void setPrefix(const QString& prefix);
    void setSuffix(const QString& suffix);

    int value() const;
    int minimum() const;
    int maximum() const;
    int singleStep() const;
    QString prefix() const;
    QString suffix() const;

signals:

    /** Only emitted by user change or when sendSignal was true in setValue() */
    void valueChanged(int);

protected:

    void wheelEvent(QWheelEvent* e) override;

private:
    struct Private;
    Private* p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_SPINBOX_H
