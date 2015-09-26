/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/26/2015</p>
*/

#include <QLayout>
#include <QPushButton>
#include <QFrame>

#include "imagelistdialog.h"
#include "gui/widget/imagelistwidget.h"
#include "io/settings.h"

namespace MO {
namespace GUI {


ImageListDialog::ImageListDialog(
        bool showUB, QWidget * parent, Qt::WindowFlags f)
    : QDialog   (parent, f)
{
    setObjectName("_ImageListDialog");
    setWindowTitle(tr("Image list"));

    settings()->restoreGeometry(this);


    // --- widgets ---

    auto lv = new QVBoxLayout(this);

        list = new ImageListWidget(this);
        list->layout()->setMargin(0);
        lv->addWidget(list);

        auto frame = new QFrame(this);
        frame->setFrameShape(QFrame::HLine);
        lv->addWidget(frame);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QPushButton(tr("Ok"), this);
            but->setDefault(true);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked(bool)), this, SLOT(accept()));

            if (showUB)
            {
                but = new QPushButton(tr("Update"), this);
                lh->addWidget(but);
                connect(but, SIGNAL(clicked(bool)), this, SIGNAL(listChanged()));
            }

            but = new QPushButton(tr("Cancel"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked(bool)), this, SLOT(reject()));

            lh->addStretch();
}

ImageListDialog::~ImageListDialog()
{
    settings()->storeGeometry(this);
}

QStringList ImageListDialog::imageList() const
{
    return list->imageList();
}

void ImageListDialog::setImageList(const QStringList& l)
{
    list->setImageList(l);
}


} // namespace GUI
} // namespace MO
