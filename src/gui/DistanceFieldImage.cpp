/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/21/2015</p>
*/

#include <QLayout>
#include <QLabel>
#include <QScrollArea>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QImageWriter>
#include <QCloseEvent>

#include "DistanceFieldImage.h"
#include "gui/PropertiesView.h"
#include "types/Properties.h"
#include "tool/DfDownsampler.h"
#include "io/ImageReader.h"
#include "io/Files.h"
#include "io/Settings.h"

namespace MO {
namespace GUI {

struct DistanceFieldImage::Private
{
    Private(DistanceFieldImage * d)
        : win       (d)
        , thread    (0)
        , isChanged (false)
    { }

    void createProperties();
    void createWidgets();
    void createMenu();
    void loadImage(const QString& fn);
    void saveImage(const QString& fn);
    void render();
    void getImage();

    bool isSaveToChange();

    DistanceFieldImage * win;

    Properties props;
    QImage inImage, outImage;
    DFDownsampler * thread;

    PropertiesView * propView;
    QProgressBar * progBar;
    QLabel * lImageIn, * lImageOut;
    bool isChanged;
};


DistanceFieldImage::DistanceFieldImage(QWidget * parent, Qt::WindowFlags f)
    : QMainWindow   (parent, f)
    , p_            (new Private(this))
{
    setObjectName("_DistanceFieldImageWindow");
    setWindowTitle(tr("Distance field image downsampler"));
    setMinimumSize(640, 480);

    settings()->restoreGeometry(this);

    p_->createProperties();
    p_->createWidgets();
    p_->createMenu();
}

DistanceFieldImage::~DistanceFieldImage()
{
    stop();

    settings()->storeGeometry(this);

    delete p_;
}

void DistanceFieldImage::closeEvent(QCloseEvent * e)
{
    if (!p_->isSaveToChange())
        e->ignore();
    else
        e->accept();
}

void DistanceFieldImage::Private::createProperties()
{
    props.set("res", tr("resolution"), tr("The downsampled resolution"),
              QSize(64, 64));
    props.set("range", tr("scan range"),
              tr("The maximum range in pixels to scan for distances"),
              QSize(256, 256));
    props.set("threshold", tr("threshold"),
              tr("A pixel is considered inside if it is >= this value [0,255]"),
              128, 0, 255);
}

void DistanceFieldImage::Private::createWidgets()
{
    auto cw = new QWidget(win);
    win->setCentralWidget(cw);

    auto lv = new QVBoxLayout(cw);
    lv->setMargin(1);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh, 1);

            // input image
            auto scroll = new QScrollArea(win);
            lh->addWidget(scroll);

            lImageIn = new QLabel(win);
            scroll->setWidget(lImageIn);

            // output image
            scroll = new QScrollArea(win);
            lh->addWidget(scroll);

            lImageOut = new QLabel(win);
            scroll->setWidget(lImageOut);

        propView = new PropertiesView(win);
        propView->setProperties(props);
        lv->addWidget(propView);

        progBar = new QProgressBar(win);
        lv->addWidget(progBar);
}

void DistanceFieldImage::Private::createMenu()
{
    QMenu * m;
    QAction * a;

    win->menuBar()->addMenu( m = new QMenu(tr("File"), win) );

        a = m->addAction(tr("Load source image"));
        a->setShortcut(Qt::CTRL + Qt::Key_L);
        connect(a, SIGNAL(triggered(bool)), win, SLOT(loadSourceImage()));

        a = m->addAction(tr("Save output image"));
        a->setShortcut(Qt::CTRL + Qt::Key_S);
        connect(a, SIGNAL(triggered(bool)), win, SLOT(saveOutputImage()));

    win->menuBar()->addMenu( m = new QMenu(tr("Render"), win) );

        a = m->addAction(tr("Start"));
        a->setShortcut(Qt::Key_F7);
        connect(a, SIGNAL(triggered(bool)), win, SLOT(start()));

        a = m->addAction(tr("Stop"));
        a->setShortcut(Qt::Key_F8);
        connect(a, SIGNAL(triggered(bool)), win, SLOT(stop()));

}

void DistanceFieldImage::start()
{
    if (!p_->inImage.isNull())
        p_->render();
}

void DistanceFieldImage::stop()
{
    if (p_->thread)
        p_->thread->stop();
}

bool DistanceFieldImage::isChanged() const
{
    return p_->isChanged;
}

bool DistanceFieldImage::Private::isSaveToChange()
{
    if (!isChanged)
        return true;

    int a = QMessageBox::question(win, tr("Save image?"),
                                  tr("The current output is not saved.\nSave it now?"),
                                  tr("Yes"), tr("No"), tr("Cancel"));
    if (a == 2)
        return false;

    if (a == 0)
        win->saveOutputImage();

    return true;
}

void DistanceFieldImage::loadSourceImage()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_TEXTURE, this);
    if (fn.isEmpty())
        return;

    p_->loadImage(fn);
}

void DistanceFieldImage::saveOutputImage()
{
    QString fn = IO::Files::getSaveFileName(IO::FT_TEXTURE, this);
    if (fn.isEmpty())
        return;

    p_->saveImage(fn);
}

void DistanceFieldImage::threadFinished_()
{
    p_->getImage();
}

void DistanceFieldImage::setProgress_(float p)
{
    p_->progBar->setValue(p);
}

void DistanceFieldImage::Private::loadImage(const QString &fn)
{
    ImageReader read;
    read.setFilename(fn);
    QImage img = read.read();
    if (img.isNull())
    {
        QMessageBox::critical(win, tr("load source image"),
                              tr("Error: %1").arg(read.errorString()));
        return;
    }

    inImage = img;
    lImageIn->setPixmap(QPixmap::fromImage(inImage));
    lImageIn->adjustSize();
}

void DistanceFieldImage::Private::saveImage(const QString &fn)
{
    QImageWriter write(fn);
    if (!write.write(outImage))
    {
        QMessageBox::critical(win, tr("save output image"),
                              tr("Error: %1").arg(write.errorString()));
    }
    else
        isChanged = false;
}

void DistanceFieldImage::Private::render()
{
    if (!thread)
    {
        thread = new DFDownsampler(win);
        connect(thread, SIGNAL(finished()), win, SLOT(threadFinished_()), Qt::QueuedConnection);
        connect(thread, SIGNAL(progressChanged(float)),
                win, SLOT(setProgress_(float)), Qt::QueuedConnection);
    }
    if (thread->isRunning())
        thread->stop();

    props = propView->properties();
    thread->setInputImage(inImage);
    thread->setOutputResolution(props.get("res").toSize());
    thread->setSamplingRange(props.get("range").toSize());
    thread->setThreshold(props.get("threshold").toUInt());

    thread->start();
}

void DistanceFieldImage::Private::getImage()
{
    if (!thread)
        return;

    outImage = thread->getOutputImage();
    lImageOut->setPixmap(QPixmap::fromImage(outImage));
    lImageOut->adjustSize();

    isChanged = true;
}


} // namespace GUI
} // namespace MO
