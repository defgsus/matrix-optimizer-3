/** @file properties.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#ifndef MOSRC_TYPES_PROPERTIES_H
#define MOSRC_TYPES_PROPERTIES_H

#include <functional>

#include <QVariant>
#include <QMap>
#include <QRectF>

class QWidget;

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

    enum SubType
    {
        ST_ANY = 0,
        ST_TEXT = 0x1000,
        ST_FILENAME = 0x2000
    };
    static const int subTypeMask;


    /** A class for handling persistent
        choose-one-of-different-values properties */
    class NamedValues
    {
    public:
        struct Value
        {
            bool isValid() const { return v.isValid(); }
            QString id, name, tip;
            QVariant v;
        };

        bool has(const QString &id) const { return p_val_.contains(id); }
        bool hasValue(const QVariant &v) const;
        /** Returns the Value for id, or an invalid Value */
        const Value& get(const QString& id) const;
        /** Returns the Value matching v, or an invalid Value */
        const Value& getByValue(const QVariant& v) const;

        void set(const QString& id, const QString& name,
                 const QVariant& v);
        void set(const QString& id, const QString& name,
                 const QString& statusTip, const QVariant& v);

        /** Access to values, sorted by id */
        QMap<QString, Value>::const_iterator begin() const { return p_val_.cbegin(); }
        /** Access to values, sorted by id */
        QMap<QString, Value>::const_iterator end() const { return p_val_.cend(); }

    private:
        friend Properties;
        QMap<QString, Value> p_val_;
    };


    struct Property
    {
        Property() : p_subType_(-1), p_idx_(0), p_vis_(true) { }

        bool isValid() const { return !p_val_.isNull(); }

        const QVariant& value() const { return p_val_; }
        const QVariant& defaultValue() const { return p_def_; }
        const QVariant& minimum() const { return p_min_; }
        const QVariant& maximum() const { return p_max_; }
        const QVariant& step() const { return p_step_; }

        const QString& name() const { return p_name_; }
        const QString& tip() const { return p_tip_; }

        int subType() const { return p_subType_; }

        bool hasNamedValues() const;
        /** Associated NamedValues class, if any */
        const NamedValues& namedValues() const { return p_nv_; }

        // ---- gui stuff ----

        /** The order of creation */
        int index() const { return p_idx_; }
        bool isVisible() const { return p_vis_; }

        /** A user-callback for the created edit widget */
        const std::function<void(QWidget*)>&
            widgetCallback() const { return p_cb_widget_; }

    private:
        friend class Properties;
        QVariant
            p_val_,
            p_def_,
            p_min_,
            p_max_,
            p_step_;
        QString
            p_name_,
            p_tip_;
        int p_subType_,
            p_idx_;
        bool
            p_vis_;
        NamedValues p_nv_;
        std::function<void(QWidget*)> p_cb_widget_;
    };

    /** The default key/value map used for all Properties */
    typedef QMap<QString, Property> Map;


