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
    colorAudioUnit_ = QColor(120,40,80).lighter(120);
    colorModulatorObject_ = QColor(0, 90, 90).lighter(120);
}


void AudioLinkViewOverpaint::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QPen pen(colorAudioUnit_);
    pen.setWidth(2);

    p.setPen(pen);

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

                p.drawLine(pfrom, pto);
            }
        }
    }
}

} // namespace GUI
} // namespace MO
