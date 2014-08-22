/** @file geometryexportdialog.cpp

    @brief Export dialog for geometry data

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QFrame>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>

#include "geometryexportdialog.h"
#include "io/error.h"
#include "io/files.h"
#include "tool/stringmanip.h"
#include "geom/objexporter.h"
#include "geom/geometry.h"

namespace MO {
namespace GUI {

GeometryExportDialog::GeometryExportDialog(QWidget *parent) :
    QDialog         (parent),
    geometry_       (0)
{
    setMinimumSize(640,480);

    createWidgets_();
}


void GeometryExportDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        infoLabel_ = new QLabel(tr("no geometry"), this);
        lv->addWidget(infoLabel_);

        lv->addStretch(2);

        auto hline = new QFrame(this);
        lv->addWidget(hline);
        hline->setFrameShape(QFrame::HLine);

        // --- format ---

        auto comboFormat_ = new QComboBox(this);
        lv->addWidget(comboFormat_);
        comboFormat_->addItem(tr("Wavefront Object"));

        // --- export flags ---

        cbNormals_ = new QCheckBox(tr("export normals"), this);
        lv->addWidget(cbNormals_);
        cbNormals_->setChecked(true);

        cbTexCoords_ = new QCheckBox(tr("export texture coords."), this);
        lv->addWidget(cbTexCoords_);
        cbTexCoords_->setChecked(true);


        // --- buttons ---

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto butOk = new QPushButton(tr("Export"), this);
            butOk->setDefault(true);
            lv->addWidget(butOk);
            connect(butOk, SIGNAL(clicked()), this, SLOT(exportNow()));

            auto butCancel = new QPushButton(tr("Cancel"), this);
            lv->addWidget(butOk);
            connect(butCancel, SIGNAL(clicked()), this, SLOT(reject()));

}

void GeometryExportDialog::setGeometry(const GEOM::Geometry * g)
{
    geometry_ = g;

    if (!geometry_)
        infoLabel_->setText(tr("no geometry"));
    else
        infoLabel_->setText(tr("%1 vertices\n%2 triangles\n%3 lines\nmemory: %4")
                            .arg(geometry_->numVertices())
                            .arg(geometry_->numTriangles())
                            .arg(geometry_->numLines())
                            .arg(byte_to_string(geometry_->memory())));
}

void GeometryExportDialog::exportNow()
{
    if (!geometry_)
        return;

    QString filename = IO::Files::getSaveFileName(IO::FT_MODEL, this, true, false);
    if (filename.isEmpty())
        return;

    if (exportObj_(filename))
        accept();
}


bool GeometryExportDialog::exportObj_(const QString &filename)
{
    GEOM::ObjExporter exp;

    exp.setOption(GEOM::ObjExporter::EO_NORMALS, cbNormals_->isChecked());
    exp.setOption(GEOM::ObjExporter::EO_TEX_COORDS, cbTexCoords_->isChecked());

    try
    {
        exp.exportGeometry(filename, geometry_);
    }
    catch (const Exception & e)
    {
        QMessageBox::critical(this, tr("obj exporter"),
                              tr("Failed to export\n%1"));
        return false;
    }

    return true;
}




} // namespace GUI
} // namespace MO
