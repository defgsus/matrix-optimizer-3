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

namespace MO {
namespace GUI {

class SpinBox;
class DoubleSpinBox;
class GeometryWidget;

class GeometryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GeometryDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~GeometryDialog();

signals:

public slots:

protected slots:

    void updateGeometry_();
    void updateFromWidgets_();

protected:
    //void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

    void createWidgets_();

    GeometryWidget * geoWidget_;
    GL::GeometryFactorySettings * settings_;
    //bool geomChanged_;

    QLabel * labelInfo_, *labelSeg_;
    QComboBox * comboType_;
    QCheckBox * cbTriangles_, *cbSharedVert_, *cbConvertToLines_, *cbTess_, *cbNorm_;
    DoubleSpinBox *spinS_, *spinSX_, *spinSY_, *spinSZ_;
    SpinBox * spinSegU_, *spinSegV_, *spinTess_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_GEOMETRYDIALOG_H
