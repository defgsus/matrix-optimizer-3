/** @file clipview.h

    @brief Clip object view

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_GUI_CLIPVIEW_H
#define MOSRC_GUI_CLIPVIEW_H

#include <QWidget>
#include <QMap>

#include "tool/selection.h"

class QGridLayout;
class QScrollArea;

namespace MO {
class Object;
class Clip;
class ClipContainer;
namespace GUI {

class ClipWidget;

class ClipView : public QWidget
{
    Q_OBJECT
public:
    explicit ClipView(QWidget *parent = 0);

    ClipContainer * clipContainer() const { return clipCon_; }

    /** Used by ClipWidget to show selection highlight */
    bool isSelected(ClipWidget * c) const;

signals:

    /** When a clip was selected */
    void objectSelected(MO::Object*);

    /** Emitted after clips have been moved */
    void clipsMoved();

public slots:

    /** Completely resets or updates the view to the new data.
        If the new container is the same as the previous one,
        it will be updated for rows/columns.
        Set to NULL to clear this view. */
    void setClipContainer(ClipContainer *);

    /** Selects the clip belonging to @p o.
        If @p o is NULL the selection is cleared. */
    void selectObject(Object * o);

    /** Update the widget for this clip */
    void updateClip(Clip *);
    /** Update the widget for this ClipContainer position */
    void updateClip(uint x, uint y);

    /** For a destroyed object @p o, remove any widgets */
    void removeObject(const Object *o);

private slots:

    void onClicked_(ClipWidget*, Qt::MouseButtons, Qt::KeyboardModifiers);
    void onMoved_(ClipWidget*, const QPoint&, Qt::MouseButtons, Qt::KeyboardModifiers);
    void onReleased_(ClipWidget*, Qt::MouseButtons, Qt::KeyboardModifiers);
    void onButtonClicked_(ClipWidget*);

    void onClipTriggered_(Clip*);
    void onClipStopTriggered_(Clip*);
    void onClipStarted_(Clip*);
    void onClipStopped_(Clip*);

private:

    void createWidgets_();
    void createClipWidgets_();

    void updateClipWidget_(uint x, uint y);

    void openPopup_();

    void clearSelection_();
    void select_(ClipWidget * w);
    void selectRange_(uint x1, uint y1, uint x2, uint y2);
    void clickSelect_(ClipWidget * w, Qt::KeyboardModifiers);

    void moveSelection_(int dx, int dy);
    void moveClip_(ClipWidget *, uint newx, uint newy);
    void pasteClips_(const QList<Object*>&, uint x, uint y);
    void pasteSubObjects_(const QList<Object*>&, Clip * parent);
    void pasteClipsInClip_(const QList<Object*>&, Clip * parent);

    ClipWidget * widgetForClip_(const Clip *);
    ClipWidget * clipWidget_(uint x, uint y);

    ClipContainer * clipCon_;

    QList<ClipWidget*> clipWidgets_;
    QMap<const Clip*, ClipWidget*> widgetMap_;

    Selection<ClipWidget*>
        selection_, goalSelection_;

    uint
        curNumX_, curNumY_,
        curX_, curY_,
        selStartX_, selStartY_;
    Clip * curClip_;
    ClipWidget * dragWidget_, *goalWidget_;

    QScrollArea * scrollArea_, * scrollAreaH_, *scrollAreaV_;
    QWidget * container_, *containerH_, *containerV_;
    QGridLayout * layout_, *layoutH_, *layoutV_;

};

} // namespace GUI
} // namespace MO



#endif // MOSRC_GUI_CLIPVIEW_H
