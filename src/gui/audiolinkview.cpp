/** @file audiolinkview.cpp

    @brief Editor for AudioUnits and Modulators

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include <QLayout>

#include "audiolinkview.h"
#include "widget/audiounitwidget.h"
#include "object/scene.h"
#include "object/audio/audiounit.h"

namespace MO {
namespace GUI {


AudioLinkView::AudioLinkView(QWidget *parent) :
    QWidget     (parent),
    scene_      (0)
{
    setObjectName("_AudioLinkView");

    createMainWidgets_();
}


void AudioLinkView::createMainWidgets_()
{
    grid_ = new QGridLayout(this);
    grid_->setSpacing(10);
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

            createAudioUnitWidgetsRec_(au->childObjects(), row, col+1);

            ++row;
        }
    }
}




} // namespace GUI
} // namespace MO
