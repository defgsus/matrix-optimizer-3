/** @file frontpreset.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.03.2015</p>
*/

#ifndef MOSRC_GUI_UTIL_FRONTPRESET_H
#define MOSRC_GUI_UTIL_FRONTPRESET_H

#include <memory>

#include <QString>
#include <QVariant>
#include <QList>

#include "types/refcounted.h"

namespace MO {
class Properties;
namespace IO { class XmlStream; }
namespace GUI {

/** (Currently) A container of QVariant for FrontItems,
    with place for additional infos (name, author, ...).
    Uses MO::Properties to handle it's values.

    @todo Could be more general maybe? */
class FrontPreset : public RefCounted
{
    ~FrontPreset();
public:

    FrontPreset(const QString& name = QString("Preset"));

    /** Swaps the contents with another instance. */
    void swap(FrontPreset&);

    // ----------------- io -------------------

    /** Writes the preset to an xml stream.
        The stream must be writeable.
        On return, the section is the same as on entry.
        @throws IoException on any error. */
    void serialize(IO::XmlStream& io) const;
    /** Reads the preset from an xml stream.
        The stream must be readable and current section
        must be the one created by serialize().
        On return, the section is the same as on entry.
        @throws IoException on any error. All data is left unchanged on errors. */
    void deserialize(IO::XmlStream& io);

    /** Returns a new Preset from an xml stream.
        The stream must be readable and current section
        must be the one created by serialize().
        On return, the section is the same as on entry.
        @throws IoException on any errors. */
    static FrontPreset * fromStream(IO::XmlStream& io);

    // ------------- getter -------------------

    const QString& name() const { return p_name_; }

    QVariant value(const QString& id) const;

    // ------------- setter -------------------

    void setName(const QString& name) { p_name_ = name; }
    void setValue(const QString& id, const QVariant& value);

private:

    QString p_name_;
    Properties * p_props_;
};




/** A collection of FrontPreset */
class FrontPresets : public RefCounted
{
    ~FrontPresets() { }
public:

    FrontPresets(const QString& name = QString("Presets"));

    // ----------------- io -------------------

    /** Writes the presets to an xml stream.
        The stream must be writeable.
        The section @section is created.
        Presets are written into individual sub-sections named by @p preset_section.
        On return, the section is the same as on entry.
        @throws IoException on any error. */
    void serialize(IO::XmlStream& io,
                   const QString& section = QString("ui-presets"),
                   const QString& preset_section = QString("ui-preset")) const;

    /** Reads the presets from an xml stream.
        The stream must be readable and current section
        must be the one created by serialize().
        All sub-sections with the name given by @p preset_section
        are treated as preset and will be parsed.
        On return, the section is the same as on entry.
        @throws IoException on any error. Data is left unchanged on io errors. */
    void deserialize(IO::XmlStream& io,
                     const QString& preset_section = QString("ui-preset"));

    /** Creates a new instance from an xml stream.
        The stream must be readable and current section
        must be the one created by serialize().
        All sub-sections with the name given by @p preset_section
        are treated as preset and will be parsed.
        On return, the section is the same as on entry.
        @throws IoException on any error. */
    static FrontPresets * fromStream(IO::XmlStream& io,
                                     const QString& preset_section = QString("ui-preset"));

    // ------------- getter -------------------

    const QString& name() const { return p_name_; }

    /** Returns the number of presets contained */
    uint numPresets() const;

    /** Returns the preset for @p id, or NULL */
    FrontPreset * preset(const QString& id);
    /** Returns the preset for @p id, or NULL */
    const FrontPreset * preset(const QString& id) const;

    /** Returns the list of all presets */
    QList<FrontPreset*> presets();

    /** Returns the list of all presets */
    QList<const FrontPreset*> presets() const;

    // ------------- setter -------------------

    void setName(const QString& name) { p_name_ = name; }

    /** Removes all presets and releases the references */
    void clear();

    /** Creates a new (or returns existing) preset for the given @p id. */
    FrontPreset * newPreset(const QString& id, const QString& name = QString("preset"));

    /** Appends or replaces the preset for the given @p id.
        The caller can release the reference on the preset afterwards. */
    void setPreset(const QString& id, FrontPreset * preset);

    /** Removes the given preset from the list and releases the reference */
    void removePreset(const QString& id);

private:

    QString p_name_;
    QMap<QString, std::shared_ptr<FrontPreset>> p_map_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_UTIL_FRONTPRESET_H
