/** @file Files.cpp

    @brief default / configured Files and filetypes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/5/2014</p>
*/

#include <QFileInfo>
#include <QDir>
#include <QFileDialog>

#include "files.h"
#include "io/settings.h"
#include "io/log.h"


namespace MO {
namespace IO {

const QStringList fileTypeIds =
{ "scene", "texture", "model", "geom-set" };

const QStringList fileTypeNames =
{ "Scene", "Texture", "Model", "Geometry settings" };

const QList<QStringList> fileTypeExtensions =
{ { "mo3" },
  { "png", "jpg", "jpeg", "bmp" },
  { "obj" },
  { "mo3-geom" }
};

const QList<QStringList> fileTypeDialogFilters =
{
    { "scene files ( *.mo3 )" },
    { "all image files ( *.png, *.jpg, *.bmp )" },
    { "Wavefront OBJ ( *.obj )" },
    { "geometry presets ( *.mo3-geom )" }
};


Files::Files()
{
}


QString Files::directory(FileType ft)
{
    // check for directory settings
    QString key = "Directory/" + fileTypeIds[ft];
    if (!settings->contains(key))
    {
        // or take directory of filename
        const QString d =
            settings->getValue("File/" + fileTypeIds[ft]).toString();
        if (d.isEmpty())
            // or return current path if both unknown
            return QDir::currentPath();

        QFileInfo info(d);
        return info.absolutePath();
    }

    const QString d =
        settings->getValue(key).toString();

    return d;
}

QString Files::filename(FileType ft)
{
    const QString key = "File/" + fileTypeIds[ft];
    if (!settings->contains(key))
        return QString();

    return
        settings->getValue(key).toString();
}

void Files::setFilename(FileType ft, const QString& fn)
{
    settings->setValue("File/" + fileTypeIds[ft], fn);
}

void Files::setDirectory(FileType ft, const QString &path)
{
    settings->setValue("Directory/" + fileTypeIds[ft], path);
}

QString Files::getOpenFileName(FileType ft, QWidget *parent, bool updateDirectory, bool updateFile)
{
    QString filters;
    for (auto &f : fileTypeDialogFilters[ft])
        filters += f + ";;";

    const QString fn =
        QFileDialog::getOpenFileName(
                parent,
                QWidget::tr("Load %1").arg(fileTypeNames[ft]),
                directory(ft),
                filters);

    if (!fn.isEmpty())
    {
        if (updateFile)
            setFilename(ft, fn);

        if (updateDirectory)
            setDirectory(ft, QFileInfo(fn).absolutePath());
    }

    return fn;
}

QString Files::getSaveFileName(FileType ft, QWidget *parent, bool updateDirectory, bool updateFile)
{
    // prepare a file dialog
    QFileDialog diag(parent);
    diag.setDefaultSuffix(fileTypeExtensions[ft][0]);
    diag.setConfirmOverwrite(true);
    diag.setAcceptMode(QFileDialog::AcceptSave);
    diag.setWindowTitle(QFileDialog::tr("Save %1").arg(fileTypeNames[ft]));
    diag.setDirectory(directory(ft));
    diag.setNameFilters(fileTypeDialogFilters[ft]);

    if (diag.exec() == QDialog::Rejected
        || diag.selectedFiles().isEmpty())
        return QString();

    MO_DEBUG("---------------" << diag.selectedNameFilter());
    /*
    // complete filename
    if (!fn.endsWith(".mo3"))
        fn.append(".mo3");

    // check existence
    if (!QFile::exists(fn))
        return fn;

    // ask for overwrite
    QMessageBox::StandardButton res =
    QMessageBox::question(this, tr("File already exists"),
                          tr("The file %1 already exists.\n"
                             "Do you want to replace it?").arg(fn)
                          , QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                          QMessageBox::No);

    if (res == QMessageBox::Yes)
        return fn;

    if (res == QMessageBox::Cancel)
        return QString();
    */

    QString fn = diag.selectedFiles()[0];

    if (updateFile)
        setFilename(ft, fn);

    if (updateDirectory)
        setDirectory(ft, QFileInfo(fn).absolutePath());

    return fn;
}


} // namespace IO
} // namespace MO
