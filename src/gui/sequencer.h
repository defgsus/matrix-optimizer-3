/** @file sequencer.h

    @brief Sequencer using TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCER_H
#define MOSRC_GUI_SEQUENCER_H

#include <QWidget>

class QGridLayout;

namespace MO {
namespace GUI {

class TrackView;
class Ruler;

class Sequencer : public QWidget
{
    Q_OBJECT
public:
    explicit Sequencer(QWidget *parent = 0);

signals:

public slots:

private:

    void createWidgets_();

    Ruler * rulerSec_, * rulerFps_;
    TrackView * trackView_;

    QGridLayout * gridLayout_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_SEQUENCER_H
