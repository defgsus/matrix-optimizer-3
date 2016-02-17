/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#include <QLayout>
#include <QPushButton>

#include "evolutiondialog.h"
#include "widget/evolutionarea.h"
#include "tool/evolutionbase.h"
#include "tool/evolutionpool.h"
#include "io/settings.h"
#include "types/properties.h"
#include "gui/propertiesscrollview.h"

namespace MO {
namespace GUI {

struct EvolutionDialog::Private
{
    Private(EvolutionDialog* win)
        : win   (win)
    { }

    void createProps();
    void createWidgets();
    void applyProps();

    EvolutionDialog * win;

    Properties props;

    EvolutionArea * area;
    PropertiesScrollView *propView;//, *prop2View;
};

EvolutionDialog::EvolutionDialog(QWidget* parent)
    : QDialog   (parent)
    , p_        (new Private(this))
{
    setObjectName("EvolutionDialog");
    setMinimumSize(320, 240);

    settings()->restoreGeometry(this);

    p_->createWidgets();
    p_->createProps();
    p_->propView->setProperties(p_->props);
    //p_->prop2View->setProperties(p_->props2);

    /*auto t = new EvolutionVectorBase(20);
    p_->area->setTile(0, t);
    t->releaseRef();
    */
}

EvolutionDialog::~EvolutionDialog()
{
    settings()->storeGeometry(this);

    delete p_;
}

void EvolutionDialog::Private::createWidgets()
{
    auto lh = new QHBoxLayout(win);

        area = new EvolutionArea(win);
        lh->addWidget(area, 3);

        auto lv = new QVBoxLayout();
        lh->addLayout(lv, 1);

            propView = new PropertiesScrollView(win);
            lv->addWidget(propView, 3);
            connect(propView, &PropertiesScrollView::propertyChanged, [=]()
            {
                props.unify(propView->properties());
                applyProps();
            });

            auto but = new QPushButton(tr("reset"), win);
            lv->addWidget(but);
            connect(but, &QPushButton::pressed, [=]()
            {
                //props2.unify(prop2View->properties());
                //applyProps();
                area->pool().randomize();
                area->update();
            });
            /*
            prop2View = new PropertiesScrollView(win);
            lv->addWidget(prop2View, 1);
            connect(propView, &PropertiesScrollView::propertyChanged, [=]()
            {

            });
            */
}

void EvolutionDialog::Private::createProps()
{
    props = area->pool().properties();
    props.clear("seed");

    props.set("num_y", tr("num tiles"), tr("Number of tiles per screen height"),
              area->numTilesY(), 1u, 50u);
}

void EvolutionDialog::Private::applyProps()
{
    area->setNumTilesY(props.get("num_y").toUInt());

    area->pool().setProperties(props);
}

} // namespace GUI
} // namespace MO
