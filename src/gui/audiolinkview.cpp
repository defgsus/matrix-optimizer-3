/** @file audiolinkview.cpp

    @brief Editor for AudioUnits and Modulators

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include <QLayout>
#include <QTimer>

#include "audiolinkview.h"
#include "widget/audiounitwidget.h"
#include "painter/audiolinkviewoverpaint.h"
#include "object/scene.h"
#include "object/audio/audiounit.h"
#include "io/log.h"

namespace MO {
namespace GUI {


AudioLinkView::AudioLinkView(QWidget *parent) :
    QWidget     (parent),
    scene_      (0),
    timer_      (new QTimer(this)),
    draggedWidget_  (0)
{
    setObjectName("_AudioLinkView");

    colorAudioUnit_ = QColor(120,40,80).lighter(120);
    colorModulatorObject_ = QColor(0, 90, 90).lighter(120);

    penAudioCable_ = QPen(colorAudioUnit_);
    penAudioCable_.setWidth(2);

    penDragFrame_ = QPen(QColor(255,255,255));
    penDragFrame_.setStyle(Qt::DashLine);
    penDragFrame_.setWidth(2);

    brushDragTo_ = QBrush(QColor(255,255,255,100));

    createMainWidgets_();

    timer_->setSingleShot(false);
    timer_->setInterval(1000 / 25);
    connect(timer_, SIGNAL(timeout()), this, SLOT(updateValueOutputs_()));

    setAnimating(true);
}


void AudioLinkView::createMainWidgets_()
{
    grid_ = new QGridLayout(this);
    grid_->setHorizontalSpacing(20);
    grid_->setVerticalSpacing(10);

    overpainter_ = new AudioLinkViewOverpaint(this);
}

void AudioLinkView::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);
    overpainter_->resize(size());
    overpainter_->raise();
}

void AudioLinkView::setScene(Scene * s)
{
    scene_ = s;

    //topLevelUnits_ = scene_->findChildObjects<AudioUnit*>();

    createAudioUnitWidgets_();
}


void AudioLinkView::clearAudioUnitWidgets_()
{
    for (auto w : unitWidgets_)
        w->deleteLater();
    unitWidgets_.clear();
}

void AudioLinkView::createAudioUnitWidgets_()
{
    clearAudioUnitWidgets_();

    int row = 0, col = 0;

    createAudioUnitWidgetsRec_(scene_->childObjects(), row, col);

    grid_->setRowStretch(grid_->rowCount(), 2);
    grid_->setColumnStretch(grid_->columnCount(), 2);

    overpainter_->raise();
}

void AudioLinkView::createAudioUnitWidgetsRec_(
        const QList<Object *> &objects, int &row, int col)
{
    for (Object * obj : objects)
    {
        if (AudioUnit * au = qobject_cast<AudioUnit*>(obj))
        {
            AudioUnitWidget * w = new AudioUnitWidget(au, this);
            // add to layout
            grid_->addWidget(w, row, col);
            // add to id->widget mao
            unitWidgets_.insert(au->idName(), w);

            // connect
            connect(w, SIGNAL(dragStart(AudioUnitWidget*)),
                    this, SLOT(onUnitDragStart_(AudioUnitWidget*)));
            connect(w, SIGNAL(dragMove(AudioUnitWidget*, QPoint)),
                    this, SLOT(onUnitDragMove_(AudioUnitWidget*,QPoint)));
            connect(w, SIGNAL(dragEnd(AudioUnitWidget*, QPoint)),
                    this, SLOT(onUnitDragEnd_(AudioUnitWidget*,QPoint)));

            // process childs
            createAudioUnitWidgetsRec_(au->childObjects(), row, col+1);

            ++row;
        }
    }
}

void AudioLinkView::setAnimating(bool enable)
{
    if (enable && !timer_->isActive())
        timer_->start();
    if (!enable && timer_->isActive())
        timer_->stop();
}

void AudioLinkView::updateValueOutputs_()
{
    for (auto w : unitWidgets_)
        w->updateValueOutputs();
}

QRect AudioLinkView::getWidgetRect_(AudioUnitWidget * w) const
{
    // get position of widget in gridlayout
    const int idx = grid_->indexOf(w);
    int row, col, rows, cols;
    grid_->getItemPosition(idx, &row, &col, &rows, &cols);

    return grid_->cellRect(row, col);
}

QRect AudioLinkView::getWidgetUpdateRect_(AudioUnitWidget * auw) const
{
    const int w = penDragFrame_.width();
    return getWidgetRect_(auw).adjusted(-w,-w,w+1,w+1);
}

void AudioLinkView::getDragGoal(const QPoint &pos, DragGoal_ &goal) const
{
    // NOTE:
    // We don't use childAt() because that returns not the AudioUnitWidget
    // when pos is over a child of the AudioUnitWidget
    // Also we need to know if we are left/right/above/below the widget as well

    goal.unitWidget = 0;

    for (AudioUnitWidget * aw : unitWidgets_)
    {
        // get each widget's rect
        QRect rect;
        rect.moveTo( aw->mapTo(this, QPoint(0,0)) );
        rect.setSize( aw->size() );

        if (rect.contains(pos))
        {
            goal.pos = DragGoal_::ON;
            goal.unitWidget = aw;
            goal.displayRect = getWidgetUpdateRect_(aw);
            goal.updateRect = getWidgetUpdateRect_(aw);
        }
    }
}

void AudioLinkView::onUnitDragStart_(AudioUnitWidget * w)
{
    draggedWidget_ = w;
    update(getWidgetUpdateRect_(w));
}

void AudioLinkView::onUnitDragMove_(AudioUnitWidget * w, const QPoint &p)
{
    if (draggedWidget_ != w)
        return;

    QPoint pos = w->mapTo(this, p);
    DragGoal_ goal;
    getDragGoal(pos, goal);
    // update new rect
    if (goal.unitWidget)
        update(goal.updateRect);
    // update previous rect
    if (dragGoal_.unitWidget)
        update(dragGoal_.updateRect);
    dragGoal_ = goal;
}

void AudioLinkView::onUnitDragEnd_(AudioUnitWidget * w, const QPoint &)
{
    // update view
    if (dragGoal_.unitWidget)
        update(dragGoal_.updateRect);
    update(getWidgetUpdateRect_(w));
    // switch off dragging
    draggedWidget_ = 0;
    dragGoal_.unitWidget = 0;
}

} // namespace GUI
} // namespace MO
