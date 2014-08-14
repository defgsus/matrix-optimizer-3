/** @file geometrydialog.h

    @brief Editor for Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_GUI_GEOMETRYDIALOG_H
#define MOSRC_GUI_GEOMETRYDIALOG_H

#include <QDialog>
#include "gl/opengl_fwd.h"

class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QToolButton;
class QStatusBar;
class QProgressBar;
class QVBoxLayout;

namespace MO {
namespace GUI {

class SpinBox;
class DoubleSpinBox;
class GeometryWidget;
class EquationEditor;

class GeometryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GeometryDialog(const GEOM::GeometryFactorySettings * = 0,
                            QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~GeometryDialog();

    /** Returns the settings as edited in the dialog */
    const GEOM::GeometryFactorySettings& getGeometrySettings() const { return *settings_; }

signals:

public slots:

    /* Sets the settings to display/edit */
    void setGeometrySettings(const GEOM::GeometryFactorySettings&);

protected slots:

    void changeView_();
    void updateGeometry_();
    void updateFromWidgets_();
    void loadModelFile_();
    void savePreset_();
    void savePresetAs_();
    void deletePreset_();
    void presetSelected_();

    void creatorProgress_(int);
    void creationFailed_(const QString&);
    void creationFinished_();

protected:
    //void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    bool event(QEvent *);

    void createMainWidgets_();
    void createModifierWidgets_();
    void updateWidgets_();
    void updatePresetList_(const QString& selectFilename = QString());
    void updatePresetButtons_();

    GeometryWidget * geoWidget_;
    GEOM::GeometryFactorySettings * settings_;
    GEOM::GeometryCreator * creator_;
    GEOM::GeometryModifierChain * modifiers_;
    bool updateGeometryLater_,
         ignoreUpdate_;

    QList<QWidget*> modifierWidgets_;
    QVBoxLayout * modifierLayout_;


    QStatusBar * statusBar_;
    QProgressBar * progressBar_;
    QLabel * labelInfo_, *labelSeg_, *labelNormAmt_, *labelSmallRadius_;
    QLineEdit * editFilename_;
    QToolButton *butSavePreset_, *butSavePresetAs_, *butDeletePreset_,
                *butLoadModelFile_;
    QComboBox * comboPreset_, * comboView_, * comboType_;
    QCheckBox * cbTriangles_, *cbSharedVert_, *cbConvertToLines_, *cbCalcNormals_,
            *cbTess_, *cbNorm_, *cbRemove_, *cbTransformEqu_,
            *cbTransformPrimEqu_, *cbCalcNormalsBeforePrimEqu_,
            *cbInvNormals_, *cbExtrude_;
    DoubleSpinBox *spinS_, *spinSX_, *spinSY_, *spinSZ_, *spinRemoveProb_,
                *spinNormAmt_, *spinSmallRadius_, *spinExtrudeConstant_, *spinExtrudeFactor_;
    SpinBox * spinSegX_, *spinSegY_, *spinSegZ_, *spinTess_, *spinRemoveSeed_;
    EquationEditor *editEquX_, *editEquY_, *editEquZ_,
                    *editPEquX_, *editPEquY_, *editPEquZ_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_GEOMETRYDIALOG_H
