/** @file properties.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#include <tuple>

#include "properties.h"
#include "io/datastream.h"
#include "io/xmlstream.h"
#include "io/error.h"
#include "io/log.h"

#if 0
#   define MO_DEBUG_PROP(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_PROP(unused__) { }
#endif

namespace MO {

#if 0
bool Properties::isAlignment(const QVariant & v) { return !strcmp(v.typeName(), "MO::Properties::Alignment"); }
const Properties::NamedStates Properties::alignmentStates = Properties::NamedStates("Alignment",
{
                { "left",           QVariant::fromValue(Alignment(A_LEFT)) },
                { "right",          QVariant::fromValue(Alignment(A_RIGHT)) },
                { "top",            QVariant::fromValue(Alignment(A_TOP)) },
                { "bottom",         QVariant::fromValue(Alignment(A_BOTTOM)) },
                { "top-left",       QVariant::fromValue(Alignment(A_TOP | A_LEFT)) },
                { "top-right",      QVariant::fromValue(Alignment(A_TOP | A_RIGHT)) },
                { "bottom-left",    QVariant::fromValue(Alignment(A_BOTTOM | A_LEFT)) },
                { "bottom-right",   QVariant::fromValue(Alignment(A_BOTTOM | A_RIGHT)) },
                { "vcenter-left",   QVariant::fromValue(Alignment(A_VCENTER | A_LEFT)) },
                { "vcenter-right",  QVariant::fromValue(Alignment(A_VCENTER | A_RIGHT)) },
                { "top-hcenter",    QVariant::fromValue(Alignment(A_TOP | A_HCENTER)) },
                { "bottom-hcenter", QVariant::fromValue(Alignment(A_BOTTOM | A_HCENTER)) },
                { "center",         QVariant::fromValue(Alignment(A_CENTER)) },
});
#endif


// --------------------- Properties::NamedValues ------------------------------

bool Properties::NamedValues::hasValue(const QVariant &v) const
{
    for (auto & val : p_val_)
        if (val.v == v)
            return true;
    return false;
}

const Properties::NamedValues::Value&
    Properties::NamedValues::getByValue(const QVariant &v) const
{
    for (auto & val : p_val_)
        if (val.v == v)
            return val;

    static Value invalid;
    return invalid;
}

const Properties::NamedValues::Value&
    Properties::NamedValues::get(const QString &id) const
{
    auto i = p_val_.find(id);
    static Value invalid;
    return i == p_val_.end() ? invalid : i.value();
}


void Properties::NamedValues::set(
        const QString &id, const QString &name, const QString &statusTip,
        const QVariant &v)
{
    auto i = p_val_.find(id);
    if (i != p_val_.end())
    {
        i.value().id = id;
        i.value().name = name;
        i.value().tip = statusTip;
        i.value().v = v;
    }
    else
    {
        Value val;
        val.id = id;
        val.name = name;
        val.tip = statusTip;
        val.v = v;
        p_val_.insert(id, val);
    }
}

void Properties::NamedValues::set(
        const QString &id, const QString &name,
        const QVariant &v)
{
    set(id, name, "", v);
}


bool Properties::Property::hasNamedValues() const
{
    return !p_nv_.p_val_.isEmpty();
}





// ---------------------------- Properties ------------------------------------

const int Properties::subTypeMask = 0xfff;

Properties::Properties()
{
}

void Properties::swap(Properties &other)
{
    if (&other == this)
        return;

    p_map_.swap(other.p_map_);
    std::swap(p_cb_vis_, other.p_cb_vis_);
}

void Properties::clear()
{
    p_map_.clear();
    p_cb_vis_ = 0;
}

void Properties::clear(const QString &id)
{
    p_map_.remove(id);
}

void Properties::serialize(IO::DataStream & io) const
{
    io.writeHeader("props", 1);

    // Note: Reimplementation of QDataStream << QMap<QString,QVariant>
    // because overloading operator << does not catch enums
    io << quint64(p_map_.size());
    for (auto i = p_map_.begin(); i != p_map_.end(); ++i)
    {
        io << i.key();

        // store id of named values
        if (i.value().hasNamedValues())
        {
            auto nv = i.value().namedValues().getByValue(
                        i.value().value());
            io << qint8(1) << nv.id;
        }

        // non-user types use the QVariant version
        else
        if (QMetaType::Type(i.value().value().type()) < QMetaType::User)
            io << qint8(0) << i.value().value();

        // handle special compound types
#define MO__IO(type__, flag__) \
        if (!strcmp(i.value().value().typeName(), #type__)) \
        { \
            io << qint8(flag__) \
               << i.value().value().value<type__>(); \
        }
        else
        MO__IO(QVector<float>, 24)
        else
        MO__IO(QVector<double>, 25)
        else
        MO__IO(QVector<int>, 26)
        else
        MO__IO(QVector<uint>, 27)
        else
        MO__IO(QVector<unsigned>, 28)

        else
        {
            io << qint8(-1);
            MO_IO_WARNING(WRITE, "Properties::serialize() unhandled QVariant '"
                       << i.value().value().typeName() << "'");
        }
#undef MO__IO

    }
}

void Properties::deserialize(IO::DataStream & io)
{
    //const auto ver =
            io.readHeader("props", 1);

    // reconstruct map
    Map temp;
    quint64 num;
    io >> num;
    for (quint64 i=0; i<num; ++i)
    {
        QString key;
        qint8 type;
        QVariant v;
        io >> key >> type;
        if (type == -1)
        {
            MO_IO_WARNING(READ, "Properties::deserialize() can't restore "
                       "unhandled QVariant for '" << key << "'");
            continue;
        }

#define MO__IO(type__, flag__) \
        if (type == flag__) \
        { \
            type__ val__; \
            io >> val__; \
            v = QVariant::fromValue(val__); \
        }

        // default QVariant
        if (type == 0)
            io >> v;

        // named values
        else if (type == 1)
        {
            QString id;
            io >> id;
            if (!has(key))
            {
                MO_IO_WARNING(READ, "Properties::deserialize() '" << key
                              << "' unknown named-value, ignored");
                continue;
            }
            auto p = getProperty(key);
            if (!p.hasNamedValues())
            {
                MO_IO_WARNING(READ, "Properties::deserialize() '" << key
                              << "' is expected to be a named value but is not, ignored");
                MO_PRINT(toString());
                continue;
            }
            if (!p.namedValues().has(id))
            {
                MO_IO_WARNING(READ, "Properties::deserialize() '" << key
                              << "' unknown named-value id '" << id << "', ignored");
                continue;
            }
            v = p.namedValues().get(id).v;
        }

        // user types
        else
        MO__IO(QVector<float>, 24)
        else
        MO__IO(QVector<double>, 25)
        else
        MO__IO(QVector<int>, 26)
        else
        MO__IO(QVector<uint>, 27)
        else
        MO__IO(QVector<unsigned>, 28)

        // unknown
        else
        {
            MO_IO_WARNING(READ, "Properties::deserialize() ignoring unknown usertype " << type
                       << " for item '" << key << "'");
            continue;
        }

        Property prop;
        prop.p_val_ = v;
        temp.insert(key, prop);
    }

    // finally assign
    for (auto i = temp.begin(); i != temp.end(); ++i)
    {
        set(i.key(), i.value().value());
    }
}

void Properties::serialize(IO::XmlStream & io) const
{
    io.newSection("properties");

        io.write("version", 1);

        for (auto i = p_map_.begin(); i != p_map_.end(); ++i)
        {
            io.newSection("property");

                io.write("id", i.key());

                // named values are stored by id
                if (i.value().hasNamedValues())
                {
                    auto nv = i.value().namedValues().getByValue(
                                i.value().value());
                    io.write("nv", nv.id);
                }

                // for everything else we count on XmlStream to handle the QVariant
                else
                //if (QMetaType::Type(i.value().value().type()) < QMetaType::User)
                    io.write("v", i.value().value());
#if 0
                else
                {
                    MO_IO_WARNING(WRITE, "Properties::serialize() unhandled QVariant '"
                               << i.value().value().typeName() << "'");
                }
#endif
            io.endSection();
        }

    io.endSection();
}

/** @todo move to io/streamoperators_qt.h */
std::ostream& operator << (std::ostream& s, const QVariant& v)
{
    QString val;
    if (v.type() == QVariant::Size)
    {
        auto t = v.toSize();
        val = QString("%1,%2").arg(t.width()).arg(t.height());
    }
    else
        val = v.toString();

    s << "QVariant(" << val << "/" << v.typeName() << ")";
    return s;
}


