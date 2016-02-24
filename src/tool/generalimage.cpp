/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/22/2016</p>
*/

#include <QPainter>
#include <QFont>
#include <QFontMetrics>

#include "generalimage.h"





QImage GeneralImage::getTextImage(const QString &text,
                                  const QColor& textCol, const QColor& backCol,
                                  const QSize &res, QImage::Format format)
{
    QImage img(res, format);
    getTextImage(img, text, textCol, backCol);
    return img;
}

void GeneralImage::getTextImage(QImage &img, const QString &text,
                                const QColor& textCol, const QColor& backCol)
{
    img.fill(backCol);

    // get font (to change size)
    QFont font;

    // -- choose size --

    font.setPixelSize(1);
    QRectF irect = img.rect();
    int flags = Qt::AlignHCenter | Qt::AlignVCenter;
    QRectF br = QFontMetricsF(font).boundingRect(irect, flags, text);

    if (br.width() < .1)
        br.setWidth(.1);
    if (br.height() < .1)
        br.setHeight(.1);
    double si = std::min(irect.width() / br.width(),
                           irect.height() / br.height());

    font.setPixelSize(std::max(1, int(si)));

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setFont(font);

    const QPen txtpen = QPen(textCol);

    // text
    p.setPen(txtpen);
    p.drawText(irect, flags, text);

}


QImage GeneralImage::getErrorImage(const QString &text,
                                 const QSize &res, QImage::Format format)
{
    QImage img(res, format);
    getErrorImage(img, text);
    return img;
}

void GeneralImage::getErrorImage(QImage &img, const QString &text)
{
    getTextImage(img, text,
                 QColor(255, 128, 128),
                 QColor(128, 0, 0));
}
