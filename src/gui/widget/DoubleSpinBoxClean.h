/** @file doublespinboxclean.h

    @brief QDoubleSpinBox without the zero fill-ins

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_DOUBLESPINBOXCLEAN_H
#define MOSRC_GUI_WIDGET_DOUBLESPINBOXCLEAN_H

#include <QDoubleSpinBox>

namespace MO {
namespace GUI {


class DoubleSpinBoxClean : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit DoubleSpinBoxClean(QWidget *parent = 0);

    virtual QString textFromValue(double val) const Q_DECL_OVERRIDE;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_DOUBLESPINBOXCLEAN_H
