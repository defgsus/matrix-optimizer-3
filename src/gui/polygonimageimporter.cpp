/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/22/2015</p>
*/

#include <QLayout>
#include <QToolButton>
#include <QProgressBar>
#include <QImage>
#include <QLabel>
#include <QScrollArea>
#include <QCloseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QImageWriter>
#include <QDebug>

#include "polygonimageimporter.h"
#include "gui/propertiesview.h"
#include "types/properties.h"
#include "geom/shploader.h"
#include "io/settings.h"
#include "io/files.h"


namespace MO {
namespace GUI {

struct PolygonImageImporter::Private
{
    Private(PolygonImageImporter * d)
        : win       (d)
        , isChanged (false)
    { }

    void createProperties();
    void createWidgets();
    void loadSHP(const QString& fn);
    void render();
    void saveImage(const QString& fn);

    bool isSaveToChange();

    PolygonImageImporter * win;

    Properties props;
    QImage outImage;

    PropertiesView * propView;
    QProgressBar * progBar;
    QLabel * lImageOut, * lInfo;
    bool isChanged;

    QVector<QPolygonF> polygons;
    QRectF boundingRect;
};


PolygonImageImporter::PolygonImageImporter(QWidget * parent, Qt::WindowFlags f)
    : QDialog       (parent, f)
    , p_            (new Private(this))
{
    setObjectName("_PolygonImageImporter");
    setWindowTitle(tr("Import polygon as image"));
    setMinimumSize(640, 480);

    settings()->restoreGeometry(this);

    p_->createProperties();
    p_->createWidgets();
}

PolygonImageImporter::~PolygonImageImporter()
{
    settings()->storeGeometry(this);

    delete p_;
}

void PolygonImageImporter::closeEvent(QCloseEvent * e)
{
    if (!p_->isSaveToChange())
        e->ignore();
    else
        e->accept();
}

void PolygonImageImporter::Private::createProperties()
{
    props.set("res", tr("resolution"), tr("Resolution of the output image"),
              QSize(1024, 1024));
    props.set("color", tr("foreground color"), tr("Color of the polygon shape"),
              QColor(255, 255, 255, 255));
    props.set("back_color", tr("background color"), tr("Color of the background"),
              QColor(0, 0, 0, 255));
    props.set("scale", tr("scale"), tr("The scale factor for x and y"),
              QVector<float>() << 1.f << 1.f);
}


void PolygonImageImporter::Private::createWidgets()
{
    auto lv = new QVBoxLayout(win);
    lv->setMargin(1);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QToolButton(win);
            but->setText(tr("&Load SHP"));
            lh->addWidget(but);
            connect(but, SIGNAL(clicked(bool)), win, SLOT(loadSHP()));

            but = new QToolButton(win);
            but->setText(tr("&Render"));
            lh->addWidget(but);
            connect(but, SIGNAL(clicked(bool)), win, SLOT(render()));

            lh->addStretch();

        // output image
        auto scroll = new QScrollArea(win);
        lv->addWidget(scroll, 2);

        lImageOut = new QLabel(win);
        scroll->setWidget(lImageOut);

        // info label
        lInfo = new QLabel(win);
        lv->addWidget(lInfo);

        propView = new PropertiesView(win);
        propView->setProperties(props);
        lv->addWidget(propView);

        progBar = new QProgressBar(win);
        lv->addWidget(progBar);
}


void PolygonImageImporter::setProgress_(int p)
{
    p_->progBar->setValue(p);
}

bool PolygonImageImporter::Private::isSaveToChange()
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

void PolygonImageImporter::render()
{
    p_->render();
}

void PolygonImageImporter::loadSHP()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_SHAPEFILE, this);
    if (fn.isEmpty())
        return;

    p_->loadSHP(fn);
}

void PolygonImageImporter::saveOutputImage()
{
    QString fn = IO::Files::getSaveFileName(IO::FT_TEXTURE, this);
    if (fn.isEmpty())
        return;

    p_->saveImage(fn);
}

void PolygonImageImporter::Private::saveImage(const QString &fn)
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

void PolygonImageImporter::Private::loadSHP(const QString &fn)
{
    GEOM::ShpLoader loader;
    loader.loadFile(fn, [=](double pr)
    {
        QMetaObject::invokeMethod(win, "setProgress_",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, pr));
    });

    loader.getPolygons(polygons);

    boundingRect = loader.getBoundingRect();

    lInfo->setText(tr("bounding rect %1, %2 - %3, %4 (%5x%6); polygons %7")
                   .arg(boundingRect.left())
                   .arg(boundingRect.top())
                   .arg(boundingRect.right())
                   .arg(boundingRect.bottom())
                   .arg(boundingRect.width())
                   .arg(boundingRect.height())
                   .arg(polygons.size())
                   );
}

void PolygonImageImporter::Private::render()
{
    props = propView->properties();

    outImage = QImage(props.get("res").toSize(), QImage::Format_ARGB32);
    outImage.fill(props.get("back_color").value<QColor>());

    // prepare transformation
    QTransform t = QTransform::fromTranslate(outImage.width() / 2.,
                                             outImage.height() / 2.);
    auto scale = props.get("scale").value<QVector<float>>();
    t.scale(scale[0], -scale[1]);

    // prepare painter
    QPainter p;
    p.begin(&outImage);

    p.setTransform(t);

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(props.get("color").value<QColor>()));

    // render all polygons
    for (const auto & polygon : polygons)
        p.drawPolygon(polygon);

    p.end();

    lImageOut->setPixmap(QPixmap::fromImage(outImage));
    lImageOut->adjustSize();

    isChanged = true;
}



} // namespace GUI
} // namespace MO
