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

#include "scenedescdialog.h"


namespace MO {
namespace GUI {

SceneDescDialog::SceneDescDialog(QWidget *parent)
    : QDialog       (parent)
{
    setObjectName("_SceneDescDialog");
    setWindowTitle(tr("Scene description"));

    setMinimumSize(640, 640);


    auto lv = new QVBoxLayout(this);

        p_edit_ = new QPlainTextEdit(this);
        lv->addWidget(p_edit_);

        p_cb_ = new QCheckBox(tr("Show when opening"), this);
        lv->addWidget(p_cb_);

        auto lh = new QHBoxLayout(this);
        lv->addLayout(lh);

            auto but = new QPushButton(tr("Save"), this);
            but->setDefault(true);
            connect(but, SIGNAL(clicked()), this, SLOT(accept()));
            lh->addWidget(but);

            but = new QPushButton(tr("Cancel"), this);
            connect(but, SIGNAL(clicked()), this, SLOT(reject()));
            lh->addWidget(but);
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
}

void SceneDescDialog::setShowOnStart(bool e)
{
    p_cb_->setChecked(e);
}


} // namespace GUI
} // namespace MO