void Properties::deserialize(IO::XmlStream& io)
{
    Properties tmp;

    io.verifySection("properties");

        const int ver = io.expectInt("version");
        Q_UNUSED(ver);

        while (io.nextSubSection())
        {
            if (io.section() == "property")
            {
                QString id = io.expectString("id");

                // general variant value?
                if (io.hasAttribute("v"))
                {
                    QVariant v = io.expectVariant("v");
                    //MO_DEBUG("read " << id << ", " << v);
                    tmp.set(id, v);
                }

                // named value
                else if (io.hasAttribute("nv"))
                {
                    QString nv = io.expectString("nv");

                    if (!has(id))
                    {
                        MO_IO_WARNING(READ, "Properties::deserialize() '" << id
                                      << "' unknown named-value, ignored");
                        io.leaveSection();
                        continue;
                    }
                    auto p = getProperty(id);
                    if (!p.hasNamedValues())
                    {
                        MO_IO_WARNING(READ, "Properties::deserialize() '" << id
                                      << "' is expected to be a named value but is not, ignored");
                        io.leaveSection();
                        continue;
                    }
                    if (!p.namedValues().has(nv))
                    {
                        MO_IO_WARNING(READ, "Properties::deserialize() '" << id
                                      << "' unknown named-value id '" << nv << "', ignored");
                        io.leaveSection();
                        continue;
                    }
                    tmp.set(id, p.namedValues().get(nv).v);
                }
            }

            io.leaveSection();
        }

    // get all new values
    unify(tmp);
}

