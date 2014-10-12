/** @file clipwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_CLIPWIDGET_H
#define MOSRC_GUI_WIDGET_CLIPWIDGET_H

#include <QWidget>
#include <QPen>
#include <QBrush>

namespace MO {
namespace GUI {

class ClipView;

class ClipWidget : public QWidget
{
public:

    enum Type
    {
        T_COLUMN,
        T_ROW,
        T_CLIP
    };

    explicit ClipWidget(int x, int y, ClipView * parent = 0);

protected:

    void updateColors_();
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    ClipView * view_;

    Type type_;
    int x_, y_;

    bool hasFocus_, isSelected_;

    QPen penOutline_, penOutlineS_;
    QBrush brushBody_, brushBodyH_, brushBodyS_, brushBodySH_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_CLIPWIDGET_H
