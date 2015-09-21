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

#include "distancefieldimage.h"
#include "gui/propertiesview.h"
#include "types/properties.h"
#include "tool/dfdownsampler.h"
#include "io/imagereader.h"
#include "io/files.h"
#include "io/settings.h"

namespace MO {
namespace GUI {

struct DistanceFieldImage::Private
{
    Private(DistanceFieldImage * d)
        : win       (d)
        , thread    (0)
    { }

    void createProperties();
    void createWidgets();
    void createMenu();
    void loadImage(const QString& fn);
    void render();
    void getImage();

    DistanceFieldImage * win;

    Properties props;
    QImage inImage, outImage;
    DFDownsampler * thread;

    PropertiesView * propView;
    QProgressBar * progBar;
    QLabel * lImageIn, * lImageOut;
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

void DistanceFieldImage::Private::createProperties()
{
    props.set("res", tr("resolution"), tr("The downsampled resolution"),
              QSize(128, 128));
    props.set("range", tr("scan range"),
              tr("The maximum range in pixels to scan for distances"),
              QSize(256, 256));
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
            //lImageIn->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
            //lImageIn->setScaledContents(true);
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

void DistanceFieldImage::loadSourceImage()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_TEXTURE, this);
    if (fn.isEmpty())
        return;

    p_->loadImage(fn);
}

void DistanceFieldImage::threadFinsihed_()
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

void DistanceFieldImage::Private::render()
{
    if (!thread)
    {
        thread = new DFDownsampler(win);
        connect(thread, SIGNAL(finished()), win, SLOT(threadFinsihed_()), Qt::QueuedConnection);
        connect(thread, SIGNAL(progressChanged(float)),
                win, SLOT(setProgress_(float)), Qt::QueuedConnection);
    }
    if (thread->isRunning())
        thread->stop();

    props = propView->properties();
    thread->setInputImage(inImage);
    thread->setOutputResolution(props.get("res").toSize());
    thread->setSamplingRange(props.get("range").toSize());

    thread->start();
}

void DistanceFieldImage::Private::getImage()
{
    if (!thread)
        return;

    outImage = thread->getOutputImage();
    lImageOut->setPixmap(QPixmap::fromImage(outImage));
    lImageOut->adjustSize();
}


} // namespace GUI
} // namespace MO
