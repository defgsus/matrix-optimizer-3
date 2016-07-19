/** @file scenedescdialog.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.04.2015</p>
*/

#include <QPlainTextEdit>
#include <QLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QCloseEvent>
#include <QMessageBox>

#include "SceneDescDialog.h"


namespace MO {
namespace GUI {

SceneDescDialog::SceneDescDialog(QWidget *parent)
    : QDialog       (parent)
    , p_changed_    (false)
{
    setObjectName("_SceneDescDialog");
    setWindowTitle(tr("Scene description"));

    setMinimumSize(640, 640);


    auto lv = new QVBoxLayout(this);

        p_edit_ = new QPlainTextEdit(this);
        connect(p_edit_, &QPlainTextEdit::textChanged, [=](){ setChanged_(); } );
        lv->addWidget(p_edit_);

        p_cb_ = new QCheckBox(tr("Show when opening"), this);
        connect(p_cb_, &QCheckBox::clicked, [=](){ setChanged_(); } );
        lv->addWidget(p_cb_);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QPushButton(tr("Save"), this);
            p_butSave_ = but;
            but->setDefault(true);
            connect(but, SIGNAL(clicked()), this, SLOT(accept()));
            lh->addWidget(but);

            p_butClose_ = but = new QPushButton(this);
            connect(but, SIGNAL(clicked()), this, SLOT(reject()));
            lh->addWidget(but);

    updateButtons_();
}


QString SceneDescDialog::text() const
{
    return p_edit_->toPlainText();
}

bool SceneDescDialog::showOnStart() const
{
    return p_cb_->isChecked();
}

void SceneDescDialog::setText(const QString &text)
{
    p_edit_->setPlainText(text);
    p_changed_ = false;
    updateButtons_();
}

void SceneDescDialog::setShowOnStart(bool e)
{
    p_cb_->setChecked(e);
}

void SceneDescDialog::setChanged_()
{
    if (p_changed_)
        return;
    p_changed_ = true;
    updateButtons_();
}

void SceneDescDialog::updateButtons_()
{
    if (p_changed_)
    {
        p_butSave_->setVisible(true);
        p_butClose_->setText(tr("Cancel"));
    }
    else
    {
        p_butSave_->setVisible(false);
        p_butClose_->setText(tr("Close"));
    }
}

void SceneDescDialog::keyPressEvent(QKeyEvent * e)
{
    /* By default, the escape key is not passed to the close() function.
       It just closes the dialog and bypasses the save-changes-dialog.
       Which is really odd, dear Qt-Team! */
    if (e->key() == Qt::Key_Escape)
    {
        e->accept();
        close();
    }
    else
        QDialog::keyPressEvent(e);
}

void SceneDescDialog::closeEvent(QCloseEvent * e)
{
    if (p_changed_)
    {
        int r = QMessageBox::question(this, tr("Close scene description"),
                              tr("Do you want to save the changes?"),
                              tr("Yes"), tr("No"), tr("Cancel"), 2);
        if (r == 0)
            accept();
        else
        if (r == 1)
        {
            e->accept();
            reject();
        }
        else
            e->ignore();
    }
    else
        e->accept();
}


} // namespace GUI
} // namespace MO
