/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/26/2015</p>
*/

#include <QFileInfo>
#include <QPixmap>
#include <QIcon>
#include <QDropEvent>
#include <QMimeData>

#include "ImageList.h"
#include "io/ImageReader.h"

namespace MO {
namespace GUI {


ImageList::ImageList(QWidget * p)
    : QListWidget   (p)
{
    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    setDragDropMode(InternalMove);

    setToolTip(tr("Drag files here to add\ndrag items to move"));
}

bool ImageList::addImage(const QString &fn)
{
    ImageReader read;
    read.setFilename(fn);
    QImage img = read.read();

    if (img.isNull())
    {
        p_errorString_ += read.errorString() + "\n";
        return false;
    }

    addImage(fn, img);
    return true;
}

void ImageList::addImage(const QString& fn, const QImage& img)
{
    auto item = new QListWidgetItem(this);
    item->setText(QFileInfo(fn).fileName());
    item->setIcon(QIcon(QPixmap::fromImage(img.scaled(QSize(64, 64),
                                                      Qt::KeepAspectRatio,
                                                      Qt::SmoothTransformation))));
    item->setData(Qt::UserRole, fn);
    item->setData(Qt::UserRole+1, QString("%1x%2")
                                    .arg(img.width())
                                    .arg(img.height())
                  );
    item->setData(Qt::UserRole+2, img);

    addItem(item);
}

void ImageList::dragEnterEvent(QDragEnterEvent * e)
{
    if (e->mimeData()->hasUrls())
    {
        e->setDropAction(Qt::CopyAction);
        e->accept();
        return;
    }

    QListWidget::dragEnterEvent(e);
}

void ImageList::dragMoveEvent(QDragMoveEvent * e)
{
    if (e->mimeData()->hasUrls())
    {
        e->accept();
        return;
    }

    QListWidget::dragMoveEvent(e);
}

void ImageList::dropEvent(QDropEvent * e)
{
    if (!e->mimeData()->hasUrls())
    {
        QListWidget::dropEvent(e);
        return;
    }

    QList<QUrl> list = e->mimeData()->urls();

    for (const QUrl& url : list)
    {
        const QString fn = url.toString(QUrl::PreferLocalFile);
        // try to read filename as image
        ImageReader read;
        read.setFilename(fn);
        QImage img = read.read();

        if (!img.isNull())
            addImage(fn, img);
    }
}

QStringList ImageList::filenames() const
{
    QStringList list;

    for (int i=0; i<count(); ++i)
    {
        const QString fn = item(i)->data(Qt::UserRole).toString();
        if (!fn.isEmpty())
            list << fn;
    }

    return list;
}


} // namespace GUI
} // namespace MO
