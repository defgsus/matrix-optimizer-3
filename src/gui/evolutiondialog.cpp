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
        : win       (win)
        , propView  (0)
    { }

    void updateProps();
    void createWidgets();
    void applyProps();

    EvolutionDialog * win;

    Properties props;

    EvolutionArea * area;
    PropertiesScrollView *propView;
    QPushButton * butLeft, *butRight;
};

EvolutionDialog::EvolutionDialog(QWidget* parent)
    : QDialog   (parent)
    , p_        (new Private(this))
{
    setObjectName("EvolutionDialog");
    setWindowTitle(tr("Evolution"));
    setMinimumSize(320, 240);

    settings()->restoreGeometry(this);

    p_->createWidgets();
    p_->updateProps();
    p_->propView->setProperties(p_->props);

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
        connect(area, &EvolutionArea::propertiesChanged, [=]() { updateProps(); });
        connect(area, &EvolutionArea::historyChanged, [=]()
        {
            butLeft->setEnabled(area->hasHistory());
            butRight->setEnabled(area->hasFuture());
        });

        auto lv = new QVBoxLayout();
        lh->addLayout(lv, 1);

            // history buttons
            auto lh1 = new QHBoxLayout;
            lv->addLayout(lh1);

                butLeft = new QPushButton("<", win);
                lh1->addWidget(butLeft);
                connect(butLeft, &QPushButton::clicked, [=]() { area->setHistory(-1); });

                butRight = new QPushButton(">", win);
                lh1->addWidget(butRight);
                connect(butRight, &QPushButton::clicked, [=]() { area->setHistory(1); });

            auto but = new QPushButton(tr("repopulate"), win);
            lv->addWidget(but);
            connect(but, &QPushButton::pressed, [=]()
            {
                area->pool().repopulate();
                area->update();
            });

            but = new QPushButton(tr("cross-breed"), win);
            lv->addWidget(but);
            connect(but, &QPushButton::pressed, [=]()
            {
                area->pool().crossBreed();
                area->update();
            });

            propView = new PropertiesScrollView(win);
            lv->addWidget(propView, 3);
            connect(propView, &PropertiesScrollView::propertyChanged, [=]()
            {
                props.unify(propView->properties());
                applyProps();
            });

}

void EvolutionDialog::Private::updateProps()
{
    props = area->pool().properties();
    props.clear("seed");

    props.set("num_y", tr("num tiles"), tr("Number of tiles per screen height"),
              area->numTilesY(), 1u, 50u);

    if (propView)
    {
        props.updateFrom(propView->properties());
        propView->setProperties(props);
    }
}

void EvolutionDialog::Private::applyProps()
{
    area->setNumTilesY(props.get("num_y").toUInt());

    area->pool().setProperties(props);
}

} // namespace GUI
} // namespace MO
