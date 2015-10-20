/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#ifndef MOSRC_GUI_OSCVIEW_H
#define MOSRC_GUI_OSCVIEW_H

#include <QWidget>

namespace MO {
namespace GUI {

/** Widget to inspect running osc connections */
class OscView : public QWidget
{
    Q_OBJECT
public:
    explicit OscView(QWidget *parent = 0);
    ~OscView();

signals:

private slots:

    void onListenersChanged();
    void onValuesChanged();

protected:

    void showEvent(QShowEvent *) Q_DECL_OVERRIDE;

private:

    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_OSCVIEW_H
