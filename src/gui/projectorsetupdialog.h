/** @file projectorsetupdialog.h

    @brief Dialog to setup and preview projectors and mapping

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#ifndef MOSRC_GUI_PROJECTORSETUPDIALOG_H
#define MOSRC_GUI_PROJECTORSETUPDIALOG_H

#include <QDialog>

class QComboBox;
class QLineEdit;
class QToolButton;

namespace MO {
class DomeSettings;
class ProjectorSettings;
class ProjectionSystemSettings;
namespace GUI {

class DomePreviewWidget;
class DoubleSpinBox;
class SpinBox;

class ProjectorSetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectorSetupDialog(QWidget *parent = 0);
    ~ProjectorSetupDialog();

signals:

public slots:

    /** Sets the view.
        @p dir is one of the Basic3DWidget::ViewDirection enums */
    void setViewDirection(int dir);

protected:

    void closeEvent(QCloseEvent *);

private slots:

    void onGlReleased_();
    void changeView_();
    void updateDomeSettings_();
    void updateDomeName_();
    void updateDomeWidgets_();
    void updateProjectorSettings_();
    void updateProjectorName_();
    void updateProjectorWidgets_();
    void updateProjectorList_();
    void updateDisplay_();

    void projectorSelected_();
    void newProjector_();
    void duplicateProjector_();
    void deleteProjector_();
    //void moveProjectorUp_();
    //void moveProjectorDown_();

    bool savePreset_(const QString & fn);
    bool savePresetChoose_();
    void loadPreset_();

private:

    void updateWindowTitle_();
    bool saveToClose_();

    void createWidgets_();
    QLineEdit * createEdit_(QLayout * layout,
                        const QString& desc, const QString& statusTip,
                        const QString& value,
                        const char * slot = 0);
    SpinBox * createSpin_(QLayout * layout,
                        const QString& desc, const QString& statusTip,
                        int value, int smallstep=1, int minv=-9999999, int maxv=9999999,
                        const char * slot = 0);
    DoubleSpinBox * createDoubleSpin_(QLayout * layout,
                        const QString& desc, const QString& statusTip,
                        double value, double smallstep=1, double minv=-9999999, double maxv=9999999,
                        const char * slot = 0);


    bool closeRequest_,
         saidNoAlready_;

    QString filename_;

    ProjectionSystemSettings * settings_, * orgSettings_;

    DomeSettings * domeSettings_;
    ProjectorSettings * projectorSettings_;

    DomePreviewWidget * display_;

    QLineEdit
        * editName_,
        * editDomeName_;

    SpinBox
        * spinWidth_,
        * spinHeight_;

    DoubleSpinBox
        * spinDomeRad_,
        * spinDomeCover_,
        * spinDomeTiltX_,
        * spinDomeTiltZ_,

        * spinFov_,
        * spinLensRad_,
        * spinDist_,
        * spinLat_,
        * spinLong_,
        * spinPitch_,
        * spinYaw_,
        * spinRoll_;

    QComboBox
        * comboProj_,
        * comboView_;

    QToolButton
        * tbAdd_,
        * tbDup_,
        * tbRemove_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_PROJECTORSETUPDIALOG_H
