/** @file geometrymodifierchain.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifierchain.h"
#include "io/datastream.h"
#include "io/error.h"

#include "geometrymodifier.h"


namespace MO {
namespace GEOM {

QMap<QString, GeometryModifier*> * GeometryModifierChain::registeredModifiers_ = 0;

GeometryModifierChain::GeometryModifierChain()
{
}

void GeometryModifierChain::serialize(IO::DataStream &io) const
{
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

GeometryModifier * GeometryModifierChain::createModifier(const QString &className)
{
    auto i = registeredModifiers_->find(className);
    return (i == registeredModifiers_->end())? 0 : i.value()->cloneClass();
}

bool GeometryModifierChain::registerModifier(GeometryModifier *g)
{
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

void GeometryModifierChain::execute(Geometry *g)
{
    for (auto m : modifiers_)
        m->execute(g);
}

} // namespace GEOM
} // namespace MO
