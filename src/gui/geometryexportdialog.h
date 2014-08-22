/** @file geometryexportdialog.h

    @brief Export dialog for geometry data

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#ifndef MOSRC_GUI_GEOMETRYEXPORTDIALOG_H
#define MOSRC_GUI_GEOMETRYEXPORTDIALOG_H

#include <QDialog>

class QLabel;
class QCheckBox;

namespace MO {
namespace GEOM { class Geometry; }
namespace GUI {


class GeometryExportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GeometryExportDialog(QWidget *parent = 0);

signals:

public slots:

    /** Sets the geometry to export */
    void setGeometry(const GEOM::Geometry *);

    /** Opens a filedialog and exports the geometry.
        After export, the GeometryExportDialog will be closed. */
    void exportNow();

private:

    void createWidgets_();
    bool exportObj_(const QString& filename);

    const GEOM::Geometry * geometry_;

    QLabel * infoLabel_;
    QCheckBox *cbNormals_, *cbTexCoords_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_GEOMETRYEXPORTDIALOG_H
