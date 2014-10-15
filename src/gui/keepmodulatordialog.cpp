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
#include "object/objectfactory.h"
#include "io/log.h"
#include "io/error.h"
#include "io/settings.h"

namespace MO {

KeepModulators::KeepModulators(Scene *scene)
    : scene_    (scene)
{
    MO_ASSERT(scene_, "Null scene given to KeepModulators");

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

    MO_DEBUG_MOD("KeepModulators::KeepModulators(" << scene << ") "
                 "found " << modPairs_.size() << " modulation pairs");
}

void KeepModulators::addOriginalObject(Object * o)
{
    MO_DEBUG_MOD("KeepModulators::addOriginalObject(" << o << ")");

    // get list of all modulator objects in o (including o)
    QList<Object*> list = o->findChildObjects(Object::TG_MODULATOR, true);
    if (o->type() & Object::TG_MODULATOR)
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
            p.object = obj;

            p_.insertMulti(obj, p);

            // next modulated parameter
            ++i;
        }
    }
}

void KeepModulators::addNewObject(Object * o)
{
    MO_DEBUG_MOD("KeepModulators::addNewObject(" << o << ")");

    // get list of all modulator objects in o (including o)
    QList<Object*> list = o->findChildObjects(Object::TG_MODULATOR, true);
    if (o->type() & Object::TG_MODULATOR)
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

            // switch default off for sequences on tracks
            // (since normally the tracks are the modulators)
            if (obj->isSequence() && obj->parentObject() && obj->parentObject()->isTrack())
                i.value().reuse = false;

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
    MO_DEBUG_MOD("KeepModulators::createNewModulators()");

    for (const Private_ & m : p_)
    if (m.reuse && m.newId != m.oldId)
    {
        MO_DEBUG_MOD("KeepModulators: creating path " << m.newId
                 << " -> " << m.param->infoName());

        scene_->addModulator(m.param, m.newId);

        // copy modulator settings
        Modulator
                * oldm = m.param->findModulator(m.oldId),
                * newm = m.param->findModulator(m.newId);
        if (oldm && newm)
            newm->copySettingsFrom(oldm);
        // XXX oldm is NULL for Sequences in Tracks
        // because the actual modulator is the track, not the sequence
        // TODO: go up the parent branch of oldId until the modulator is found!
        /*else
            MO_WARNING("Something went wrong on creating modulation path '"
                       << m.newId << " -> " << m.param->infoName() << "'");
        */

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
    if (!m.newId.isEmpty())
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
        item->setIcon(ObjectFactory::iconForObject(m.object));
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
