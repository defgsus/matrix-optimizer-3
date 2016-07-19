/** @file sceneconvertdialog.cpp

    @brief Batch scene converter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/25/2014</p>
*/

#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLayout>
#include <QDir>
#include <QTextEdit>

#include "SceneConvertDialog.h"
#include "io/Files.h"
#include "io/error.h"
#include "io/log.h"
#include "object/util/ObjectFactory.h"
#include "object/Scene.h"

namespace MO {
namespace GUI {

SceneConvertDialog::SceneConvertDialog(QWidget *parent) :
    QDialog(parent)
{
    setObjectName("_SceneConvertDialog");
    setWindowTitle(tr("Batch scene converter"));
    setMinimumSize(640,480);

    createWidgets_();
}

void SceneConvertDialog::createWidgets_()
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

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                outputPath_ = new QLineEdit(this);
                outputPath_->setReadOnly(true);
                lh2->addWidget(outputPath_);

                but = new QPushButton("...", this);
                lh2->addWidget(but);
                connect(but, SIGNAL(clicked()), this, SLOT(chooseOutputPath_()));

            output_ = new QListWidget(this);
            lv->addWidget(output_);

    log_ = new QTextEdit(this);
    lv0->addWidget(log_);
    log_->setReadOnly(true);

    butConv_ = new QPushButton(tr("Convert all"), this);
    lv0->addWidget(butConv_);
    butConv_->setDefault(true);
    butConv_->setEnabled(false);
    connect(butConv_, SIGNAL(clicked()), this, SLOT(convert_()));

    auto butClose = new QPushButton(tr("Close"), this);
    lv0->addWidget(butClose);
    connect(butClose, SIGNAL(clicked()), this, SLOT(accept()));

}


void SceneConvertDialog::chooseInputPath_()
{
    QString path = IO::Files::getDirectory(IO::FT_SCENE, this, false);
    if (path.isEmpty())
        return;

    inputPath_->setText(path);
    fillList_(input_, path, true);

    butConv_->setEnabled(canConvert_());
}

void SceneConvertDialog::chooseOutputPath_()
{
    QString path = IO::Files::getDirectory(IO::FT_SCENE, this, false);
    if (path.isEmpty())
        return;

    outputPath_->setText(path);
    fillList_(output_, path, false);

    butConv_->setEnabled(canConvert_());
}

void SceneConvertDialog::fillList_(QListWidget * list, const QString &path, bool checkable)
{
    list->clear();

    QStringList files;
    IO::Files::findFiles(IO::FT_SCENE, path, true, files);

    QDir dir(path);

    for (auto & f : files)
    {
        auto item = new QListWidgetItem(list);
        if (checkable)
            item->setFlags(Qt::ItemIsSelectable
                           | Qt::ItemIsUserCheckable
                           | Qt::ItemIsEnabled);
        else
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setData(Qt::UserRole, f);
        item->setData(Qt::UserRole+1, dir.relativeFilePath(f));
        item->setText(dir.relativeFilePath(f));
        if (checkable)
            item->setCheckState(Qt::Checked);
        list->addItem(item);
    }
}

bool SceneConvertDialog::canConvert_()
{
    return input_->count() && QDir(outputPath_->text()).exists()
            && inputPath_->text() != outputPath_->text();
}

void SceneConvertDialog::convert_()
{
    if (!canConvert_())
        return;

    QTextCursor tex = log_->textCursor();
    tex.insertText(tr("Starting conversion") + "\n");

    for (int i=0; i<input_->count(); ++i)
    {
        QListWidgetItem * item = input_->item(i);
        if (item->checkState() != Qt::Checked)
            continue;

        const QString filename = item->data(Qt::UserRole).toString();
        const QString newfilename = outputPath_->text() + QDir::separator()
                + item->data(Qt::UserRole+1).toString();

        if (QFileInfo(newfilename).exists())
        {
            tex.insertText("skipping existing file " + newfilename + "\n");
            continue;
        }

        tex.insertText("converting " + filename + "\n");

        QString newpath = QFileInfo(newfilename).absolutePath();
        if (!QDir(newpath).exists())
        {
            tex.insertText("creating directory " + newpath + "\n");
            QDir().mkpath(newpath);
        }

        Scene * scene = 0;

        try
        {
            scene = ObjectFactory::loadScene(filename);
        }
        catch (Exception & e)
        {
            tex.insertText("-> " + tr("loading failed") + " : " + e.what() + "\n\n");
            continue;
        }

        try
        {
            ObjectFactory::saveScene(newfilename, scene);
        }
        catch (Exception & e)
        {
            tex.insertText("-> " + tr("saving failed") + " : " + e.what() + "\n\n");
            continue;
        }

        if (scene)
            scene->releaseRef("scene convert complete");
        item->setCheckState(Qt::Unchecked);
    }

    fillList_(output_, outputPath_->text(), false);
}

} // namespace GUI
} // namespace MO
