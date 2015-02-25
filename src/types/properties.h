/** @file properties.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#ifndef MOSRC_TYPES_PROPERTIES_H
#define MOSRC_TYPES_PROPERTIES_H

#include <QVariant>
#include <QMap>

namespace MO {
namespace IO { class DataStream; }

/** Generic property container */
class Properties
{
public:

    typedef QMap<QString, QVariant> Map;

    Properties();

    // ------------------ io --------------------

    void serialize(IO::DataStream&) const;
    void deserialize(IO::DataStream&);

    // ---------------- getter ------------------

    /** Returns the given property, or an invalid QVariant */
    QVariant get(const QString& id) const;

    /** Returns the given property, or the default value. */
    QVariant get(const QString& id, const QVariant& def) const;

    /** Returns true, when there is a property named @p id */
    bool has(const QString& id) const { return p_props_.contains(id); }

    // -------------- iterator ------------------

    Map::const_iterator begin() const { return p_props_.constBegin(); }
    Map::const_iterator end() const { return p_props_.constEnd(); }

    // --------------- setter -------------------

    /** Wipes out everything */
    void clear() { p_props_.clear(); }

    /** Sets the given property */
    void set(const QString& id, const QVariant&);

    /** Merge the settings from another container */
    void merge(const Properties& other);

private:

    Map p_props_;
};

} // namespace MO

#endif // MOSRC_TYPES_PROPERTIES_H
