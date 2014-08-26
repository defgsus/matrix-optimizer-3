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
class DomeSettings;
class ProjectorSettings;
namespace GUI {

class DomePreviewWidget;
class DoubleSpinBox;

class ProjectorSetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectorSetupDialog(QWidget *parent = 0);
    ~ProjectorSetupDialog();

signals:

public slots:

protected:

    void closeEvent(QCloseEvent *);

private slots:

    void onGlReleased_();
    void updateDomeSettings_();
    void updateProjectorSettings_();
private:

    void createWidgets_();
    DoubleSpinBox * createDoubleSpin(QLayout * layout,
                        const QString& desc, const QString& statusTip,
                        double value, double smallstep=1, double minv=-9999999, double maxv=9999999,
                        const char * slot = 0 );

    bool closeRequest_;

    DomeSettings * domeSettings_;
    ProjectorSettings * projectorSettings_;

    DomePreviewWidget * display_;

    DoubleSpinBox
        * spinDomeRad_,
        * spinDomeCover_,
        * spinDomeTiltX_,
        * spinDomeTiltZ_,

        * spinFov_,
        * spinRadius_,
        * spinLat_,
        * spinLong_,
        * spinPitch_,
        * spinYaw_,
        * spinRoll_;

};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_PROJECTORSETUPDIALOG_H
