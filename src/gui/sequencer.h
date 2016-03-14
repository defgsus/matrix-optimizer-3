/** @file sequencer.h

    @brief Sequencer using TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCER_H
#define MOSRC_GUI_SEQUENCER_H

#include <QWidget>
#include <QMap>

#include "types/float.h"

class QGridLayout;
class QScrollBar;

namespace MO {
class Track;
class Object;
class Sequence;
namespace GUI {

class TrackHeader;
class TrackView;
class Ruler;
class TimeBar;
class Spacer;
class SceneSettings;


class Sequencer : public QWidget
{
    Q_OBJECT
public:
    explicit Sequencer(QWidget *parent = 0);

    void setSceneSettings(SceneSettings * s);
    SceneSettings * sceneSettings() const;

signals:

    /** Emitted when a sequence was double-clicked */
    void sequenceSelected(MO::Sequence *);

    /** User dragged the time bar */
    void sceneTimeChanged(Double);

public slots:

    /** Tells me that the scene time has changed */
    void setSceneTime(Double);

    /** Remove everything from this view. */
    void clearTracks();

    /** Tell the sequencer which object is currently selected.
        This might change the track list according to filter settings. */
    void setCurrentObject(Object *);

    void updateLocatorBars();

protected:

    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *);

protected slots:

    void updateVScroll_();
    void updatePlaybar_();
    void updateBars_() { updatePlaybar_(); updateLocatorBars(); }
    void onSequenceSelected_(Sequence *);

    /** creates or returns current instance */
    TimeBar* getLocatorBar_(const QString& name);
private:

    void createWidgets_();

    TrackHeader * trackHeader_;
    TrackView * trackView_;
    Ruler * rulerSec_, * rulerFps_;
    QScrollBar * vScroll_;
    Spacer * spacer_;
    TimeBar * playBar_;
    QMap<QString,TimeBar*> locatorBars_;

    QGridLayout * gridLayout_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_SEQUENCER_H
