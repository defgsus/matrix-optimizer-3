/** @file bulkrenamedialog.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 14.01.2015</p>
*/

#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLayout>
#include <QDir>
#include <QTextEdit>
#include <QLabel>
#include <QSpinBox>

#include "bulkrenamedialog.h"
#include "object/util/objectfactory.h"
#include "object/scene.h"
#include "io/settings.h"
#include "io/files.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {

BulkRenameDialog::BulkRenameDialog(QWidget *parent) :
    QDialog(parent)
{
    setObjectName("_BulkRenameDialog");
    setWindowTitle(tr("Bulk file renamer"));
    setMinimumSize(640,480);
    settings()->restoreGeometry(this);

    createWidgets_();
}

BulkRenameDialog::~BulkRenameDialog()
{
    settings()->storeGeometry(this);
}

void BulkRenameDialog::createWidgets_()
{
    auto lv0 = new QVBoxLayout(this);

        auto lh = new QHBoxLayout();
        lv0->addLayout(lh);

            // ---- input ----

            auto lv = new QVBoxLayout();
            lh->addLayout(lv);

                auto lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                inputPath_ = new QLineEdit(this);
                inputPath_->setReadOnly(true);
                lh2->addWidget(inputPath_);

                auto but = new QPushButton("...", this);
                lh2->addWidget(but);
                connect(but, SIGNAL(clicked()), this, SLOT(chooseInputPath_()));

            input_ = new QListWidget(this);
            lv->addWidget(input_);

            // ---- output ----

            lv = new QVBoxLayout();
            lh->addLayout(lv);

                auto label = new QLabel(tr("preview"), this);
                lv->addWidget(label);

                output_ = new QListWidget(this);
                lv->addWidget(output_);

            // ---- options ----

            lv = new QVBoxLayout();
            lh->addLayout(lv);

                label = new QLabel(tr("name options"), this);
                lv->addWidget(label);

                lv->addSpacing(10);

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    label = new QLabel(tr("substitution"), this);
                    lh2->addWidget(label);

                    subStr_ = new QLineEdit(this);
                    subStr_->setText("%name_%n.%ext");
                    lh2->addWidget(subStr_);
                    connect(subStr_, SIGNAL(textChanged(QString)), this, SLOT(optionsChanged_()));

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    label = new QLabel(tr("sequence start"), this);
                    lh2->addWidget(label);

                    seqStart_ = new QSpinBox(this);
                    seqStart_->setRange(0, 1<<30);
                    lh2->addWidget(seqStart_);
                    connect(seqStart_, SIGNAL(valueChanged(int)), this, SLOT(optionsChanged_()));

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    label = new QLabel(tr("number digits"), this);
                    lh2->addWidget(label);

                    digitCount_= new QSpinBox(this);
                    digitCount_->setRange(0, 256);
                    digitCount_->setValue(6);
                    lh2->addWidget(digitCount_);
                    connect(digitCount_, SIGNAL(valueChanged(int)), this, SLOT(optionsChanged_()));

                lv->addStretch(1);

    log_ = new QTextEdit(this);
    lv0->addWidget(log_);
    log_->setReadOnly(true);

    butRename_ = new QPushButton(tr("Rename all"), this);
    lv0->addWidget(butRename_);
    butRename_->setDefault(true);
    butRename_->setEnabled(false);
    connect(butRename_, SIGNAL(clicked()), this, SLOT(rename_()));

    auto butClose = new QPushButton(tr("Close"), this);
    lv0->addWidget(butClose);
    connect(butClose, SIGNAL(clicked()), this, SLOT(accept()));

}


void BulkRenameDialog::chooseInputPath_()
{
    QString path = IO::Files::getDirectory(IO::FT_ANY, this, false);
    if (path.isEmpty())
        return;

    inputPath_->setText(path);
    fillInputList_(path, true);

    butRename_->setEnabled(canRename_());

    fillOutputList_();
}


void BulkRenameDialog::fillInputList_(const QString &path, bool checkable)
{
    input_->clear();

    QStringList files;
    IO::Files::findFiles(IO::FT_ANY, path, false, files, false);

    QDir dir(path);

    for (auto & f : files)
    {
        auto item = new QListWidgetItem(input_);
        if (checkable)
            item->setFlags(Qt::ItemIsSelectable
                           | Qt::ItemIsUserCheckable
                           | Qt::ItemIsEnabled);
        else
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setData(Qt::UserRole, f);
        item->setData(Qt::UserRole+1, path + QDir::separator());
        item->setText(f);
        if (checkable)
            item->setCheckState(Qt::Checked);
        input_->addItem(item);
    }
}

void BulkRenameDialog::fillOutputList_()
{
    output_->clear();
    for (int i=0; i<input_->count(); ++i)
    {
        auto iitem = input_->item(i);
        const QString fn = rename_( iitem->data(Qt::UserRole).toString(), i );

        output_->addItem(fn);
    }
}

void BulkRenameDialog::optionsChanged_()
{
    fillOutputList_();
}

bool BulkRenameDialog::canRename_()
{
    return input_->count();
}

QString BulkRenameDialog::rename_(const QString &fn, int index) const
{
    const QFileInfo f(fn);
    const QString ext = QFileInfo(fn).completeSuffix();

    QString s = subStr_->text();
    s = s.replace("%name", f.baseName());
    //s = s.replace("%.ext", "." + ext);
    s = s.replace("%ext", ext);
    s = s.replace("%n",
            QString("%1").arg(index + seqStart_->value(),
                              digitCount_->value(), 10, QChar('0')));

    return s;
}

void BulkRenameDialog::rename_()
{
    if (!canRename_())
        return;

    QTextCursor tex = log_->textCursor();
    tex.insertText(tr("Starting renaming") + "\n");

    for (int i=0; i<input_->count(); ++i)
    {
        QListWidgetItem * item = input_->item(i);
        if (item->checkState() != Qt::Checked)
            continue;

        const QString path = item->data(Qt::UserRole+1).toString();
        const QString filename = item->data(Qt::UserRole).toString();
        const QString newfilename = rename_( filename, i );

        QFileInfo inf(newfilename);
        if (inf.exists())
        {
            tex.insertText("skipping existing file " + newfilename + "\n");
            continue;
        }

        tex.insertText("renaming " + path + filename + " to " + newfilename + "\n");

        if (!QFile::rename(path + filename, path + newfilename))
        {
            tex.insertText("could not rename " + filename + "\n");
            continue;
        }

        item->setCheckState(Qt::Unchecked);
    }
}

} // namespace GUI
} // namespace MO
