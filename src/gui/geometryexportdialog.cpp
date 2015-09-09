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
#include <QFile>

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
    setWindowTitle(tr("geometry exporter"));
    setObjectName("_GeometryExportDialog");

    setMinimumSize(480,320);

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

        comboFormat_ = new QComboBox(this);
        lv->addWidget(comboFormat_);
        comboFormat_->addItem(tr("Wavefront Object"));
        comboFormat_->addItem(tr("JavaScript arrays"));

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
            lh->addWidget(butOk);
            connect(butOk, SIGNAL(clicked()), this, SLOT(exportNow()));

            auto butCancel = new QPushButton(tr("Cancel"), this);
            lh->addWidget(butCancel);
            connect(butCancel, SIGNAL(clicked()), this, SLOT(reject()));

}

void GeometryExportDialog::setGeometry(const GEOM::Geometry * g)
{
    geometry_ = g;

    QString text;

    if (!geometry_)
        text = tr("no geometry");
    else
    {
        text = "<html>" + tr("%1 vertices<br/>%2 triangles<br/>%3 lines<br/>memory: %4")
                            .arg(geometry_->numVertices())
                            .arg(geometry_->numTriangles())
                            .arg(geometry_->numLines())
                            .arg(byte_to_string(geometry_->memory()));

        if (!geometry_->numTriangles() && !geometry_->numLines())
            text += "<p><font color=\"#a00\">" + tr("No faces in geometry!") + "</font></p>";
        else
        if (!geometry_->numTriangles())
            text += "<p><font color=\"#a00\">"
                    + tr("Lines in obj files are generally not supported "
                         "by other applications.") + "</font></p>";

        text += "</html>";
    }

    infoLabel_->setText(text);

    // make export flags sensible
    const bool hasTriangle = geometry_ && geometry_->numTriangles() != 0;
    cbNormals_->setChecked(cbNormals_->isChecked() && hasTriangle);
    cbTexCoords_->setChecked(cbTexCoords_->isChecked() && hasTriangle);
}

void GeometryExportDialog::exportNow()
{
    if (!geometry_)
        return;

    QString filename = IO::Files::getSaveFileName(IO::FT_MODEL, this, true, false);
    if (filename.isEmpty())
        return;

    if (export_(filename))
        accept();
}

bool GeometryExportDialog::export_(const QString &filename)
{
    auto idx = comboFormat_->currentIndex();
    if (idx < 1)
        return exportObj_(filename);
    if (idx < 2)
        return exportJavaScript_(filename);
    return false;
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
                              tr("Failed to export\n%1").arg(filename));
        return false;
    }

    return true;
}

bool GeometryExportDialog::exportJavaScript_(const QString &filename)
{
    QString script = geometry_->toJavaScriptArray("",
                                                  cbNormals_->isChecked(),
                                                  cbTexCoords_->isChecked());
    QFile f(filename);
    if (!f.open(QFile::Text | QFile::WriteOnly))
    {
        QMessageBox::critical(this, tr("javascript exporter"),
                              tr("Failed to open for writing:\n%1").arg(filename));
        return false;
    }

    f.write(script.toUtf8());

    return true;
}



} // namespace GUI
} // namespace MO
