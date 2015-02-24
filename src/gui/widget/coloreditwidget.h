/** @file coloreditwidget.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#ifndef COLOREDITWIDGET_H
#define COLOREDITWIDGET_H

#include <QLineEdit>
#include <QColor>

namespace MO {
namespace GUI {

/** General QRgb input widget in hex format */
class ColorEditWidget : public QLineEdit
{
    Q_OBJECT
public:
    explicit ColorEditWidget(const QColor& , QWidget *parent = 0);
    explicit ColorEditWidget(QWidget *parent = 0)
           : ColorEditWidget(QColor(), parent) { }

    // ----- getter -------

    /** Currently selected color */
    const QColor& currentColor() const { updateFromText_(); return p_color_; }

signals:

public slots:

    /** Sets current color and updates gui */
    void setCurrentColor(const QColor& c) { p_color_ = c; updateFromColor_(); }

private:

    void updateFromColor_();
    void updateFromText_() const;

    mutable QColor p_color_;
};


} // namespace GUI
} // namespace MO


#endif // COLOREDITWIDGET_H
