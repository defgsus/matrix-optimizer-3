/** @file audiolinkviewoverpaint.cpp

    @brief Draws cables and stuff ontop of AudioLinkView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include <QPainter>

#include "audiolinkviewoverpaint.h"
#include "gui/audiolinkview.h"
#include "gui/widget/audiounitwidget.h"
#include "gui/widget/audiounitconnectorwidget.h"

namespace MO {
namespace GUI {


AudioLinkViewOverpaint::AudioLinkViewOverpaint(AudioLinkView * parent) :
    QWidget     (parent),
    view_       (parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
}


void AudioLinkViewOverpaint::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);

    //p.setBrush(QBrush(QColor(rand()%255,0,0,50)));
    //p.drawRect(rect());

    // ------ draw audio cables ------

    p.setPen(view_->penAudioCable_);

    for (const AudioUnitWidget * from : view_->unitWidgets_)
    {
        for (auto &id : from->connectedIds())
        {
            // find connected widget
            auto i = view_->unitWidgets_.find(id);
            if (i == view_->unitWidgets_.end())
                continue;

            const AudioUnitWidget * to = i.value();

            // draw audio connections
            const int num = std::min(from->audioOutWidgets().size(),
                               to->audioInWidgets().size());
            for (int i = 0; i < num; ++i)
            {
                const QPoint
                        pfrom = from->audioOutWidgets()[i]->mapTo(view_,
                                        from->audioOutWidgets()[i]->rect().center()),
                        pto = to->audioInWidgets()[i]->mapTo(view_,
                                      to->audioInWidgets()[i]->rect().center());

                QPainterPath path(pfrom);
                path.cubicTo(pfrom.x()+20, pfrom.y(), pto.x()-20, pto.y(), pto.x(), pto.y());
                p.drawPath(path);
                //p.drawLine(pfrom, pto);
            }
        }
    }


    // ----- drag widget indicator -----

    if (view_->draggedWidget_)
    {
        p.setPen(view_->penDragFrame_);
        p.setBrush(Qt::NoBrush);
        p.drawRect(view_->getWidgetRect_(view_->draggedWidget_));
    }

    if (view_->dragGoal_.unitWidget)
    {
        p.setPen(view_->penDragFrame_);
        p.setBrush(view_->brushDragTo_);
        p.drawRect(view_->dragGoal_.displayRect);

        p.drawLine(view_->getWidgetRect_(view_->draggedWidget_).center(),
                   view_->dragGoal_.displayRect.center());
    }

    p.restore();
}

} // namespace GUI
} // namespace MO
