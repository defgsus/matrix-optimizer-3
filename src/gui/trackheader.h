/** @file trackheader.h

    @brief Track/Names area working with TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#ifndef MOSRC_GUI_TRACKHEADER_H
#define MOSRC_GUI_TRACKHEADER_H

#include <QWidget>

namespace MO {
class Track;
namespace GUI {

class TrackHeader : public QWidget
{
    Q_OBJECT
public:
    explicit TrackHeader(QWidget *parent = 0);

signals:

public slots:

};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TRACKHEADER_H
