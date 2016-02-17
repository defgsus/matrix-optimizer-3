/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#include <QLayout>
#include <QSpinBox>

#include "evolutiondialog.h"
#include "widget/evolutionarea.h"


namespace MO {
namespace GUI {

struct EvolutionDialog::Private
{
    Private(EvolutionDialog* win)
        : win   (win)
    { }

    void createWidgets();

    EvolutionDialog * win;

    EvolutionArea * area;
    QSpinBox* spinTileY;
};

EvolutionDialog::EvolutionDialog(QWidget* parent)
    : QDialog   (parent)
    , p_        (new Private(this))
{
    setObjectName("EvolutionDialog");
    setMinimumSize(320, 240);
    p_->createWidgets();
}

EvolutionDialog::~EvolutionDialog()
{
    delete p_;
}

void EvolutionDialog::Private::createWidgets()
{
    auto lv = new QVBoxLayout(win);

        area = new EvolutionArea(win);
        lv->addWidget(area);

        spinTileY = new QSpinBox(win);
        spinTileY->setRange(1, 50);
        spinTileY->setValue(area->numTilesY());
        connect(spinTileY, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=]()
        {
            area->setNumTilesY(spinTileY->value());
        });

        lv->addWidget(spinTileY);

}



} // namespace GUI
} // namespace MO
