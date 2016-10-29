/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/18/2016</p>
*/

#include <QLayout>
#include <QPushButton>
#include <QCloseEvent>
#include <QMessageBox>

#include "FloatMatrixDialog.h"
#include "widget/FloatMatrixWidget.h"
#include "object/Object.h"
#include "object/interface/ValueFloatMatrixInterface.h"
#include "io/Settings.h"

namespace MO {
namespace GUI {


FloatMatrixDialog::FloatMatrixDialog(QWidget *parent)
    : QDialog       (parent)
    , p_readOnly_   (false)
    , p_isChanged_  (true)
{
    setObjectName("FloatMatrixDialog");
    setWindowTitle(tr("Float Matrix"));

    settings()->restoreGeometry(this);

    auto lv = new QVBoxLayout(this);

        p_widget_ = new FloatMatrixWidget(this);
        lv->addWidget(p_widget_);
        connect(p_widget_, &FloatMatrixWidget::matrixChanged, [=]()
        {
            p_setChanged(true);
            //emit matrixChanged();
        });

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);


            auto but = p_butRevert_ = new QPushButton(tr("Revert"), this);
            connect(but, &QPushButton::clicked, [=]()
            {
                p_widget_->setFloatMatrix(p_backup_);
                p_setChanged(false);
                emit matrixChanged();
            });
            lh->addWidget(but);

            but = p_butCancel_ = new QPushButton(tr("Cancel"), this);
            connect(but, &QPushButton::clicked, [=]()
            {
                p_widget_->setFloatMatrix(p_backup_);
                emit matrixChanged();
                reject();
            });
            lh->addWidget(but);


            lh->addStretch(1);

            but = p_butOk_ = new QPushButton(tr("Ok"), this);
            connect(but, &QPushButton::clicked, [=]()
            {
                emit matrixChanged();
                accept();
            });
            lh->addWidget(but);

            but = p_butUpdate_ = new QPushButton(tr("Up&date"), this);
            but->setShortcut(Qt::CTRL + Qt::Key_Return);
            connect(but, &QPushButton::clicked, [=]()
            {
                p_setChanged(false);
                emit matrixChanged();
            });
            lh->addWidget(but);

    p_setChanged(false);
}

FloatMatrixDialog::~FloatMatrixDialog()
{
    settings()->storeGeometry(this);
}

void FloatMatrixDialog::closeEvent(QCloseEvent* e)
{
    if (p_isChanged_)
    {
        int r = QMessageBox::question(this, tr("Discard Changes?"),
            tr("The Matrix data has changed. Discard?"),
            tr("Discard"), tr("Cancel"));
        if (r == 1)
        {
            e->ignore();
            return;
        }
    }
    QDialog::closeEvent(e);
}

const FloatMatrix& FloatMatrixDialog::floatMatrix() const
{
    return p_widget_->floatMatrix();
}

void FloatMatrixDialog::setFloatMatrix(const FloatMatrix& m)
{
    p_widget_->setFloatMatrix(m);
    p_backup_ = m;
    p_setChanged(false);
}

void FloatMatrixDialog::setReadOnly(bool e)
{
    p_readOnly_ = e;
    p_widget_->setReadOnly(p_readOnly_);
    p_butRevert_->setVisible(!p_readOnly_);
    p_butCancel_->setVisible(!p_readOnly_);
    p_butUpdate_->setVisible(!p_readOnly_);
    p_butOk_->setText(p_readOnly_ ? tr("Close") : tr("Ok"));
}

void FloatMatrixDialog::p_setChanged(bool e)
{
    if (e == p_isChanged_)
        return;
    p_isChanged_ = e;

    p_butRevert_->setEnabled(p_isChanged_);
    p_butUpdate_->setEnabled(p_isChanged_);
}

FloatMatrixDialog* FloatMatrixDialog::openForInterface(
        ValueFloatMatrixInterface *iface,
        const RenderTime& time, uint channel,
        QWidget* parent)
{
    auto diag = new FloatMatrixDialog(parent);
    if (auto o = dynamic_cast<Object*>(iface))
        diag->setWindowTitle(o->name() + " " + diag->windowTitle());
    diag->setAttribute(Qt::WA_DeleteOnClose, true);
    diag->setModal(false);
    diag->setReadOnly(true);

    diag->setFloatMatrix(iface->valueFloatMatrix(channel, time));

    diag->show();

    return diag;
}


} // namespace GUI
} // namespace MO
