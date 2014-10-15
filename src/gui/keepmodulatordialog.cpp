/** @file keepmodulatordialog.cpp

    @brief Dialog for selecting modulators to be reused after pasting

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.10.2014</p>
*/
#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

#include "keepmodulatordialog.h"
#include "object/scene.h"
#include "object/param/parameter.h"
#include "object/param/modulator.h"
#include "io/log.h"
#include "io/error.h"
#include "io/settings.h"

namespace MO {

KeepModulators::KeepModulators(Scene *scene)
    : scene_    (scene)
{
    MO_ASSERT(scene_, "No scene given to KeepModulators");

    // list of all objects
    QList<Object*> list = scene->findChildObjects(Object::TG_ALL, true);
    list.prepend(scene);

    // get all currently wired modulation pairs
    for (auto o : list)
    {
        QList<QPair<Parameter*, Object*>> pairs = o->getModulationPairs();
        for (auto & p : pairs)
            modPairs_.insertMulti(p.second->idName(), p.first);
    }
}

void KeepModulators::addOriginalObject(Object * o)
{
    // get list of all modulator objects in o (including o)
    QList<Object*> list = o->findChildObjects(Object::TG_MODULATOR, true);
    if (o->type() & Object::TG_MODULATOR_OBJECT)
        list.prepend(o);

    // for each object that is an actually wired modulator
    for (auto obj : list)
    {
        // the modulated parameter
        auto i = modPairs_.find(obj->idName());

        // add all parameters that are modulated
        while (i != modPairs_.end() && i.key() == obj->idName())
        {
            // keep an entry with the original modulator id
            Private_ p;
            p.oldId = obj->idName();
            p.param = i.value();

            p_.insertMulti(obj, p);

            // next modulated parameter
            ++i;
        }
    }
}

void KeepModulators::addNewObject(Object * o)
{
    // get list of all modulator objects in o (including o)
    QList<Object*> list = o->findChildObjects(Object::TG_MODULATOR, true);
    if (o->type() & Object::TG_MODULATOR_OBJECT)
        list.prepend(o);

    for (auto obj : list)
    {
        auto i = p_.find(obj);

        while (i != p_.end() && i.key() == obj)
        {
            // remember new id
            i.value().newId = obj->idName();
            // and initialize to rewire
            i.value().reuse = true;

            ++i;
        }
    }
}


bool KeepModulators::modulatorsPresent() const
{
    return !p_.isEmpty();
}

void KeepModulators::createNewModulators()
{
    for (const Private_ & m : p_)
    if (m.reuse && m.newId != m.oldId)
    {
        scene_->addModulator(m.param, m.newId);

        // copy modulator settings
        Modulator
                * oldm = m.param->getModulator(m.oldId),
                * newm = m.param->getModulator(m.newId);
        if (oldm && newm)
            newm->copySettingsFrom(oldm);
        else
            MO_WARNING("Something went wrong on creating modulation path '"
                       << m.newId << " -> " << m.param->infoName() << "'");
    }
}


namespace GUI {





KeepModulatorDialog::KeepModulatorDialog(KeepModulators& mods, QWidget * parent)
    : QDialog   (parent),
      mods_     (mods)
{
    setObjectName("_KeepModulatorDialog");
    setWindowTitle(tr("Modulators"));

    setMinimumSize(320, 200);

    settings->restoreGeometry(this);

    createWidgets_();
    createList_();
}

KeepModulatorDialog::~KeepModulatorDialog()
{
    settings->saveGeometry(this);
}


void KeepModulatorDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        auto label = new QLabel(this);
        label->setText(tr("Some objects you have pasted are modulator sources. "
                          "You can choose to assign the pasted objects to the same modulator goals. "
                          "Select or deselect the modulators in the list and then click Ok "
                          "to create the additional modulation paths."));

        label->setWordWrap(true);
        lv->addWidget(label);

        list_ = new QListWidget(this);
        lv->addWidget(list_);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QPushButton(tr("Ok"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SLOT(doit_()));

            but = new QPushButton(tr("Cancel"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SLOT(reject()));
}

void KeepModulatorDialog::createList_()
{
    list_->clear();
    for (auto & m : mods_.p_)
    {
        // prepare an entry
        auto item = new QListWidgetItem(list_);
        item->setText(QString("create %1 <- %2 (copy of %3)")
                      .arg(m.param->infoName())
                      .arg(m.newId)
                      .arg(m.oldId)
                      );
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item->setCheckState(m.reuse ? Qt::Checked : Qt::Unchecked);
        list_->addItem(item);
    }
}


void KeepModulatorDialog::doit_()
{
    // copy checked-state
    int k=0;
    for (auto & m : mods_.p_)
    {
        if (k < list_->count())
            m.reuse = list_->item(k)->checkState() == Qt::Checked;

        ++k;
    }

    mods_.createNewModulators();
    accept();
}


} // namespace GUI
} // namespace MO