#if 0
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
#endif

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

    /** Returns the Property for the given id, or an invalid Property */
    const Property& getProperty(const QString& id) const;

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
        Some value types may have a associated sub-type:
        QString:
            ST_TEXT | MO::TextType (in object/object_fwd.h)
            ST_FILENAME | MO::IO::FileType (in io/filetypes.h)
        @returns -1 if not defined.
    */
    int getSubType(const QString& id) const;

    /** Returns true, when there is a property named @p id */
    bool has(const QString& id) const { return p_map_.contains(id); }

    /** Returns the default value for the property, or an invalid QVariant */
    bool hasDefault(const QString& id) const { return getProperty(id).defaultValue().isValid(); }
    bool hasMin(const QString& id) const { return getProperty(id).minimum().isValid(); }
    bool hasMax(const QString& id) const { return getProperty(id).maximum().isValid(); }
    bool hasStep(const QString& id) const { return getProperty(id).step().isValid(); }
    bool hasName(const QString& id) const { return !getProperty(id).name().isNull(); }
    bool hasTip(const QString& id) const { return !getProperty(id).tip().isNull(); }
    bool hasSubType(const QString& id) const { return getProperty(id).subType() != -1; }
    bool isVisible(const QString& id) const;

    /** Returns a css-style list of all properties */
    QString toString(const QString& indent = "") const;

    // -------------- iterator ------------------

    Map::const_iterator begin() const { return p_map_.constBegin(); }
    Map::const_iterator end() const { return p_map_.constEnd(); }

    // --------------- setter -------------------

    /** Wipes out everything */
    void clear();

    /** Removes a single item */
    void clear(const QString& id);

    /* Starts a new group. Adding a new property with set() will assign
        the property to this group
        XXX Not implemented */
    //void beginGroup(const QString&);

    /* Ends a property group. Same as calling beginGroup(""); */
    //void endGroup() { beginGroup(""); }

    /** Sets the given property (and default value) */
    void set(const QString& id, const QVariant& v);

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue);

    /** @{ */
    /** Initializers for integral or float types */

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue, const T& step);

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue, const T& minimum, const T& maximum);

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue, const T& minimum, const T& maximum, const T& step);
    /** @} */


    /** @{ */
    /** NamedValues */

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const NamedValues& names, const T& v);

    /* Initializer for selectable values, e.g. enum lists and such */
    /*void set(const QString& id, const QString& name, const QString& statusTip,
             const QStringList& valueIds,
             const QStringList& valueNames,
             const QVariantList& values,
             const QStringList& valueStatusTips = QStringList());*/

    /** @} */


    /** Sets the given default value */
    void setDefault(const QString& id, const QVariant& v);
    void setMin(const QString& id, const QVariant& v);
    void setMax(const QString& id, const QVariant& v);
    void setRange(const QString& id, const QVariant& mi, const QVariant& ma);
    void setStep(const QString& id, const QVariant& v);
    void setName(const QString& id, const QString& name);
    void setTip(const QString& id, const QString& statusTip);
    void setSubType(const QString& id, int t);
    void setNamedValues(const QString& id, const NamedValues& names);
    void setVisible(const QString& id, bool vis);

    /** Sets the given property if existing. */
    bool change(const QString& id, const QVariant& v);

    /** @{ */
    /** Helper to make sure, that user-extended QVariants get caught. */
    template <class T>
    void set(const QString& id, const T& v) { set(id, QVariant::fromValue(v)); }
    template <class T>
    bool change(const QString& id, const T& v) { return change(id, QVariant::fromValue(v)); }
    template <class T>
    void setDefault(const QString& id, const T& v) { setDefault(id, QVariant::fromValue(v)); }
    template <class T>
    void setMin(const QString& id, const T& v) { setMin(id, QVariant::fromValue(v)); }
    template <class T>
    void setMax(const QString& id, const T& v) { setMax(id, QVariant::fromValue(v)); }
    template <class T>
    void setRange(const QString& id, const T& mi, const T& ma) { setRange(id, QVariant::fromValue(mi), QVariant::fromValue(ma)); }
    template <class T>
    void setStep(const QString& id, const T& v) { setStep(id, QVariant::fromValue(v)); }
    /** @} */

    /** Copy all values from @p other.
        This creates or overwrites values for each value contained in @p other. */
    void unify(const Properties& other);

    /** Create a union of this and @p other,
        while prefering other's values over own. */
    Properties unified(const Properties& other) const { Properties p(*this); p.unify(other); return p; }

    // --------- callbacks -----------

    /** Installs a callback that is called by the gui to update
        the visibility of widgets after a value is changed.
        Changes other than to Property::setVisible() will not be
        reflected in the gui. */
    void setUpdateVisibilityCallback(std::function<void(Properties&)> f) { p_cb_vis_ = f; }
    /** Calls the user-callback and returns true if the visibility
        of any widget has changed. */
    bool callUpdateVisibility();


    /** Sets a callback that is called for the particular widget that is created
        for this property. The callback is called once after creation of the widget. */
    void setWidgetCallback(
            const QString& id, std::function<void(QWidget*)> f);

    /** Applies the user-callback from setWidgetCallback() to the given widget, if any. */
    void callWidgetCallback(const QString& id, QWidget * ) const;

    // -------------- static helper --------------------
#if 0
    /** Returns the aligned version of @p rect within @p parent.
        @p alignment is an OR combination of Alignment flags. */
    static QRectF align(const QRectF& rect, const QRectF& parent,
                        int alignment = A_CENTER, qreal margin = 0.0, bool outside = false);
#endif

private:

    Map p_map_;
    std::function<void(Properties&)> p_cb_vis_;
};

} // namespace MO


//Q_DECLARE_METATYPE(MO::Properties::NamedValues)


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
    setRange(id, minimum, maximum);
}

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const T& defaultValue, const T& minimum, const T& maximum, const T& step)
{
    set(id, defaultValue);
    setName(id, name);
    setTip(id, statusTip);
    setRange(id, minimum, maximum);
    setStep(id, step);
}

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const NamedValues& names, const T& v)
{
    set(id, v);
    setName(id, name);
    setTip(id, statusTip);
    setNamedValues(id, names);
}

} // namespace MO

#endif // MOSRC_TYPES_PROPERTIES_H
