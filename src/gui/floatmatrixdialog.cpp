/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/18/2016</p>
*/

#include <QLayout>
#include <QPushButton>

#include "floatmatrixdialog.h"
#include "widget/floatmatrixwidget.h"
#include "io/settings.h"

namespace MO {
namespace GUI {


FloatMatrixDialog::FloatMatrixDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName("FloatMatrixDialog");
    setWindowTitle(tr("Float Matrix"));

    settings()->restoreGeometry(this);

    auto lv = new QVBoxLayout(this);

        p_widget_ = new FloatMatrixWidget(this);
        lv->addWidget(p_widget_);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QPushButton(tr("Ok"), this);
            connect(but, &QPushButton::clicked, [=]()
            {
                emit matrixChanged();
                accept();
            });
            lh->addWidget(but);

            but = new QPushButton(tr("Up&date"), this);
            but->setShortcut(Qt::CTRL + Qt::Key_Return);
            connect(but, &QPushButton::clicked, [=]()
            {
                emit matrixChanged();
            });
            lh->addWidget(but);

            lh->addStretch(1);

            but = new QPushButton(tr("Revert"), this);
            connect(but, &QPushButton::clicked, [=]()
            {
                p_widget_->setFloatMatrix(p_backup_);
                emit matrixChanged();
            });
            lh->addWidget(but);

            but = new QPushButton(tr("Cancel"), this);
            connect(but, &QPushButton::clicked, [=]()
            {
                p_widget_->setFloatMatrix(p_backup_);
                emit matrixChanged();
                reject();
            });
            lh->addWidget(but);

}

FloatMatrixDialog::~FloatMatrixDialog()
{
    settings()->storeGeometry(this);
}

const FloatMatrix& FloatMatrixDialog::floatMatrix() const
{
    return p_widget_->floatMatrix();
}

void FloatMatrixDialog::setFloatMatrix(const FloatMatrix& m)
{
    p_widget_->setFloatMatrix(m);
    p_backup_ = m;
}




} // namespace GUI
} // namespace MO
