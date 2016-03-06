/** @file frontpreset.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.03.2015</p>
*/

#include "frontpreset.h"
#include "types/properties.h"
#include "io/xmlstream.h"

#if 0
#   include "io/log.h"
#   define MO_DEBUG_PRESET(arg__) MO_DEBUG("FrontPreset(" << this << ")::" << arg__)
#   define MO_DEBUG_PRESETS(arg__) MO_DEBUG("FrontPresets(" << this << ")::" << arg__)
#else
#   define MO_DEBUG_PRESET(unused__)
#   define MO_DEBUG_PRESETS(unused__)
#endif

namespace MO {
namespace GUI {

FrontPreset::FrontPreset(const QString& name)
    : RefCounted("FrontPreset")
    , p_name_       (name)
    , p_props_      (new Properties)

{
    MO_DEBUG_PRESET("FrontPreset(" << name << ")");
}

FrontPreset::~FrontPreset()
{
    MO_DEBUG_PRESET("~FrontPreset()");

    delete p_props_;
}

void FrontPreset::swap(FrontPreset & o)
{
    MO_DEBUG_PRESET("swap(" << &o << ")");

    if (&o == this)
        return;

    std::swap(p_name_, o.p_name_);
    p_props_->swap(*o.p_props_);
}

// ----------------- io -------------------

void FrontPreset::serialize(IO::XmlStream& io) const
{
    MO_DEBUG_PRESET("serialize(" << &io << ") section = '" << io.section() << "'");

    io.write("version", 1);
    io.write("name", p_name_);

    p_props_->serialize(io);
}


void FrontPreset::deserialize(IO::XmlStream& io)
{
    MO_DEBUG_PRESET("deserialize(" << &io << ") section = '" << io.section() << "'");

    auto tmp = fromStream(io);
    swap(*tmp);
}

FrontPreset * FrontPreset::fromStream(IO::XmlStream& io)
{
    //MO_DEBUG_PRESET("fromStream(" << &io << ")");

    const int ver = io.expectInt("version");
    Q_UNUSED(ver);

    QString name = io.expectString("name");

    // create new instance for loading
    auto tmp = new FrontPreset(name);
    std::shared_ptr<FrontPreset> ptmp(tmp, RefCountedDeleter("FrontPresest load"));

    while (io.nextSubSection())
    {
        if (io.section() == "properties")
            tmp->p_props_->deserialize(io);

        io.leaveSection();
    }

    tmp->addRef("FrontPreset load finish");
    return tmp;
}


// ------------- getter -------------------

QVariant FrontPreset::value(const QString& id) const
{
    return p_props_->get(id);
}

// ------------- setter -------------------

void FrontPreset::setValue(const QString& id, const QVariant& value)
{
    p_props_->set(id, value);
}







// ----------------------------------- presetS ---------------------------------------


FrontPresets::FrontPresets(const QString& name)
    : RefCounted("FrontPresets")
    , p_name_       (name)
{
    MO_DEBUG_PRESETS("FrontPresets(" << name << ")");
}

FrontPresets::FrontPresets(const FrontPresets &other)
    : RefCounted("FrontPresets")
{
    copyFrom(other);
}

FrontPresets::~FrontPresets()
{
    MO_DEBUG_PRESETS("~FrontPreset()");
}

// ----------------- io -------------------

void FrontPresets::serialize(IO::XmlStream& io,
                             const QString& section,
                             const QString& preset_section) const
{
    MO_DEBUG_PRESETS("serialize(" << &io << ", " << section << ", " << preset_section << ") section = '" << io.section() << "'");

    io.createSection(section);

        io.write("version", 1);
        io.write("name", p_name_);

        for (auto i = p_map_.begin(); i != p_map_.end(); ++i)
        {
            io.createSection(preset_section);
                io.write("preset-id", i.key());
                i.value()->serialize(io);
            io.endSection();
        }

    io.endSection();

    //MO_DEBUG_PRESETS(io.data());
}


void FrontPresets::deserialize(IO::XmlStream& io, const QString& preset_section)
{
    MO_DEBUG_PRESETS("deserialize(" << &io << ", " << preset_section << ") section = '" << io.section() << "'");

    auto tmp = fromStream(io, preset_section);

    copyFrom(*tmp);
}

FrontPresets * FrontPresets::fromStream(IO::XmlStream& io, const QString& preset_section)
{
    //MO_DEBUG_PRESETS("fromStream(" << &io << ", " << preset_section << ")");

    const int ver = io.expectInt("version");
    Q_UNUSED(ver);

    QString name = io.expectString("name");

    // create instance for loading
    auto tmp = new FrontPresets(name);
    std::shared_ptr<FrontPresets> ptmp(tmp, RefCountedDeleter("FrontPreset streamload"));

    while (io.nextSubSection())
    {
        if (io.section() == preset_section)
        {
            QString id = io.expectString("preset-id");

            auto p = FrontPreset::fromStream(io);
            tmp->setPreset(id, p);
            p->releaseRef("FrontPreset load add finish");
        }

        io.leaveSection();
    }

    tmp->addRef("FrontPreset load finish");
    return tmp;
}


void FrontPresets::saveFile(const QString &filename)
{
    MO_DEBUG_PRESETS("saveFile(" << filename << ")");

    IO::XmlStream xml;
    xml.startWriting();
    serialize(xml, "ui-presets", "ui-preset");
    xml.stopWriting();
    xml.save(filename);
}

void FrontPresets::loadFile(const QString &filename)
{
    MO_DEBUG_PRESETS("loadFile(" << filename << ")");

    IO::XmlStream xml;
    xml.load(filename);
    xml.startReading();
    while (xml.nextSubSection())
    {
        if (xml.section() == "ui-presets")
        {
            deserialize(xml, "ui-preset");
            break;
        }
        xml.leaveSection();
    }
    xml.stopReading();
}


void FrontPresets::copyFrom(const FrontPresets &other)
{
    if (&other == this)
        return;

    clear();

    p_name_ = other.p_name_;

    // copy each preset
    for (auto i = other.p_map_.begin(); i != other.p_map_.end(); ++i)
    {
        p_map_.insert(i.key(), i.value());
        // XXX not needed when copy-assigning shared-ptr, isit?
        //i.value().get()->addRef();
    }
}







// ------------- getter -------------------

QString FrontPresets::toString() const
{
    QString r;
    for (auto & i : p_map_)
        r += i->name() + i->properties().toString() + "\n";
    return r;
}

uint FrontPresets::numPresets() const { return p_map_.size(); }

FrontPreset * FrontPresets::preset(const QString& id)
{
    auto i = p_map_.find(id);
    return i == p_map_.end()
            ? 0
            : i.value().get();
}

const FrontPreset * FrontPresets::preset(const QString& id) const
{
    auto i = p_map_.find(id);
    return i == p_map_.end()
            ? 0
            : i.value().get();
}

QList<FrontPreset*> FrontPresets::presets()
{
    QList<FrontPreset*> list;
    for (auto & i : p_map_)
        list << i.get();
    return list;
}

QList<const FrontPreset*> FrontPresets::presets() const
{
    QList<const FrontPreset*> list;
    for (auto & i : p_map_)
        list << i.get();
    return list;
}

QList<QPair<FrontPreset*, QString>> FrontPresets::presetsIds()
{
    QList<QPair<FrontPreset*, QString>> list;
    for (auto i = p_map_.begin(); i != p_map_.end(); ++i)
        list << qMakePair(i.value().get(), i.key());
    return list;
}

QList<QPair<const FrontPreset*, QString>> FrontPresets::presetsIds() const
{
    QList<QPair<const FrontPreset*, QString>> list;
    for (auto i = p_map_.begin(); i != p_map_.end(); ++i)
        list << QPair<const FrontPreset*, QString>(i.value().get(), i.key());
    return list;
}

QString FrontPresets::uniqueId() const
{
    //static uint count = 0;
    uint count = numPresets();

    QString id = QString("preset%1").arg(count);
    while (preset(id))
        id = QString("preset%1").arg(++count);

    return id;
}

// ------------- setter -------------------

void FrontPresets::clear()
{
    p_map_.clear();
}

FrontPreset * FrontPresets::newPreset(const QString& id, const QString& name)
{
    if (auto p = preset(id))
        return p;

    auto p = new FrontPreset(name);
    auto pp = std::shared_ptr<FrontPreset>(p, RefCountedDeleter("FrontPresets new preset"));
    p_map_.insert(id, pp);

    return p;
}

void FrontPresets::setPreset(const QString &id, FrontPreset *preset)
{
    // replace?
    p_map_.remove(id);

    auto pp = std::shared_ptr<FrontPreset>(preset, RefCountedDeleter("FrontPresets set preset"));
    p_map_.insert(id, pp);
    preset->addRef("FrontPresets set preset");
}

void FrontPresets::removePreset(const QString &id)
{
    p_map_.remove(id);
}


} // namespace GUI
} // namespace MO
