/** @file commonresultions.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.10.2014</p>
*/

#include <QMenu>
#include <QActionGroup>

#include "commonresolutions.h"

/* http://en.wikipedia.org/wiki/List_of_common_resolutions */

namespace MO {

const QList<CommonResolutions::Resolution> CommonResolutions::resolutions =
{
    Resolution(QSize(320, 200),     QSizeF(8,5),   "CGA"),
    Resolution(QSize(320, 240),     QSizeF(4,3),   "QVGA"),
    Resolution(QSize(640, 400),     QSizeF(8,5),   "Atari ST"),
    Resolution(QSize(640, 480),     QSizeF(4,3),   "VGA"),
    Resolution(QSize(800, 600),     QSizeF(4,3),   "SVGA"),
    Resolution(QSize(1280, 768),    QSizeF(5,3),   "WXGA"),
    Resolution(QSize(1280, 960),    QSizeF(4,3),   "SXGA-"),
    Resolution(QSize(1280, 1024),   QSizeF(5,4),   "SXGA"),
    Resolution(QSize(1334, 750),    QSizeF(667,375),"iPhone 6"),

    Resolution(QSize(576, 480),     QSizeF(4,3),   "SVCD PAL"),
    Resolution(QSize(576, 520),     QSizeF(4,3),   "PAL/SECAM"),
    Resolution(QSize(720, 480),     QSizeF(5,3),   "DVD NTSC"),
    Resolution(QSize(720, 576),     QSizeF(5,4),   "DVD PAL"),
    Resolution(QSize(848, 480),     QSizeF(53,30), "Wide PAL"),
    Resolution(QSize(1024, 576),    QSizeF(16,9),  "576p"),
    Resolution(QSize(1280, 720),    QSizeF(16,9),  "720p"),
    Resolution(QSize(1440, 1080),   QSizeF(4,3),   "1080i HDV"),
    Resolution(QSize(1600, 900),    QSizeF(16,9),  "900p"),
    Resolution(QSize(1920, 1080),   QSizeF(16,9),  "1080i FullHD"),

    Resolution(QSize(1024, 1024),   QSizeF(1,1),   "1k"),
    Resolution(QSize(2048, 2048),   QSizeF(1,1),   "2k"),
    Resolution(QSize(3072, 3072),   QSizeF(1,1),   "3k"),
    Resolution(QSize(4096, 4096),   QSizeF(1,1),   "4k"),
    Resolution(QSize(5120, 5120),   QSizeF(1,1),   "5k"),
    Resolution(QSize(6144, 6144),   QSizeF(1,1),   "6k"),
    Resolution(QSize(7168, 7168),   QSizeF(1,1),   "7k"),
    Resolution(QSize(8192, 8192),   QSizeF(1,1),   "8k")
};

QString CommonResolutions::Resolution::descriptiveName() const
{
    if (size_.width() == size_.height())
        return QString("%1 ² (%2)")
                .arg(size_.width())
                .arg(name_);
    else
        return QString("%1x%2 %3:%4 (%5)")
                .arg(size_.width()).arg(size_.height())
                .arg(ratio_.width()).arg(ratio_.height())
                .arg(name_);
}


CommonResolutions::CommonResolutions()
{
}


void CommonResolutions::addResolutionActions(QMenu * m, bool checkable)
{
    QActionGroup * ag = 0;
    if (checkable)
        ag = new QActionGroup(m);

    int k=0;
    for (auto & r : resolutions)
    {
        QAction * a = new QAction(m);
        a->setText(r.descriptiveName());
        a->setStatusTip(r.descriptiveName());
        a->setData(k);
        a->setCheckable(checkable);
        if (ag)
            ag->addAction(a);

        m->addAction(a);

        ++k;
    }
}

const CommonResolutions::Resolution * CommonResolutions::findResolution(const QSize & size)
{
    for (auto & r : resolutions)
        if (r.size() == size)
            return &r;
    return 0;
}

} // namespace MO
