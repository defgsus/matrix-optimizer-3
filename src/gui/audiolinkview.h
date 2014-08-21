/** @file audiolinkview.h

    @brief Editor for AudioUnits and Modulators

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef MOSRC_GUI_AUDIOLINKVIEW_H
#define MOSRC_GUI_AUDIOLINKVIEW_H

#include <QWidget>
#include <QMap>
#include <QPen>
#include <QBrush>

#include "object/object_fwd.h"

class QGridLayout;
class QTimer;

namespace MO {
namespace GUI {

class AudioUnitWidget;
class AudioLinkViewOverpaint;

class AudioLinkView : public QWidget
{
    Q_OBJECT

    friend class AudioLinkViewOverpaint;
    struct DragGoal_;

public:
    explicit AudioLinkView(QWidget *parent = 0);

    void setScene(Scene * );

signals:

public slots:

    void setAnimating(bool enable);
    void updateAll();

protected:

    void resizeEvent(QResizeEvent *);

private slots:

    void updateValueOutputs_();

    void onUnitDragStart_(AudioUnitWidget*);
    void onUnitDragMove_(AudioUnitWidget*, const QPoint&);
    void onUnitDragEnd_(AudioUnitWidget*, const QPoint&);

private:

    void createMainWidgets_();
    void clearAudioUnitWidgets_();

    void createAudioUnitWidgets_();
    void createAudioUnitWidgetsRec_(const QList<Object*>& objects, int& row, int col);

    /** Returns the rect for the widget in the gridlayout */
    QRect getWidgetRect_(AudioUnitWidget * ) const;
    QRect getWidgetUpdateRect_(AudioUnitWidget * ) const;
    void getDragGoal(const QPoint& pos, DragGoal_& goal) const;

    struct DragGoal_
    {
        enum Pos
        {
            ON,
            LEFT,
            RIGHT,
            ABOVE,
            BELOW
        };

        AudioUnitWidget * unitWidget;
        Pos pos;
        QRect displayRect, updateRect;
        DragGoal_() : unitWidget(0), pos(ON) { }
    };

    QGridLayout * grid_;

    Scene * scene_;
    //QList<AudioUnit*> topLevelUnits_;

    QMap<QString, AudioUnitWidget*> unitWidgets_;

    AudioLinkViewOverpaint * overpainter_;

    QTimer * timer_;

    AudioUnitWidget * draggedWidget_;
    DragGoal_ dragGoal_;

    // -- config --

    QColor colorAudioUnit_, colorModulatorObject_;

    QPen penAudioCable_, penDragFrame_;
    QBrush brushDragTo_;

};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_AUDIOLINKVIEW_H