const Properties::Property& Properties::getProperty(const QString &id) const
{
    static Property invalid;

    auto i = p_map_.find(id);
    if (i == p_map_.end())
        return invalid;
    else
        return i.value();
}

QVariant Properties::get(const QString &id, const QVariant& def) const
{
    auto i = p_map_.find(id);

    return i == p_map_.end() ? def : i.value().value();
}

QVariant Properties::get(const QString &id) const
{
#define MO__GETTER(mem__, ret__) \
    auto i = mem__.find(id); \
    if (i == mem__.end()) \
        MO_DEBUG_PROP("Properties::get[" #mem__ "](\"" << id << "\") unknown"); \
    if (i == mem__.end()) \
        return ret__;

    MO__GETTER(p_map_, QVariant());
    return i.value().value();
}

QVariant Properties::getDefault(const QString &id) const
{
    MO__GETTER(p_map_, QVariant());
    return i.value().defaultValue();
}

QVariant Properties::getMin(const QString &id) const
{
    MO__GETTER(p_map_, QVariant());
    return i.value().minimum();
}

QVariant Properties::getMax(const QString &id) const
{
    MO__GETTER(p_map_, QVariant());
    return i.value().maximum();
}

QVariant Properties::getStep(const QString &id) const
{
    MO__GETTER(p_map_, QVariant());
    return i.value().step();
}

QString Properties::getName(const QString &id) const
{
    MO__GETTER(p_map_, QString());
    return i.value().name();
}


QString Properties::getTip(const QString &id) const
{
    MO__GETTER(p_map_, QString());
    return i.value().tip();
}

int Properties::getSubType(const QString &id) const
{
    MO__GETTER(p_map_, -1);
    return i.value().subType();
    #undef MO__GETTER
}






void Properties::set(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::set '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");

#define MO__GETPROP \
    auto i = p_map_.find(id); \
    if (i == p_map_.end()) \
    { \
        p_map_.insert(id, Property()); \
        i = p_map_.find(id); \
        i.value().p_idx_ = p_map_.size()-1; \
    } \

    MO__GETPROP
    i.value().p_val_ = v;
}


void Properties::setDefault(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::setDefault '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    MO__GETPROP
    i.value().p_def_ = v;
}

void Properties::setMin(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::setMin '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    MO__GETPROP
    i.value().p_min_ = v;
}

void Properties::setMax(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::setMax '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    MO__GETPROP
    i.value().p_max_ = v;
}

void Properties::setRange(const QString &id, const QVariant & mi, const QVariant & ma)
{
    MO_DEBUG_PROP("Properties::setRange '" << id << "': " << mi << "-" << ma << " type "
                  << mi.typeName() << " (" << mi.type() << ")");
    MO__GETPROP
    i.value().p_min_ = mi;
    i.value().p_max_ = ma;
}

void Properties::setStep(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::setStep '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    MO__GETPROP
    i.value().p_step_ = v;
}


void Properties::setName(const QString &id, const QString& v)
{
    MO_DEBUG_PROP("Properties::setName '" << id << "': " << v << ")");
    MO__GETPROP
    i.value().p_name_ = v;
}

void Properties::setTip(const QString &id, const QString& v)
{
    MO_DEBUG_PROP("Properties::setTip '" << id << "': " << v << ")");
    MO__GETPROP
    i.value().p_tip_ = v;
}


void Properties::setSubType(const QString &id, int v)
{
    MO_DEBUG_PROP("Properties::setSubType '" << id << "': " << v << ")");
    MO__GETPROP
    i.value().p_subType_ = v;
}

void Properties::setNamedValues(const QString &id, const NamedValues &names)
{
    MO_DEBUG_PROP("Properties::setNamedValues '" << id << "'");
    MO__GETPROP
    i.value().p_nv_ = names;
}

void Properties::setVisible(const QString &id, bool vis)
{
    MO_DEBUG_PROP("Properties::setVisible '" << id << "' " << vis);
    auto i = p_map_.find(id);
    if (i != p_map_.end())
        i.value().p_vis_ = vis;
}

void Properties::setWidgetCallback(
        const QString &id, std::function<void (QWidget *)> f)
{
    MO_DEBUG_PROP("Properties::setWidgetCallback '" << id << "'");
    auto i = p_map_.find(id);
    if (i != p_map_.end())
        i.value().p_cb_widget_ = f;
}


bool Properties::change(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("property '" << id << "': " << v << " type "<< v.typeName() << " (" << v.type() << ")");
    auto i = p_map_.find(id);
    if (i == p_map_.end())
        return false;
    i.value().p_val_ = v;
    return true;
}

void Properties::unify(const Properties &other)
{
    for (auto i = other.p_map_.begin(); i != other.p_map_.end(); ++i)
    {
        auto j = p_map_.find(i.key());
        if (j == p_map_.end())
            p_map_.insert(i.key(), i.value());
        else
            j.value().p_val_ = i.value().p_val_;
    }
}

bool Properties::isVisible(const QString &id) const
{
    auto i = p_map_.find(id);
    if (i != p_map_.end())
        return i.value().isVisible();
    return false;
}

QString Properties::toString(const QString &indent) const
{
    QString r;
    for (auto i = begin(); i != end(); ++i)
    {
        /** @todo print correct value for all types */
        r += indent + i.key() + ": " + i.value().value().toString()
                + "; // " + QString::number(i.value().value().type())
                + " " + i.value().value().typeName();
        if (i.value().hasNamedValues())
            r += " (" + QString::number(i.value().namedValues().p_val_.size())
                    + " NamedValues)";
        if (!i.value().isVisible())
            r += " (off)";
        r += "\n";
    }
    return r;
}


bool Properties::callUpdateVisibility()
{
    if (!p_cb_vis_)
        return false;

    // store state
    std::vector<bool> vis;
    for (auto & p : *this)
        vis.push_back(p.isVisible());

    p_cb_vis_(*this);

    // check difference
    auto v = vis.begin();
    for (auto & p : *this)
        if (p.isVisible() != *v++)
            return true;
    return false;
}

void Properties::callWidgetCallback(const QString & id, QWidget * w) const
{
    if (w)
    {
        auto i = p_map_.find(id);
        if (i != p_map_.end() && i.value().p_cb_widget_)
            i.value().p_cb_widget_(w);
    }
}

#if 0
QRectF Properties::align(const QRectF &rect, const QRectF &parent,
                         int alignment, qreal margin, bool outside)
{
    QRectF r(rect);

    // h&v center (ignore outside flag and margin)
    if ((alignment & A_CENTER) == A_CENTER)
    {
        r.moveLeft( parent.left() + (parent.width() - rect.width()) / 2.);
        r.moveTop( parent.top() + (parent.height() - rect.height()) / 2.);
        return r;
    }

    if (outside)
        margin = -margin;

    r.moveTopLeft(parent.topLeft() + QPointF(
                      margin - (outside ? rect.width() : 0),
                      margin - (outside ? rect.height() : 0)));

    // hcenter or right (it's already left)
    if ((alignment & A_HCENTER) == A_HCENTER)
        r.moveLeft( parent.left() + (parent.width() - rect.width()) / 2.);
    else if (alignment & A_RIGHT)
        r.moveRight( parent.right() - margin
                     + (outside ? rect.width() : 0));

    // vcenter or bottom (it's already top)
    if ((alignment & A_VCENTER) == A_VCENTER)
        r.moveTop( parent.top() + (parent.height() - rect.height()) / 2.);
    else if (alignment & A_BOTTOM)
        r.moveBottom( parent.bottom() - margin
                      + (outside ? rect.height() : 0));

    return r;
}
#endif


} // namespace MO
