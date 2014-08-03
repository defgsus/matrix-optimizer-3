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
    //void setGeometrySettings(const GEOM::GeometryFactorySettings&);

protected slots:

    void updateGeometry_();
    void updateFromWidgets_();
    void loadModelFile_();

    void creationFailed_(const QString&);
    void creationFinished_();

protected:
    //void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

    void createWidgets_();

    GeometryWidget * geoWidget_;
    GEOM::GeometryFactorySettings * settings_;
    GEOM::GeometryCreator * creator_;
    bool updateGeometryLater_;

    QLabel * labelInfo_, *labelSeg_, *labelNormAmt_;
    QLineEdit * editFilename_;
    QToolButton * butLoadModelFile_;
    QComboBox * comboType_;
    QCheckBox * cbTriangles_, *cbSharedVert_, *cbConvertToLines_, *cbCalcNormals_,
            *cbTess_, *cbNorm_, *cbRemove_, *cbTransformEqu_;
    DoubleSpinBox *spinS_, *spinSX_, *spinSY_, *spinSZ_, *spinRemoveProb_,
                *spinNormAmt_;
    SpinBox * spinSegX_, *spinSegY_, *spinSegZ_, *spinTess_, *spinRemoveSeed_;
    EquationEditor *editEquX_, *editEquY_, *editEquZ_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_GEOMETRYDIALOG_H
