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

/** Generic property container.
    This thing supports all QVariant types.
    Extending QVariant types is a bit tricky though.

    In general, it's not clear whether it was a good idea,
    to directly use QVariant as storage type.
    Pro: It's a generic type so should work with everything else in Qt.
    Contra: No default values, no tranlated name, etc...
            (but can be added via methods in this class)
*/
class Properties
{
public:

    // --------------- types --------------------

    /** The default key/value map used for all Properties */
    typedef QMap<QString, QVariant> Map;

    /** Helper for value lists,
        e.g. enums and such.
        Adding instances of this class to the Properties framework
        only requires updating code in properties.h/cpp.
        (search for "// [add new NamedStates here]") */
    struct NamedStates
    {
        /** Helper struct to feed in static values to NamedStates constructor */
        struct NamedStateHelper
        {
            QString id;
            QVariant v;
            NamedStateHelper(const QString& id, const QVariant& v) : id(id), v(v) { }
        };

        /** Construct a set by supplying tuples, e.g.:
            @code
            const NamedStates states("the-states",
            {
                { "state-1", 1 },
                { "state-2", QColor(Qt::red) }
            });
            @endcode
            Both @p persistent_name and the IDs in @p tuples
            must not change for file persistence. */
        NamedStates(const QString& persitent_name, const QList<NamedStateHelper>& tuples);
        /** Construct a set by supplying two separate lists for keys and values.
            Both @p persistent_name and the @p ids
            must not change for file persistence.*/
        NamedStates(const QString& persistent_name, const QList<QString>& ids, const QList<QVariant> values);

        // ----------- getter ----------

        /** Returns the name of the state set */
        const QString& name() const { return p_name_; }

        /** Returns value for id, or invalid QVariant */
        const QVariant& value(const QString& id) const;
        /** Returns id for value, or null QString */
        const QString& id(const QVariant& value) const;

        /** Number of different values */
        uint size() const { return p_sv_.size(); }

        /** Const iterator through all possible values */
        Map::const_iterator begin() const { return p_sv_.constBegin(); }
        /** Const iterator through all possible values */
        Map::const_iterator end() const { return p_sv_.constEnd(); }

        private:
        Map p_sv_;
        QString p_name_;
    };

    /** Returns true when the variant wraps a value belonging to a
        NamedStates set. */
    static bool isNamedStates(const QVariant&);
    /** Returns a pointer to the static instance of NamedStated which
        belongs to the value wrapped in the QVariant,
        or NULL if the value does not belong to such a class. */
    static const NamedStates * getNamedStates(const QVariant&);

    /** Returns a pointer to the static instance of NamedStated
        with the given @p name, or NULL if that name doesn't exists. */
    static const NamedStates * getNamedStates(const QString& name);

    // ------------------- enums ------------------

    // [add new NamedStates here]

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
    static const NamedStates alignmentStates;
    static bool isAlignment(const QVariant&);


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

    /** Returns a css-style list of all properties */
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
    static QRectF align(const QRectF& rect, const QRectF& parent,
                        int alignment = A_CENTER, qreal margin = 0.0);

private:

    void p_set_(const QString& id, const QVariant& v);

    Map p_props_;
};

} // namespace MO



// [add new NamedStates here]

Q_DECLARE_METATYPE(MO::Properties::Alignment)


#endif // MOSRC_TYPES_PROPERTIES_H
