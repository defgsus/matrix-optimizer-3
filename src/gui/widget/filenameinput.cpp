/** @file filenameinput.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/25/2015</p>
*/

#include <QLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QCompleter>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include "filenameinput.h"
#include "io/application.h"
#include "io/files.h"

namespace MO {
namespace GUI {


FilenameInput::FilenameInput(IO::FileType filetype, bool directoryOnly, QWidget *parent)
    : QWidget       (parent)
    , directoryOnly_(directoryOnly)
    , ignoreSig_    (false)
    , ftype_        (filetype)
{
    createWidgets_();

    ignoreSig_ = true;
    onTextChanged_();
    ignoreSig_ = false;
}



void FilenameInput::createWidgets_()
{
    auto fsmodel = new QFileSystemModel(this);
    fsmodel->setRootPath(QDir::currentPath());
    auto comple = new QCompleter(fsmodel, this);

    auto lh = new QHBoxLayout(this);
    lh->setMargin(0);

        edit_ = new QLineEdit(this);
        edit_->setCompleter(comple);
        edit_->setStatusTip(tr("Type the filename or use the dialog button at the right"));
        connect(edit_, SIGNAL(textChanged(QString)),
                this, SLOT(onTextChanged_()));
        lh->addWidget(edit_);

        but_ = new QToolButton(this);
        but_->setText(tr("..."));
        but_->setStatusTip(tr("Click to open a file dialog"));
        connect(but_, SIGNAL(clicked()), this, SLOT(openDialog()));
        lh->addWidget(but_);

}


QString FilenameInput::fileName() const
{
    return edit_->text();
}

void FilenameInput::setFilename(const QString & fn)
{
    ignoreSig_ = true;
    edit_->setText(fn);
    ignoreSig_ = false;
}

void FilenameInput::onTextChanged_()
{
    bool valid = !fileName().isEmpty();
    if (valid)
    {
        if (directoryOnly_)
        {
            QDir dir(fileName());
            valid = dir.exists();
        }
        else
        {
            QFileInfo fi(fileName());
            valid = fi.exists();
        }
    }

    edit_->setProperty("invalid", !valid);
    edit_->setStyleSheet(application()->styleSheet());

    if (ignoreSig_)
        return;

    emit filenameChanged(fileName());
}

void FilenameInput::openDialog()
{
    QString fn;
    if (directoryOnly_)
        fn = IO::Files::getDirectory(ftype_, this);
    else
        /** @todo need open-or-save flag */
        fn = IO::Files::getOpenFileName(ftype_, this);

    if (fn.isEmpty())
        return;

    setFilename(fn);
}

} // namespace GUI
} // namespace MO
