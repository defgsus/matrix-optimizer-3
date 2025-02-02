/** @file files.cpp

    @brief default / configured Files and filetypes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/5/2014</p>
*/

#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QTemporaryFile>

#include "Files.h"
#include "io/Settings.h"
#include "io/log_io.h"


namespace MO {
namespace IO {


void Files::fixDirectoryString(QString& f)
{
#ifdef Q_OS_WIN
    f.replace('/', QDir::separator());
#else
    Q_UNUSED(f);
#endif
}

void Files::trailingSeparator(QString& s)
{
#ifdef Q_OS_WIN
    if (s.endsWith('/'))
        s[s.size()-1] = '\\';
    else if (!s.endsWith('\\'))
        s += '\\';
#else
    if (s.endsWith('\\'))
        s[s.size()-1] = '/';
    else if (!s.endsWith('/'))
        s += '/';
#endif
}

QString Files::directory(FileType ft, bool retCur)
{
    // check for directory settings
    QString key = "Directory/" + fileTypeIds[ft];
    QString dir = settings()->getValue(key).toString();

    if (dir.isEmpty())
    {
        // or take directory of last filename
        dir = settings()->getValue("File/" + fileTypeIds[ft]).toString();
        if (dir.isEmpty())
        {
            if (!retCur)
                return QString();
            // or return current path if both unknown
            return QDir::currentPath();
        }

        QFileInfo info(dir);
        dir = info.absolutePath();
    }

    trailingSeparator(dir);

    return dir;
}

QString Files::filename(FileType ft)
{
    const QString key = "File/" + fileTypeIds[ft];
    if (!settings()->contains(key))
        return QString();

    return
        settings()->getValue(key).toString();
}

void Files::setFilename(FileType ft, const QString& fn)
{
    settings()->setValue("File/" + fileTypeIds[ft], fn);
}

void Files::setDirectory(FileType ft, const QString &path)
{
    settings()->setValue("Directory/" + fileTypeIds[ft],
                         QFileInfo(path).absolutePath());
}

QString Files::getOpenFileName(FileType ft, QWidget *parent,
                               bool updateDirectory, bool updateFile)
{
    bool isDir = false;
    QString fn = filename(ft);
    if (fn.isEmpty())
        fn = directory(ft), isDir = true;

    //MO_PRINT(":" << filename(ft) << "\n:" << fn << "\n");

    // prepare a file dialog
    QFileDialog diag(parent);
    diag.setConfirmOverwrite(false);
    diag.setAcceptMode(QFileDialog::AcceptOpen);
    diag.setWindowTitle(QFileDialog::tr("Open %1").arg(fileTypeNames[ft]));
    diag.setDirectory(fn);
    diag.setNameFilters(fileTypeDialogFilters[ft]);
    diag.setFileMode(QFileDialog::ExistingFiles);
    if (!isDir && !fn.isEmpty())
        diag.selectFile(fn);

    if (diag.exec() == QDialog::Rejected
        || diag.selectedFiles().isEmpty())
        return QString();

    fn = diag.selectedFiles()[0];

    if (!fn.isEmpty())
    {
        if (updateFile)
            setFilename(ft, fn);

        if (updateDirectory)
            setDirectory(ft, QFileInfo(fn).absolutePath());
    }

    return fn;
}


QStringList Files::getOpenFileNames(FileType ft, QWidget *parent,
                                    bool updateDirectory, bool updateFile)
{
    bool isDir = false;
    QString fn = filename(ft);
    if (fn.isEmpty())
        fn = directory(ft), isDir = true;

    // prepare a file dialog
    QFileDialog diag(parent);
    diag.setConfirmOverwrite(false);
    diag.setAcceptMode(QFileDialog::AcceptOpen);
    diag.setWindowTitle(QFileDialog::tr("Open %1").arg(fileTypeNames[ft]));
    diag.setDirectory(QFileInfo(fn).absolutePath());
    diag.setNameFilters(fileTypeDialogFilters[ft]);
    diag.setFileMode(QFileDialog::ExistingFiles);
    if (!isDir && !fn.isEmpty())
        diag.selectFile(fn);

    if (diag.exec() == QDialog::Rejected
        || diag.selectedFiles().isEmpty())
        return QStringList();

    fn = diag.selectedFiles()[0];

    if (!fn.isEmpty())
    {
        if (updateFile)
            setFilename(ft, fn);

        if (updateDirectory)
            setDirectory(ft, QFileInfo(fn).absolutePath());
    }

    return diag.selectedFiles();
}

QString Files::getSaveFileName(FileType ft, QWidget *parent, bool updateDirectory, bool updateFile)
{
    bool isDir = false;
    QString fn = filename(ft);
    if (fn.isEmpty())
        fn = directory(ft), isDir = true;

    // prepare a file dialog
    QFileDialog diag(parent);
    diag.setDefaultSuffix(fileTypeExtensions[ft][0]);
    diag.setConfirmOverwrite(true);
    diag.setAcceptMode(QFileDialog::AcceptSave);
    diag.setWindowTitle(QFileDialog::tr("Save %1").arg(fileTypeNames[ft]));
    diag.setDirectory(fn);
    diag.setNameFilters(fileTypeDialogFilters[ft]);
    if (!isDir && !fn.isEmpty())
        diag.selectFile(fn);

    if (diag.exec() == QDialog::Rejected
        || diag.selectedFiles().isEmpty())
        return QString();

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

    fn = diag.selectedFiles()[0];

    if (updateFile)
        setFilename(ft, fn);

    if (updateDirectory)
        setDirectory(ft, QFileInfo(fn).absolutePath());

    return fn;
}



QString Files::getDirectory(FileType ft, QWidget *parent, bool updateDirectory, const QString& title)
{
    QString fn = directory(ft);

    // prepare a file dialog
    QFileDialog diag(parent);
    diag.setConfirmOverwrite(false);
    diag.setAcceptMode(QFileDialog::AcceptOpen);
    if (title.isEmpty())
        diag.setWindowTitle(QFileDialog::tr("Choose directory for %1").arg(fileTypeNames[ft]));
    else
        diag.setWindowTitle(title);
    diag.setDirectory(fn);
    diag.setFileMode(QFileDialog::Directory);
    diag.setOption(QFileDialog::ShowDirsOnly, true);

    if (diag.exec() == QDialog::Rejected)
        return QString();

    fn = diag.directory().absolutePath();

    if (!fn.isEmpty())
    {
        if (updateDirectory)
            setDirectory(ft, fn);
    }

    return fn;
}

void Files::findFiles(FileType ft, bool recursive, QStringList &entryList, bool include_directory)
{
    findFiles(ft, directory(ft), recursive, entryList, include_directory);
}

void Files::findFiles(FileType ft, const QString &directory,
                      bool recursive, QStringList &entryList, bool include_directory)
{
    MO_DEBUG_IO("Files::findFiles(" << ft << ", " << directory << ", "
             << recursive << ")");

    QDir dir(directory);

    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot
                  | QDir::Readable  | QDir::Hidden);
    //dir.setSorting(QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);

    QStringList filters;
    for (auto & s : fileTypeExtensions[ft])
        filters << ("*." + s);
    dir.setNameFilters(filters);

    QStringList files = dir.entryList();
    if (include_directory)
        for (auto & f : files)
            entryList << (directory + QDir::separator() + f);
    else
        for (auto & f : files)
            entryList << f;

    if (recursive)
    {
        QStringList dirs;
        getDirectories(directory, dirs, true);

        for (auto & d : dirs)
        {
            findFiles(ft, directory + QDir::separator() + d, true, entryList);
        }
    }
}

void Files::getDirectories(const QString &basePath, QStringList &directoryList,
                           bool recursive)
{
    QDir dir(basePath);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);

    QStringList dirs = dir.entryList();

    directoryList << dirs;

    if (recursive)
    {
        for (auto & s : dirs)
            getDirectories(s, directoryList, true);
    }
}

QString Files::getTempFilename(const QString &ext)
{
    QTemporaryFile file("XXXXXX" + ext);
    if (!file.open())
        return QString();
    file.setAutoRemove(false);
    return file.fileName();
}

} // namespace IO
} // namespace MO
