/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/26/2015</p>
*/

#include <QLayout>
#include <QLabel>
#include <QListWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QFileInfo>

#include "imagelistwidget.h"
#include "imagelist.h"
#include "io/imagereader.h"
#include "io/files.h"

namespace MO {
namespace GUI {


struct ImageListWidget::Private
{
    Private(ImageListWidget * w)
        : widget(w)
    { }

    void createWidgets();
    void addFiles(const QStringList& fn);
    void updateImageLabel();

    ImageListWidget * widget;
    ImageList * list;
    QLabel * lImage, * lInfo;
};


ImageListWidget::ImageListWidget(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    p_->createWidgets();
}

ImageListWidget::~ImageListWidget()
{
    delete p_;
}

void ImageListWidget::Private::createWidgets()
{
    auto lh = new QHBoxLayout(widget);

        auto lv = new QVBoxLayout();
        lh->addLayout(lv);

            list = new ImageList(widget);
            connect(list, &QListWidget::currentItemChanged, [=]() { updateImageLabel(); });
            lv->addWidget(list);

            auto lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                auto but = new QToolButton(widget);
                but->setText(tr("Clear"));
                lh2->addWidget(but);
                connect(but, SIGNAL(clicked(bool)), widget, SLOT(clearList()));

                but = new QToolButton(widget);
                but->setText(tr("Add files"));
                lh2->addWidget(but);
                connect(but, SIGNAL(clicked(bool)), widget, SLOT(addImages()));

                lh2->addStretch();

        lv = new QVBoxLayout();
        lh->addLayout(lv);

            lImage = new QLabel(widget);
            lImage->setFixedWidth(256);
            lv->addWidget(lImage);

            lInfo = new QLabel(widget);
            lv->addWidget(lInfo);

            lv->addStretch();
}

QStringList ImageListWidget::imageList() const
{
    return p_->list->filenames();
}

void ImageListWidget::setImageList(const QStringList& l)
{
    clearList();
    p_->addFiles(l);
}

void ImageListWidget::clearList()
{
    p_->list->clear();
}

void ImageListWidget::addImages()
{
    QStringList fn = IO::Files::getOpenFileNames(IO::FT_TEXTURE, this);
    p_->addFiles(fn);
}

void ImageListWidget::Private::addFiles(const QStringList &files)
{
    list->clearErrors();
    for (auto & fn : files)
    {
        list->addImage(fn);
    }

    if (list->hasError())
    {
        QMessageBox::critical(widget, tr("load images"),
                              tr("ERROR: %1").arg(list->errorString()));
    }
}

void ImageListWidget::Private::updateImageLabel()
{
    auto item = list->currentItem();
    if (!item)
    {
        lInfo->clear();
        lImage->clear();
        return;
    }

    lImage->setPixmap(QPixmap::fromImage(
                          item->data(Qt::UserRole+2).value<QImage>())
                            .scaled(QSize(256, 256),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
    lInfo->setText(item->data(Qt::UserRole+1).toString());
}


} // namespace GUI
} // namespace MO
