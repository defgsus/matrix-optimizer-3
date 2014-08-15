/** @file audiounitwidget.cpp

    @brief Widget for displaying/connecting AudioUnits

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include <QLayout>

#include "audiounitwidget.h"
#include "audiounitconnectorwidget.h"
#include "object/audio/audiounit.h"
#include "object/modulatorobject.h"


namespace MO {
namespace GUI {



AudioUnitWidget::AudioUnitWidget(AudioUnit * au, QWidget *parent) :
    QWidget     (parent),
    unit_       (au)
{
    setMinimumSize(100,100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAutoFillBackground(true);
    QPalette pal(palette());
    pal.setColor(QPalette::Window, pal.color(QPalette::Window).darker(130));
    setPalette(pal);

    createWidgets_();

    // find connected IDs
    if (unit_->numChannelsOut())
        for (Object * o : unit_->childObjects())
            if (AudioUnit * au = qobject_cast<AudioUnit*>(o))
                if (au->numChannelsIn())
                    connectedIds_.append(au->idName());
}


void AudioUnitWidget::createWidgets_()
{
    auto lh = new QHBoxLayout(this);

        // --- inputs ---

        auto lv = new QVBoxLayout();
        lh->addLayout(lv);

            for (uint i=0; i<unit_->numChannelsIn(); ++i)
            {
                auto c = new AudioUnitConnectorWidget(unit_, i, true, true, this);
                lv->addWidget( c );
                audioIns_.append( c );
            }

        lh->addStretch(2);


        // --- outputs ---

        lv = new QVBoxLayout();
        lh->addLayout(lv);

            // audio
            for (uint i=0; i<unit_->numChannelsOut(); ++i)
            {
                auto c = new AudioUnitConnectorWidget(unit_, i, false, true, this);
                lv->addWidget( c );
                audioOuts_.append( c );
            }

            // modulator
            QList<ModulatorObject*> mods = unit_->findChildObjects<ModulatorObject>();
            for (int i=0; i<mods.size(); ++i)
            {
                auto c = new AudioUnitConnectorWidget(unit_, i, false, false, this);
                lv->addWidget( c );
                modulatorOuts_.append( c );
            }

}

} // namespace GUI
} // namespace MO
