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

class QGridLayout;
class QScrollArea;

namespace MO {
class Clip;
class ClipContainer;
namespace GUI {

class ClipWidget;

class ClipView : public QWidget
{
    Q_OBJECT
public:
    explicit ClipView(QWidget *parent = 0);

signals:

public slots:

    /** Completely resets the view to the new data */
    void setClipContainer(ClipContainer *);

private slots:

    void onClicked_(ClipWidget*, Qt::MouseButtons, Qt::KeyboardModifiers);

private:

    void createWidgets_();
    void createClipWidgets_();

    void updateClipWidget_(uint x, uint y);

    void openPopup_();

    ClipWidget * widgetForClip_(Clip *);
    ClipWidget * clipWidget_(uint x, uint y);

    ClipContainer * clipCon_;

    QList<ClipWidget*> clipWidgets_;
    QMap<Clip*, ClipWidget*> widgetMap_;

    uint curX_, curY_;
    Clip * curClip_;

    QScrollArea * scrollArea_, * scrollAreaH_, *scrollAreaV_;
    QWidget * container_, *containerH_, *containerV_;
    QGridLayout * layout_, *layoutH_, *layoutV_;

};

} // namespace GUI
} // namespace MO



#endif // MOSRC_GUI_CLIPVIEW_H
