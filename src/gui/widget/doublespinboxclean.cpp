/** @file doublespinboxclean.cpp

    @brief QDoubleSpinBox without the zero fill-ins

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#include "doublespinboxclean.h"


namespace MO {
namespace GUI {



DoubleSpinBoxClean::DoubleSpinBoxClean(QWidget *parent) :
    QDoubleSpinBox(parent)
{
}


QString DoubleSpinBoxClean::textFromValue(double val) const
{
    return QString::number(val);
}




} // namespace GUI
} // namespace MO
