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
#include <QRectF>

namespace MO {
namespace IO { class DataStream; }

/** Generic property container */
class Properties
{
public:

    // --------------- types --------------------

    enum Types
    {
        T_ALIGNMENT = QMetaType::User + 1024
    };

    enum Alignment
    {
        A_LEFT = 1,
        A_RIGHT = 1<<1,
        A_TOP = 1<<2,
        A_BOTTOM = 1<<3,
        A_HCENTER = A_LEFT | A_RIGHT,
        A_VCENTER = A_TOP | A_BOTTOM,
        A_CENTER = A_HCENTER | A_VCENTER
    };
    static const uint AlignmentType;
    static QString alignmentToName(uint a);
    static uint alignmentFromName(const QString&);


    typedef QMap<QString, QVariant> Map;

    // -------------- ctor ----------------------

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

    /** Returns a css-sytle list of all properties */
    QString toString(const QString& indent = "") const;

    // -------------- iterator ------------------

    Map::const_iterator begin() const { return p_props_.constBegin(); }
    Map::const_iterator end() const { return p_props_.constEnd(); }

    // --------------- setter -------------------

    /** Wipes out everything */
    void clear() { p_props_.clear(); }

    /** Removes a single item */
    void clear(const QString& id) { p_props_.remove(id); }

    /** Sets the given property */
    template <class T>
    void set(const QString& id, const T& v) { p_set_(id, QVariant::fromValue(v)); }

    /** Merge the settings from another container */
    void merge(const Properties& other);


    // -------------- static helper --------------------

    /** Returns the aligned version of @p rect within @p parent.
        @p alignment is an OR combination of Alignment flags. */
    static QRectF align(const QRectF& rect, const QRectF& parent, int alignment);

private:

    void p_set_(const QString& id, const QVariant& v);

    Map p_props_;
};

} // namespace MO

Q_DECLARE_METATYPE(MO::Properties::Alignment)

#endif // MOSRC_TYPES_PROPERTIES_H
