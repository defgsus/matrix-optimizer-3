/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/22/2016</p>
*/

#ifndef MOSRC_TOOL_GENERALIMAGE_H
#define MOSRC_TOOL_GENERALIMAGE_H

#include <QImage>
#include <QString>

/** An image generator for commonly used images */
class GeneralImage
{
public:

    static QImage getTextImage(const QString& text,
                               const QColor& textCol = QColor(255,255,255),
                               const QColor& backCol = QColor(0,0,0),
                               const QSize& res = QSize(512, 512),
                               QImage::Format format = QImage::Format_ARGB32_Premultiplied);
    static void getTextImage(QImage& img, const QString& text,
                             const QColor& textCol = QColor(255,255,255),
                             const QColor& backCol = QColor(0,0,0));

    static QImage getErrorImage(const QString& text,
                                const QSize& res = QSize(512, 512),
                                QImage::Format format = QImage::Format_ARGB32_Premultiplied);
    static void getErrorImage(QImage& img, const QString& text);
};

#endif // MOSRC_TOOL_GENERALIMAGE_H
