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

    bool isSelected(ClipWidget * c) const { return selection_.isSelected(c); }

signals:

public slots:

    /** Completely resets or updates the view to the new data.
        If the new container is the same as the previous one,
        it will be updated for rows/columns.
        Set to NULL to clear this view. */
    void setClipContainer(ClipContainer *);

    /** Update the widget for this clip */
    void updateClip(Clip *);
    /** Update the widget for this ClipContainer position */
    void updateClip(uint x, uint y);

    /** For a destroyed object @p o, remove any widgets */
    void removeObject(const Object *o);

private slots:

    void onClicked_(ClipWidget*, Qt::MouseButtons, Qt::KeyboardModifiers);

private:

    void createWidgets_();
    void createClipWidgets_();

    void updateClipWidget_(uint x, uint y);

    void openPopup_();

    void clearSelection_();
    void select_(ClipWidget * w);

    ClipWidget * widgetForClip_(const Clip *);
    ClipWidget * clipWidget_(uint x, uint y);

    ClipContainer * clipCon_;

    QList<ClipWidget*> clipWidgets_;
    QMap<const Clip*, ClipWidget*> widgetMap_;

    Selection<ClipWidget*> selection_;

    uint
        curNumX_, curNumY_,
        curX_, curY_,
        selStartX_, selStartY_;
    Clip * curClip_;

    QScrollArea * scrollArea_, * scrollAreaH_, *scrollAreaV_;
    QWidget * container_, *containerH_, *containerV_;
    QGridLayout * layout_, *layoutH_, *layoutV_;

};

} // namespace GUI
} // namespace MO



#endif // MOSRC_GUI_CLIPVIEW_H
