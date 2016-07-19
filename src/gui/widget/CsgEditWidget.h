/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/7/2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_CSGEDITWIDGET_H
#define MOSRC_GUI_WIDGET_CSGEDITWIDGET_H

#include <QWidget>

namespace MO {
class CsgRoot;
namespace GUI {

class CsgEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CsgEditWidget(QWidget *parent = 0);
    ~CsgEditWidget();

    CsgRoot * rootObject() const;

signals:

    void changed() const;

public slots:

    void setRootObject(CsgRoot *);

private:
    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_CSGEDITWIDGET_H
