/** @file frontpreset.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.03.2015</p>
*/

#include "frontpreset.h"
#include "types/properties.h"
#include "io/xmlstream.h"

namespace MO {
namespace GUI {

FrontPreset::FrontPreset(const QString& name)
    : p_name_       (name)
    , p_props_      (new Properties)

{

}

FrontPreset::~FrontPreset()
{
    delete p_props_;
}

void FrontPreset::swap(FrontPreset & o)
{
    if (&o == this)
        return;

    std::swap(p_name_, o.p_name_);
    p_props_->swap(*o.p_props_);
}

// ----------------- io -------------------

void FrontPreset::serialize(IO::XmlStream& io) const
{
    io.write("version", 1);
    io.write("name", p_name_);

    p_props_->serialize(io);
}


void FrontPreset::deserialize(IO::XmlStream& io)
{
    auto tmp = fromStream(io);
    swap(*tmp);
}

FrontPreset * FrontPreset::fromStream(IO::XmlStream& io)
{
    const int ver = io.expectInt("version");
    Q_UNUSED(ver);

    QString name = io.expectString("name");

    // create new instance for loading
    auto tmp = new FrontPreset(name);
    std::shared_ptr<FrontPreset> ptmp(tmp, RefCountedDeleter());

    while (io.nextSubSection())
    {
        if (io.section() == "properties")
            tmp->p_props_->deserialize(io);

        io.leaveSection();
    }

    ptmp.reset();
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
    : p_name_       (name)
{

}


// ----------------- io -------------------

void FrontPresets::serialize(IO::XmlStream& io,
                             const QString& section,
                             const QString& preset_section) const
{
    io.newSection(section);

        io.write("version", 1);
        io.write("name", p_name_);

        for (auto i = p_map_.begin(); i != p_map_.end(); ++i)
        {
            io.newSection(preset_section);
                io.write("preset-id", i.key());
                i.value()->serialize(io);
            io.endSection();
        }

    io.endSection();
}


void FrontPresets::deserialize(IO::XmlStream& io, const QString& preset_section)
{
    auto tmp = fromStream(io, preset_section);

    clear();

    p_name_ = tmp->p_name_;

    // copy each preset
    for (auto i = tmp->p_map_.begin(); i != tmp->p_map_.end(); ++i)
    {
        p_map_.insert(i.key(), i.value());
        i.value().get()->addRef();
    }
}

FrontPresets * FrontPresets::fromStream(IO::XmlStream& io, const QString& preset_section)
{
    const int ver = io.expectInt("version");
    Q_UNUSED(ver);

    QString name = io.expectString("name");

    // create instance for loading
    auto tmp = new FrontPresets(name);
    std::shared_ptr<FrontPresets> ptmp(tmp, RefCountedDeleter());

    while (io.nextSubSection())
    {
        if (io.section() == preset_section)
        {
            QString id = io.expectString("preset-id");

            auto p = FrontPreset::fromStream(io);
            tmp->setPreset(id, p);
            p->releaseRef();
        }

        io.leaveSection();
    }

    ptmp.reset();
    return tmp;
}

// ------------- getter -------------------

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
    auto pp = std::shared_ptr<FrontPreset>(p, RefCountedDeleter());
    p_map_.insert(id, pp);

    return p;
}

void FrontPresets::setPreset(const QString &id, FrontPreset *preset)
{
    // replace?
    p_map_.remove(id);

    auto pp = std::shared_ptr<FrontPreset>(preset, RefCountedDeleter());
    p_map_.insert(id, pp);
}

void FrontPresets::removePreset(const QString &id)
{
    p_map_.remove(id);
}


} // namespace GUI
} // namespace MO
