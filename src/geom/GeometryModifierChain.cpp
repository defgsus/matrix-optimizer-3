/** @file geometrymodifierchain.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "GeometryModifierChain.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log_geom.h"
#include "GeometryModifier.h"
#include "GeometryModifierCreate.h"

namespace MO {
namespace GEOM {

QMap<QString, GeometryModifier*> * GeometryModifierChain::registeredModifiers_ = 0;

GeometryModifierChain::GeometryModifierChain()
    : curMod_       (0)
{
    MO_DEBUG_GEOM("GeometryModifierChain::GeometryModifierChain()");
}

GeometryModifierChain::~GeometryModifierChain()
{
    MO_DEBUG_GEOM("GeometryModifierChain::~GeometryModifierChain()");
    clear();
}

GeometryModifierChain::GeometryModifierChain(const GeometryModifierChain &other)
{
    MO_DEBUG_GEOM("GeometryModifierChain::GeometryModifierChain(&other)");
    *this = other;
}

GeometryModifierChain& GeometryModifierChain::operator = (const GeometryModifierChain& other)
{
    MO_DEBUG_GEOM("GeometryModifierChain::operator = (&other)");

    clear();

    for (auto m : other.modifiers_)
    {
        addModifier(m->clone());
    }

    return *this;
}

bool GeometryModifierChain::operator != (const GeometryModifierChain& o) const
{
    if (modifiers_.size() != o.modifiers_.size())
        return true;

    for (int i=0; i<modifiers_.size(); ++i)
        if (*modifiers_[i] != *o.modifiers_[i])
            return true;
    return false;
}


void GeometryModifierChain::clear()
{
    MO_DEBUG_GEOM("GeometryModifierChain::clear()");

    curMod_ = 0;
    for (auto m : modifiers_)
        delete m;
    modifiers_.clear();
}


void GeometryModifierChain::serialize(IO::DataStream &io) const
{
    MO_DEBUG_GEOM("GeometryModifierChain::serialize()");

    io.writeHeader("geommodchain", 1);

    io << (quint32)modifiers_.size();

    for (GeometryModifier * m : modifiers_)
    {
        io << m->className();

        qint64 skip = io.beginSkip();

        m->serialize(io);

        io.endSkip(skip);
    }
}

void GeometryModifierChain::deserialize(IO::DataStream &io)
{
    MO_DEBUG_GEOM("GeometryModifierChain::deserialize()");

    clear();

    io.readHeader("geommodchain", 1);

    quint32 num;
    io >> num;

    for (uint i=0; i<num; ++i)
    {
        QString className;
        io >> className;

        qint64 skiplen;
        io >> skiplen;

        GeometryModifier * geom = createModifier(className);
        if (!geom)
        {
            MO_IO_WARNING(VERSION_MISMATCH,
                          "skipped unknown geometry modifier '" << className << "'");
            io.skip(skiplen);
            continue;
        }

        geom->deserialize(io);

        addModifier(geom);
    }
}

QList<QString> GeometryModifierChain::modifierClassNames()
{
    QList<QString> list;
    for (auto i = registeredModifiers_->begin();
                i!=registeredModifiers_->end(); ++i)
        list.append(i.key());
    return list;
}

QList<QString> GeometryModifierChain::modifierGuiNames()
{
    QList<QString> list;
    for (auto m : *registeredModifiers_)
        list.append(m->guiName());
    return list;
}

void GeometryModifierChain::getNeededFiles(IO::FileList &files) const
{
    for (auto m : modifiers())
        if (auto mc = dynamic_cast<GeometryModifierCreate*>(m))
            if (mc->isFile())
                files.append(IO::FileListEntry(mc->filename(), IO::FT_MODEL));
}

GeometryModifier * GeometryModifierChain::createModifier(const QString &className)
{
    if (!registeredModifiers_)
        registeredModifiers_ = new QMap<QString, GeometryModifier*>;

    auto i = registeredModifiers_->find(className);
    return (i == registeredModifiers_->end())? 0 : i.value()->cloneClass();
}

bool GeometryModifierChain::registerModifier(GeometryModifier *g)
{
    MO_DEBUG_GEOM("GeometryModifierChain::registerModifier('" << g->className() << "')");

    if (!registeredModifiers_)
        registeredModifiers_ = new QMap<QString, GeometryModifier*>;

    if (registeredModifiers_->contains(g->className()))
    {
        MO_WARNING("duplicate GeometryModifier '" << g->className() << "' registered!");
        return false;
    }

    registeredModifiers_->insert(g->className(), g);
    return true;
}

void GeometryModifierChain::addModifier(GeometryModifier *g)
{
    modifiers_.append(g);
}

void GeometryModifierChain::insertModifier(
        GeometryModifier * geom, uint index)
{
    modifiers_.insert(index, geom);
}

GeometryModifier * GeometryModifierChain::addModifier(const QString &className)
{
    GeometryModifier * geom = createModifier(className);
    if (!geom)
    {
        MO_WARNING("request for unknown GeometryModifier '" << className << "'");
        return 0;
    }

    addModifier(geom);
    return geom;
}

GeometryModifier * GeometryModifierChain::insertModifier(
        const QString &className, GeometryModifier *before)
{
    GeometryModifier * geom = createModifier(className);
    if (!geom)
    {
        MO_WARNING("request for unknown GeometryModifier '" << className << "'");
        return 0;
    }

    const int idx = modifiers_.indexOf(before);
    if (idx < 0)
        addModifier(geom);
    else
        modifiers_.insert(idx, geom);

    return geom;
}

GeometryModifier * GeometryModifierChain::insertModifier(
        const QString &className, uint index)
{
    GeometryModifier * geom = createModifier(className);
    if (!geom)
    {
        MO_WARNING("request for unknown GeometryModifier '" << className << "'");
        return 0;
    }

    modifiers_.insert(index, geom);

    return geom;
}


bool GeometryModifierChain::moveModifierUp(GeometryModifier *g)
{
    const int idx = modifiers_.indexOf(g);
    if (idx < 1)
        return false;

    modifiers_.move(idx, idx-1);

    return true;
}


bool GeometryModifierChain::moveModifierDown(GeometryModifier *g)
{
    const int idx = modifiers_.indexOf(g);
    if (idx < 0 || idx >= modifiers_.size()-1)
        return false;

    modifiers_.move(idx, idx+1);

    return true;
}

bool GeometryModifierChain::deleteModifier(GeometryModifier *g)
{
    if (!modifiers_.contains(g))
        return false;

    modifiers_.removeAll(g);
    delete g;
    curMod_ = 0;

    return true;
}

void GeometryModifierChain::execute(Geometry *g, Object * o) const
{
    doStop_ = false;
    curMod_ = 0;

    for (auto m : modifiers_)
    {
        if (doStop_)
            break;

        if (m->isEnabled())
        {
            curMod_ = m;
            m->executeBase(g, o);
        }
    }
}

void GeometryModifierChain::stop()
{
    doStop_ = true;
}

double GeometryModifierChain::progress() const
{
    if (curMod_)
    {
        // XXX Cast away volatile
        GeometryModifier * mod = (GeometryModifier*)(curMod_);
        return mod->progress();
    }
    else
        return 0.;
}


} // namespace GEOM
} // namespace MO
