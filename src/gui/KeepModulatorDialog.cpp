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

#include "KeepModulatorDialog.h"
#include "util/AppIcons.h"
#include "object/Scene.h"
#include "object/param/Parameter.h"
#include "object/param/Modulator.h"
#include "object/util/ObjectFactory.h"
#include "object/util/ObjectEditor.h"
#include "io/log_mod.h"
#include "io/error.h"
#include "io/Settings.h"

namespace MO {


struct KeepModulators::Private
{
    // struct used to copy a modulation path
    struct Mod
    {
        // the old and new modulator ids
        QString oldId, newId, oldName, newName;
        // the original modulation goal
        Parameter* param,
        // the modulation goal in the copied object
        // XXX not used yet
            *newparam;
        // the original modulator
        Modulator * oldMod;
        // the object itself (the modulation source)
        Object * object;
        // flag to copy the modulation path
        bool createCopy;
        // flag to reassign the modpath
        bool reassign;
    };

    // struct used to re-assign a modulation path
    /*struct Param
    {
        Object * modulator;
        Parameter * param;
        QString oldObjId, newObjId;
        bool reassign;
    };*/

    // list of all objects that are modulators
    // (multiple entries for each modulation pair)
    QMultiMap<Object*, Mod> mods;

    // list of all parameters that are modulated
    // (multiple entries for each modulation pair)
    //QMultiMap<Parameter*, Mod> params;

    Scene * scene;

    // pairs of modulator-id and goal-parameter
    QMultiMap<QString, Parameter*> modPairs;

    // mapping from old ids to new ids
    QMap<QString, QString> ids;
};


KeepModulators::KeepModulators(Scene *scene)
    : p_    (new Private())
{
    p_->scene = scene;

    MO_ASSERT(p_->scene, "Null scene given to KeepModulators");

    // list of all objects
    QList<Object*> list = p_->scene->findChildObjects(Object::TG_ALL, true);
    list.prepend(scene);

    // get all currently wired modulation pairs
    for (auto o : list)
    {
        QList<QPair<Parameter*, Object*>> pairs = o->getModulationPairs();
        for (auto & p : pairs)
            p_->modPairs.insertMulti(p.second->idName(), p.first);
    }

    MO_DEBUG_MOD("KeepModulators::KeepModulators(" << p_->scene << ") "
                 "found " << p_->modPairs.size() << " modulation pairs");
}

KeepModulators::~KeepModulators()
{
    MO_DEBUG_MOD("KeepModulators::~KeepModulators()");
    delete p_;
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
        auto i = p_->modPairs.find(obj->idName());

        // add all parameters that are modulated
        while (i != p_->modPairs.end() && i.key() == obj->idName())
        {
            // -- for copying modulations --

            // keep an entry with the original modulator id
            Private::Mod m;
            m.oldId = obj->idName(); // modulator id
            m.oldName = obj->name();
            m.object = obj; // modulator
            m.param = i.value(); // modulation goal (always parameter)
            m.newparam = 0;

            // find Modulator class
            // (e.g for Sequences the Modulator class points to the track)
            MO_DEBUG("---check pair: " << obj->idName() << " > " << i.value()->infoName());
            m.oldMod = 0;
            Object * o = obj;
            while (!m.oldMod && o)
            {
                m.oldMod = m.param->findModulator(o->idName());
                MO_DEBUG("look: " << o->className() << ":"
                         << o->idName() << " " << m.oldMod);
                o = o->parentObject();
            }

            p_->mods.insertMulti(obj, m);

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
        auto i = p_->mods.find(obj);

        // all modulation pairs the the object is part of
        while (i != p_->mods.end() && i.key() == obj)
        {
            // remember new id
            i.value().newId = obj->idName();
            i.value().newName = obj->name();
            p_->ids.insert(i.value().oldId, i.value().newId);
            // and initialize to rewire
            i.value().createCopy = true;

            // switch default off for sequences on tracks
            // (since normally the tracks are the modulators)
            if (obj->isSequence() && obj->parentObject() && obj->parentObject()->isTrack())
                i.value().createCopy = false;

            ++i;
        }
    }
}


bool KeepModulators::modulatorsPresent() const
{
    return !p_->mods.isEmpty();
}

void KeepModulators::createNewModulators()
{
    MO_DEBUG_MOD("KeepModulators::createNewModulators()");

    for (const Private::Mod & m : p_->mods)
    if (m.createCopy && m.newId != m.oldId)
    {
        MO_DEBUG_MOD("KeepModulators: creating path " << m.newId
                 << " -> " << m.param->infoName());

        p_->scene->editor()->addModulator(m.param, m.newId, "");

        // copy modulator settings
        Modulator
                * oldm = m.oldMod,//param->findModulator(m.oldId),
                * newm = m.param->findModulator(m.newId);
        if (oldm && newm)
            newm->copySettingsFrom(oldm);
        else
            MO_WARNING("Could not copy Modulator settings for path '"
                       << m.newId << " -> " << m.param->infoName() << "' "
                       << "(" << oldm << ", " << newm << ")");
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

    settings()->restoreGeometry(this);

    createWidgets_();
    createList_();
}

KeepModulatorDialog::~KeepModulatorDialog()
{
    settings()->storeGeometry(this);
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

            auto but = new QPushButton(tr("Select all"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SLOT(selectAll_()));

            but = new QPushButton(tr("Unselect all"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SLOT(unselectAll_()));

            but = new QPushButton(tr("Flip selection"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SLOT(flipAll_()));

            lh->addStretch(1);

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            but = new QPushButton(tr("Ok"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SLOT(doit_()));

            but = new QPushButton(tr("Cancel"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SLOT(reject()));
}

void KeepModulatorDialog::createList_()
{
    list_->clear();
    for (auto & m : mods_.p_->mods)
    if (!m.newId.isEmpty())
    {
        // prepare an entry
        auto item = new QListWidgetItem(list_);
        item->setText(QString("create \"%1\" <- \"%2\" (copy of \"%3\")")
                      .arg(m.param->infoName())
                      .arg(m.newName)
                      .arg(m.oldName)
                      );
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item->setCheckState(m.createCopy ? Qt::Checked : Qt::Unchecked);
        item->setIcon(AppIcons::iconForObject(m.object));
        list_->addItem(item);
    }
}


void KeepModulatorDialog::doit_()
{
    // copy checked-state
    int k=0;
    for (auto & m : mods_.p_->mods)
    {
        if (k < list_->count())
            m.createCopy = list_->item(k)->checkState() == Qt::Checked;

        ++k;
    }

    mods_.createNewModulators();
    accept();
}

void KeepModulatorDialog::selectAll_()
{
    for (int i=0; i<list_->count(); ++i)
    {
        auto item = list_->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void KeepModulatorDialog::unselectAll_()
{
    for (int i=0; i<list_->count(); ++i)
    {
        auto item = list_->item(i);
        item->setCheckState(Qt::Unchecked);
    }
}

void KeepModulatorDialog::flipAll_()
{
    for (int i=0; i<list_->count(); ++i)
    {
        auto item = list_->item(i);
        item->setCheckState(
                item->checkState() == Qt::Checked
                    ? Qt::Unchecked : Qt::Checked);
    }
}

} // namespace GUI
} // namespace MO
