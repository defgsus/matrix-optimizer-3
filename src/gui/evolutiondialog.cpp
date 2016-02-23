/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#include <QLayout>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QMessageBox>

#include "evolutiondialog.h"
#include "widget/evolutionarea.h"
#include "gui/propertiesscrollview.h"
#include "tool/evolutionbase.h"
#include "tool/evolutionpool.h"
#include "types/properties.h"
#include "object/interface/evolutioneditinterface.h"
#include "object/object.h"
#include "object/util/objecteditor.h"
#include "object/scenelock_p.h"
#include "io/settings.h"

namespace MO {
namespace GUI {

struct EvolutionDialog::Private
{
    Private(EvolutionDialog* win)
        : win       (win)
        , propView  (0)
        , isEdit    (false)
    { }

    void updateProps();
    void updateButtons();
    void createWidgets();
    void applyProps();
    void onSelected();

    EvolutionDialog * win;

    Properties props;

    EvolutionArea * area;
    PropertiesScrollView *propView;
    QPlainTextEdit * textView;
    QPushButton *butCross, *butLeft, *butRight, *butOk, *butApply;
    bool isEdit;
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
        connect(area, &EvolutionArea::selected, [=]() { onSelected(); });
        connect(area, &EvolutionArea::propertiesChanged, [=]() { updateProps(); });
        connect(area, &EvolutionArea::historyChanged, [=]()
        {
            butLeft->setEnabled(area->hasHistory());
            butRight->setEnabled(area->hasFuture());
        });
        area->pool().setRandomSeed();

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

            but = butCross = new QPushButton(tr("cross-mate"), win);
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

            // text view
            textView = new QPlainTextEdit(win);
            lv->addWidget(textView);
            textView->setReadOnly(true);

            // dialog buttons
            lh1 = new QHBoxLayout;
            lv->addLayout(lh1);

                butApply = new QPushButton("Apply", win);
                lh1->addWidget(butApply);
                connect(butApply, &QPushButton::clicked, [=]()
                    { if (area->selectedSpecimen()) emit win->apply(); });

                butOk = new QPushButton("Ok && Close", win);
                butOk->setDefault(true);
                lh1->addWidget(butOk);
                connect(butOk, &QPushButton::clicked, [=]()
                    { if (area->selectedSpecimen()) emit win->apply(); win->accept(); });

                updateButtons();
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

void EvolutionDialog::Private::updateButtons()
{
    butOk->setVisible(isEdit);
    butApply->setVisible(isEdit);
    bool isSel = area->selectedSpecimen() != nullptr;
    butOk->setEnabled(isSel);
    butApply->setEnabled(isSel);
    butCross->setEnabled(area->numLockedTiles() >= 2);
}

void EvolutionDialog::Private::onSelected()
{
    updateButtons();
    auto evo = area->selectedSpecimen();
    if (!evo)
    {
        textView->clear();
    }
    else
    {
        textView->setPlainText(evo->toString());
    }
}
EvolutionBase* EvolutionDialog::selectedSpecimen() const
{
    return p_->area->selectedSpecimen();
}

void EvolutionDialog::setEditSpecimen(const EvolutionBase* evo)
{
    p_->isEdit = evo != nullptr;
    if (evo)
    {
        auto s = evo->createClone();
        p_->area->pool().setSpecimen(0, s);
        s->releaseRef();
        p_->area->pool().repopulateFrom(0);
        p_->updateProps();
    }
    p_->area->update();
    p_->updateButtons();
}

EvolutionDialog* EvolutionDialog::openForInterface(
        EvolutionEditInterface* iface, const QString& key)
{
    // put existing to front
    if (auto diag = iface->getAttachedEvolutionDialog(key))
    {
        diag->show();
        diag->raise();
        return diag;
    }

    // get specimen
    auto evo = iface->getEvolution(key);
    if (!evo)
    {
        QMessageBox::critical(0, tr("evolution"),
                    tr("The '%1' set can not be retrieved for evolution").arg(key));
        return nullptr;
    }

    // create new
    auto diag = new EvolutionDialog();
    diag->setAttribute(Qt::WA_DeleteOnClose);

    auto obj = dynamic_cast<Object*>(iface);
        diag->setWindowTitle(obj->namePath() + " " + tr("evolution"));

    iface->setAttachedEvolutionDialog(key, diag);
    diag->setEditSpecimen(evo);

    connect(diag, &EvolutionDialog::destroyed, [=]()
    {
        iface->setAttachedEvolutionDialog(key, 0);
    });
    connect(diag, &EvolutionDialog::apply, [=]()
    {
        auto root = obj? obj->sceneObject() : 0;
        auto editor = obj? obj->editor() : 0;
        if (root)
        {
            ScopedObjectChange lock(root, obj);
            iface->setEvolution(key, diag->selectedSpecimen());
        }
        else
            iface->setEvolution(key, diag->selectedSpecimen());
        if (editor)
            editor->emitObjectChanged(obj);
    });

    diag->show();

    return diag;

}


} // namespace GUI
} // namespace MO
