/** @file audiolinkviewoverpaint.h

    @brief Draws cables and stuff ontop of AudioLinkView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef MOSRC_GUI_PAINTER_AUDIOLINKVIEWOVERPAINT_H
#define MOSRC_GUI_PAINTER_AUDIOLINKVIEWOVERPAINT_H

#include <QWidget>
#include <QPen>

namespace MO {
namespace GUI {

class AudioLinkView;

class AudioLinkViewOverpaint : public QWidget
{
    Q_OBJECT
public:
    explicit AudioLinkViewOverpaint(AudioLinkView * parent);

signals:

public slots:

protected:

    void paintEvent(QPaintEvent *);

    AudioLinkView * view_;

};



} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_PAINTER_AUDIOLINKVIEWOVERPAINT_H
