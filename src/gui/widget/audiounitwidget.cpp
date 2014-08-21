/** @file audiounitwidget.cpp

    @brief Widget for displaying/connecting AudioUnits

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QIcon>
#include <QToolButton>
#include <QPaintEvent>

#include "audiounitwidget.h"
#include "audiounitconnectorwidget.h"
#include "object/audio/audiounit.h"
#include "object/modulatorobjectfloat.h"
#include "object/objectfactory.h"
#include "io/log.h"

namespace MO {
namespace GUI {



AudioUnitWidget::AudioUnitWidget(AudioUnit * au, QWidget *parent) :
    QWidget     (parent),
    unit_       (au),
    expanded_   (true),
    dragging_   (false)
{
    setMinimumSize(100,50);
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

    updateExpandState_();
}

QWidget * AudioUnitWidget::createHeader_()
{
    auto head = new QWidget(this);
    head->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    head->setAutoFillBackground(true);
    QPalette pal(head->palette());
    pal.setColor(QPalette::Window, pal.color(QPalette::Window).darker(120));
    head->setPalette(pal);


    auto lh = new QHBoxLayout(head);
    lh->setMargin(0);

        butExpand_ = new QToolButton(head);
        lh->addWidget(butExpand_);
        butExpand_->setFixedSize(16,16);
        connect(butExpand_, &QToolButton::clicked, [=]()
        {
            setExpanded(!isExpanded(), true);
        });

        auto label = new QLabel(unit_->name(), this);
        lh->addWidget(label);
        lh->setAlignment(label, Qt::AlignCenter | Qt::AlignHCenter);

        auto butRemove = new QToolButton(head);
        lh->addWidget(butRemove);
        butRemove->setFixedSize(16,16);
        butRemove->setIcon(QIcon(":/icon/delete.png"));

    return head;
}

void AudioUnitWidget::createWidgets_()
{
    auto lv0 = new QVBoxLayout(this);
    lv0->setMargin(1);

        // header

        auto header = createHeader_();
        lv0->addWidget(header);

        lv0->addStretch(1);

        // body

        auto lh = new QHBoxLayout();
        lh->setMargin(4);
        lv0->addLayout(lh);

            // --- inputs ---

            auto lv = new QVBoxLayout();
            lh->addLayout(lv);

                for (uint i=0; i<unit_->numChannelsIn(); ++i)
                {
                    auto c = new AudioUnitConnectorWidget(unit_, i, true, true, this);
                    lv->addWidget( c );
                    audioIns_.append( c );
                }

            // icon

            icon_ = new QLabel(this);
            icon_->setPixmap(ObjectFactory::iconForObject(unit_).pixmap(48,48));
            icon_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            lh->addWidget(icon_);
            lh->setStretchFactor(icon_, 2);
            lh->setAlignment(icon_, Qt::AlignCenter);


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

                    if (auto modf = dynamic_cast<ModulatorObjectFloat*>(mods[i]))
                        c->setModulatorObjectFloat(modf);
                }

}

void AudioUnitWidget::updateValueOutputs()
{
    for (auto m : modulatorOuts_)
        m->update();
}

void AudioUnitWidget::mousePressEvent(QMouseEvent * e)
{
    if (!dragging_)
    {
        dragStart_ = e->pos();
    }
}

void AudioUnitWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        if (!dragging_ &&
                (e->pos() - dragStart_).manhattanLength() >= 4)
        {
            dragging_ = true;
            emit dragStart(this);

            e->accept();
            return;
        }

        if (dragging_)
        {
            emit dragMove(this, e->pos());
            e->accept();
            return;
        }
    }
    e->ignore();
}

void AudioUnitWidget::mouseReleaseEvent(QMouseEvent * e)
{
    if (dragging_)
    {
        dragging_ = false;
        emit dragEnd(this, e->pos());

        e->accept();
        return;
    }
}

void AudioUnitWidget::setExpanded(bool enable, bool send_signal)
{
    if (expanded_ == enable)
        return;

    expanded_ = enable;

    updateExpandState_();

    if (send_signal)
        emit expansionChanged(expanded_);
}

void AudioUnitWidget::updateExpandState_()
{
    butExpand_->setArrowType(expanded_? Qt::RightArrow : Qt::DownArrow);

    icon_->setVisible(expanded_);
    for (auto c : audioIns_)
        c->setVisible(expanded_);
    for (auto c : audioOuts_)
        c->setVisible(expanded_);
    for (auto c : modulatorOuts_)
        c->setVisible(expanded_);
}

} // namespace GUI
} // namespace MO
