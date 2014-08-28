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
class QMenuBar;
class QAction;

namespace MO {
class DomeSettings;
class ProjectorSettings;
class CameraSettings;
class ProjectionSystemSettings;
namespace GUI {

class DomePreviewWidget;
class DoubleSpinBox;
class SpinBox;
class GroupWidget;

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

    void clearPreset_();
    bool savePresetAuto_();
    bool savePreset_(const QString & fn);
    bool savePresetChoose_();
    void loadPreset_();

    void nextProjector_();
    void previousProjector_();
    void copyProjector_();
    void pasteProjector_();
    void copyCamera_();
    void pasteCamera_();

private:

    void updateWindowTitle_();
    void updateActions_();

    bool saveToClose_();
    bool saveToClear_();

    void createMenu_();
    void createWidgets_();

    QLineEdit * createEdit_(GroupWidget * group,
                        const QString& desc, const QString& statusTip,
                        const QString& value,
                        const char * slot = 0);
    SpinBox * createSpin_(GroupWidget * group,
                        const QString& desc, const QString& statusTip,
                        int value, int smallstep=1, int minv=-9999999, int maxv=9999999,
                        const char * slot = 0);
    DoubleSpinBox * createDoubleSpin_(GroupWidget * group,
                        const QString& desc, const QString& statusTip,
                        double value, double smallstep=1, double minv=-9999999, double maxv=9999999,
                        const char * slot = 0);


    bool closeRequest_,
         saidNoAlready_;

    QString filename_;

    ProjectionSystemSettings * settings_, * orgSettings_;

    DomeSettings * domeSettings_;
    ProjectorSettings * projectorSettings_, *copyOfProjectorSettings_;
    CameraSettings * cameraSettings_, *copyOfCameraSettings_;

    DomePreviewWidget * display_;

    QMenuBar * mainMenu_;

    GroupWidget * projectorGroup_, * cameraGroup_;

    QLineEdit
        * editName_,
        * editDomeName_;

    SpinBox
        * spinWidth_,
        * spinHeight_,

        * spinCamWidth_,
        * spinCamHeight_;

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
        * spinRoll_,

        * spinCamFov_,
        * spinCamX_,
        * spinCamY_,
        * spinCamZ_,
        * spinCamPitch_,
        * spinCamYaw_,
        * spinCamRoll_,
        * spinCamZNear_,
        * spinCamZFar_;

    QComboBox
        * comboProj_,
        * comboView_;

    QToolButton
        * tbAdd_,
        * tbDup_,
        * tbRemove_;

    QAction
        * aPasteProjector_,
        * aPasteCamera_,
        * aNext_,
        * aPrevious_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_PROJECTORSETUPDIALOG_H
