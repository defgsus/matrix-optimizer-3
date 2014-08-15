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
}


void AudioUnitWidget::createWidgets_()
{
    auto lh = new QHBoxLayout(this);

        // --- inputs ---

        auto lv = new QVBoxLayout();
        lh->addLayout(lv);

            for (uint i=0; i<unit_->numChannelsIn(); ++i)
            {
                lv->addWidget( new AudioUnitConnectorWidget(unit_, i, true, true, this) );
            }

        lh->addStretch(2);


        // --- outputs ---

        lv = new QVBoxLayout();
        lh->addLayout(lv);

            // audio
            for (uint i=0; i<unit_->numChannelsOut(); ++i)
            {
                lv->addWidget( new AudioUnitConnectorWidget(unit_, i, false, true, this) );
            }

            // modulator
            QList<ModulatorObject*> mods = unit_->findChildObjects<ModulatorObject>();
            for (int i=0; i<mods.size(); ++i)
            {
                lv->addWidget( new AudioUnitConnectorWidget(unit_, i, false, false, this) );
            }

}

} // namespace GUI
} // namespace MO
