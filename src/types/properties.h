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
namespace IO { class DataStream; class XmlStream; }

/** Generic property container.
    This thing supports all QVariant types.
    Extending QVariant types is a bit tricky though.

    In general, it's not clear whether it was a good idea,
    to directly use QVariant as storage type.
    Pro: It's a generic type so should work with everything else in Qt.
    Contra: No default values, no tranlated name, etc...
            (but can be added via methods in this class)
            UPDATE: Have been added now...
            @todo NamedStates is still a bit hacky..
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

    void swap(Properties& other);

    // ------------------ io --------------------

    void serialize(IO::DataStream&) const;
    void deserialize(IO::DataStream&);

    /** Writes the properties to an xml stream.
        The stream must be writeable.
        The section "properties" is created.
        On return, the section is the same as on entry. */
    void serialize(IO::XmlStream&) const;

    /** Reads the properties from an xml stream.
        This will overwrite all values contained in the xml
        and leave all other values untouched (like unify()).
        The stream must be readable and current section
        must be "properties".
        On return, the section is the same as on entry.
        @throws IoException. This class' data is left unchanged on errors. */
    void deserialize(IO::XmlStream&);

    // ---------------- getter ------------------

    /* XXX Might by helpful but is single-client!?!?:
        Returns true when properties are new or have changed,
        and clearChanged() has not been called. */
    //bool isChanged() const;

    /** Returns the given property, or an invalid QVariant */
    QVariant get(const QString& id) const;

    /** Returns the given property, or the given default value. */
    QVariant get(const QString& id, const QVariant& def) const;

    /** Returns the default value for the property, or an invalid QVariant */
    QVariant getDefault(const QString& id) const;
    QVariant getMin(const QString& id) const;
    QVariant getMax(const QString& id) const;
    QVariant getStep(const QString& id) const;
    QString getName(const QString& id) const;
    QString getTip(const QString& id) const;
    /** Returns the subtype of the value.
        Some value types may have a associated type:
        QString: MO::TextType (in object/object_fwd.h)
        @returns -1 if not defined.
    */
    int getSubType(const QString& id) const;

    /** Returns true, when there is a property named @p id */
    bool has(const QString& id) const { return p_val_.contains(id); }

    /** Returns the default value for the property, or an invalid QVariant */
    bool hasDefault(const QString& id) const { return p_def_.contains(id); }
    bool hasMin(const QString& id) const { return p_min_.contains(id); }
    bool hasMax(const QString& id) const { return p_max_.contains(id); }
    bool hasStep(const QString& id) const { return p_step_.contains(id); }
    bool hasName(const QString& id) const { return p_name_.contains(id); }
    bool hasTip(const QString& id) const { return p_tip_.contains(id); }
    bool hasSubType(const QString& id) const { return p_subType_.contains(id); }

    /** Returns a css-style list of all properties */
    QString toString(const QString& indent = "") const;

    // -------------- iterator ------------------

    Map::const_iterator begin() const { return p_val_.constBegin(); }
    Map::const_iterator end() const { return p_val_.constEnd(); }

    // --------------- setter -------------------

    /** Wipes out everything */
    void clear();

    /** Removes a single item */
    void clear(const QString& id);

    /* Starts a new group. Adding a new property with set() will assign
        the property to this group */
    void beginGroup(const QString&);

    /* Ends a property group. Same as calling beginGroup(""); */
    void endGroup() { beginGroup(""); }

    /** Sets the given property (and default value) */
    void set(const QString& id, const QVariant& v);
    /** Sets the given property.
        Helper to make sure, that user-extended QVariants get caught. */
    template <class T>
    void set(const QString& id, const T& v) { set(id, QVariant::fromValue(v)); }

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue);

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue, const T& step);

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue, const T& minimum, const T& maximum);

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue, const T& minimum, const T& maximum, const T& step);

    /** Sets the given default value */
    void setDefault(const QString& id, const QVariant& v);
    void setMin(const QString& id, const QVariant& v);
    void setMax(const QString& id, const QVariant& v);
    void setRange(const QString& id, const QVariant& mi, const QVariant& ma);
    void setStep(const QString& id, const QVariant& v);
    void setName(const QString& id, const QString& name);
    void setTip(const QString& id, const QString& statusTip);
    void setSubType(const QString& id, int t);

    /** Sets the given property if existing. */
    bool change(const QString& id, const QVariant& v);
    /** Sets the given property if existing.
        Helper to make sure, that user-extended QVariants get caught. */
    template <class T>
    bool change(const QString& id, const T& v) { return change(id, QVariant::fromValue(v)); }

    /** Copy all values from @p other.
        This creates or overwrites values for each value contained in @p other. */
    void unify(const Properties& other);

    /** Create a union of this and @p other,
        while prefering other's values over own. */
    Properties unified(const Properties& other) const { Properties p(*this); p.unify(other); return p; }


    // -------------- static helper --------------------

    /** Returns the aligned version of @p rect within @p parent.
        @p alignment is an OR combination of Alignment flags. */
    static QRectF align(const QRectF& rect, const QRectF& parent,
                        int alignment = A_CENTER, qreal margin = 0.0, bool outside = false);

private:

    /* XXX These could be refactured in, e.g.:
        QMap<QString, Property> */

    Map p_val_
      , p_def_
      , p_min_
      , p_max_
      , p_step_;

    QMap<QString, QString>
        p_name_,
        p_tip_;

    QMap<QString, int>
        p_subType_;
};

} // namespace MO


// [add new NamedStates here]

Q_DECLARE_METATYPE(MO::Properties::Alignment)


// --------- templ impl. ------------------
namespace MO {

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const T& defaultValue)
{
    set(id, defaultValue);
    setName(id, name);
    setTip(id, statusTip);
}

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const T& defaultValue, const T& step)
{
    set(id, defaultValue);
    setName(id, name);
    setTip(id, statusTip);
    setStep(id, step);
}

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const T& defaultValue, const T& minimum, const T& maximum)
{
    set(id, defaultValue);
    setName(id, name);
    setTip(id, statusTip);
    setMin(id, minimum);
    setMax(id, maximum);
}

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const T& defaultValue, const T& minimum, const T& maximum, const T& step)
{
    set(id, defaultValue);
    setName(id, name);
    setTip(id, statusTip);
    setMin(id, minimum);
    setMax(id, maximum);
    setStep(id, step);
}

} // namespace MO

#endif // MOSRC_TYPES_PROPERTIES_H
