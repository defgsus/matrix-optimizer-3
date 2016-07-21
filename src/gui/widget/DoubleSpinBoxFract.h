/** @file doublespinboxclean.h

    @brief QDoubleSpinBox without the zero fill-ins

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_DOUBLESPINBOXCLEAN_H
#define MOSRC_GUI_WIDGET_DOUBLESPINBOXCLEAN_H

#include <QDoubleSpinBox>
#include "math/Fraction.h"

namespace MO {
namespace GUI {

/** A spinbox that can also understand fractions */
class DoubleSpinBoxFract : public QWidget
{
    Q_OBJECT
public:
    explicit DoubleSpinBoxFract(QWidget *parent = 0);

    QLineEdit* lineEdit() const;

    bool isFractional() const;
    const MATH::Fraction& fraction() const;

    void setLabel(const QString&);

    void setValue(double, bool sendSignal = false);
    void setValue(const MATH::Fraction&, bool sendSignal = false);
    void step(int direction, bool sendSignal = false);

    void setMinimum(double min);
    void setMaximum(double max);
    void setRange(double minimum, double maximum);
    void setDecimals(int prec);
    void setSingleStep(double val);
    void setPrefix(const QString& prefix);
    void setSuffix(const QString& suffix);

    double value() const;
    double minimum() const;
    double maximum() const;
    int decimals() const;
    double singleStep() const;
    QString prefix() const;
    QString suffix() const;

signals:

    /** Only emitted by user change or when sendSignal was true in setValue() */
    void valueChanged(double);

protected:

    void wheelEvent(QWheelEvent* e) override;

private:
    struct Private;
    Private* p_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_DOUBLESPINBOXCLEAN_H
