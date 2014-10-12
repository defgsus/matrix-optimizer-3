/** @file clipview.h

    @brief Clip object view

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_GUI_CLIPVIEW_H
#define MOSRC_GUI_CLIPVIEW_H

#include <QWidget>

class QGridLayout;
class QScrollArea;

namespace MO {
namespace GUI {

class ClipView : public QWidget
{
    Q_OBJECT
public:
    explicit ClipView(QWidget *parent = 0);

signals:

public slots:


private:

    void createWidgets_();

    QScrollArea * scrollArea_;
    QWidget * container_;
    QGridLayout * layout_;

};

} // namespace GUI
} // namespace MO



#endif // MOSRC_GUI_CLIPVIEW_H
