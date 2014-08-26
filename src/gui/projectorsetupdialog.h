/** @file projectorsetupdialog.h

    @brief Dialog to setup and preview projectors and mapping

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#ifndef MOSRC_GUI_PROJECTORSETUPDIALOG_H
#define MOSRC_GUI_PROJECTORSETUPDIALOG_H

#include <QDialog>

class QVBoxLayout;

namespace MO {
namespace GUI {

class DomePreviewWidget;
class DoubleSpinBox;

class ProjectorSetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectorSetupDialog(QWidget *parent = 0);

signals:

public slots:

protected:

    void closeEvent(QCloseEvent *);

private slots:

    void onGlReleased_();

private:

    void createWidgets_();
    DoubleSpinBox * createDoubleSpin(QLayout * layout,
                        const QString& desc, const QString& statusTip,
                        double value, double smallstep=1, double minv=-9999999, double maxv=9999999);

    bool closeRequest_;

    DomePreviewWidget * display_;

    DoubleSpinBox
        * spinRadius_,

        * spinFov_,
        * spinLat_,
        * spinLong_,
        * spinPitch_,
        * spinYaw_,
        * spinRoll_;

};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_PROJECTORSETUPDIALOG_H
