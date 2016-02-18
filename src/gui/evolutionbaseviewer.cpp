/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/17/2016</p>
*/

#include <QLayout>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QJsonDocument>
#include <QFile>
#include <QImageWriter>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include "evolutionbaseviewer.h"
#include "tool/evolutionbase.h"
#include "io/settings.h"
#include "io/files.h"
#include "io/error.h"

namespace MO {
namespace GUI {


EvolutionBaseViewer::EvolutionBaseViewer(QWidget *parent)
    : QDialog   (parent)
    , evo_      (0)
    , doRender_ (true)
{
    setObjectName("EvolutionBaseViewer");
    setWindowTitle(tr("Evolution viewer"));
    settings()->restoreGeometry(this);

    setMinimumSize(320, 320);

    auto lv = new QVBoxLayout(this);
    //lv->setMargin(0);

        label_ = new QLabel(this);
        label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        lv->addWidget(label_);

        image_ = new QImage();
}

EvolutionBaseViewer::~EvolutionBaseViewer()
{
    settings()->storeGeometry(this);

    if (evo_)
        evo_->releaseRef();
    delete image_;
}

void EvolutionBaseViewer::setSpecimen(const EvolutionBase* evo)
{
    if (evo == evo_)
        return;
    if (evo_)
        evo_->releaseRef();
    if (!evo)
        evo_ = nullptr;
    else
        evo_ = evo->createClone();
    doRender_ = true;
}

void EvolutionBaseViewer::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::RightButton)
    {
        auto menu = new QMenu(this);
        QAction* a;

        a = menu->addAction(tr("Save Json"));
        a->setShortcut(Qt::CTRL + Qt::Key_S);
        connect(a, &QAction::triggered, [=]() { saveJson(); });

        a = menu->addAction(tr("Save Image"));
        a->setShortcut(Qt::CTRL + Qt::Key_I);
        connect(a, &QAction::triggered, [=]() { saveImage(); });

        menu->popup(QCursor::pos());
    }
}

void EvolutionBaseViewer::paintEvent(QPaintEvent* p)
{
    if (image_->size() != label_->size()
        || doRender_)
    {
        *image_ = QImage(label_->size(), QImage::Format_ARGB32_Premultiplied);

        if (evo_)
            evo_->getImage(*image_);
        else
            image_->fill(Qt::black);

        doRender_ = false;
        label_->setPixmap(QPixmap::fromImage(*image_));
    }

    QDialog::paintEvent(p);
}

void EvolutionBaseViewer::saveJson()
{
    auto fn = IO::Files::getSaveFileName(IO::FT_EVOLUTION, this);
    if (!fn.isEmpty())
        saveJson(fn);

}


void EvolutionBaseViewer::saveImage()
{
    auto fn = IO::Files::getSaveFileName(IO::FT_TEXTURE, this);
    if (!fn.isEmpty())
        saveImage(fn);
}


void EvolutionBaseViewer::saveJson(const QString& fn)
{
    if (!evo_)
        return;

    try
    {
        evo_->saveJsonFile(fn);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("saving json failed"), e.what());
    }
}

void EvolutionBaseViewer::saveImage(const QString& fn)
{
    if (image_->isNull() || !evo_)
        return;

    try
    {
        evo_->saveImage(fn, *image_);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("saving image failed"), e.what());
    }
}

} // namespace GUI
} // namespace MO
