/** @file coloreditwidget.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#include "coloreditwidget.h"


namespace MO {
namespace GUI {


ColorEditWidget::ColorEditWidget(const QColor&, QWidget *parent)
    : QLineEdit     (parent)
{
    setInputMask("HHHHHHHH");
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    setCurrentColor(Qt::white);
}


void ColorEditWidget::updateFromColor_()
{
    setText( QString("%1").arg(p_color_.rgba(), 8, 16) );
}

void ColorEditWidget::updateFromText_() const
{
    p_color_.setRgba( text().toUInt(0, 16) );
}

} // namespace GUI
} // namespace MO
