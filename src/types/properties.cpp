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

Properties::NamedStates::NamedStates(const QString& name, const QList<NamedStateHelper> &tuples)
    : p_name_   (name)
{
    for (auto & s : tuples)
        p_sv_.insert(s.id, s.v);
}

Properties::NamedStates::NamedStates(const QString& name, const QList<QString> &ids, const QList<QVariant> values)
    : p_name_   (name)
{
    int n = std::min(ids.size(), values.size());
    for (int i=0; i<n; ++i)
        p_sv_.insert(ids[i], values[i]);
}

const QVariant& Properties::NamedStates::value(const QString& id) const
{
    auto i = p_sv_.find(id);
    if (i != p_sv_.end())
        return i.value();

    static QVariant invalid;
    return invalid;
}

const QString& Properties::NamedStates::id(const QVariant& value) const
{
    for (auto i = p_sv_.begin(); i != p_sv_.end(); ++i)
        if (value == i.value())
            return i.key();

    static QString invalid;
    return invalid;
}


bool Properties::isNamedStates(const QVariant & v)
{
    // [add new NamedStates here]
    return isAlignment(v);
}

const Properties::NamedStates* Properties::getNamedStates(const QVariant & v)
{
    if (QMetaType::Type(v.type()) < QMetaType::User)
        return 0;

    // [add new NamedStates here]
    if (isAlignment(v))
        return &alignmentStates;

    return 0;
}

const Properties::NamedStates* Properties::getNamedStates(const QString &name)
{
    // lazy-initialized list of all known states
    static QMap<QString, const NamedStates*> map;
    if (map.isEmpty())
    {
        // [add new NamedStates here]
        map.insert(alignmentStates.name(), &alignmentStates);
    }

    auto i = map.find(name);
    return i == map.end() ? 0 : i.value();
}

// [add new NamedStates here]

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






// ---------------------------- Properties ------------------------------------

Properties::Properties()
{
}

void Properties::swap(Properties &other)
{
    if (&other == this)
        return;

    p_val_.swap(other.p_val_);
    p_def_.swap(other.p_def_);
    p_min_.swap(other.p_min_);
    p_max_.swap(other.p_max_);
    p_step_.swap(other.p_step_);
    p_name_.swap(other.p_name_);
    p_tip_.swap(other.p_tip_);
}

void Properties::clear()
{
    p_val_.clear();
    p_def_.clear();
    p_min_.clear();
    p_max_.clear();
    p_step_.clear();
    p_name_.clear();
    p_tip_.clear();
}

void Properties::clear(const QString &id)
{
    p_val_.remove(id);
    p_def_.remove(id);
    p_min_.remove(id);
    p_max_.remove(id);
    p_step_.remove(id);
    p_name_.remove(id);
    p_tip_.remove(id);
}

void Properties::serialize(IO::DataStream & io) const
{
    io.writeHeader("props", 2);

    // Note: Reimplementation of QDataStream << QMap<QString,QVariant>
    // because overloading operator << does not catch enums
    io << quint64(p_val_.size());
    for (auto i = p_val_.begin(); i != p_val_.end(); ++i)
    {
        io << i.key();

        // non-user types use the QVariant version
        if (QMetaType::Type(i.value().type()) < QMetaType::User)
            io << qint8(0) << i.value();
        else
        {
            // NamedStates
            if (auto ns = getNamedStates(i.value()))
            {
                io << qint8(1)
                // store state group
                   << ns->name()
                // store value id string
                   << ns->id(i.value());
            }
            else
            {
                io << qint8(-1);
                MO_WARNING("Properties::serialize() unhandled QVariant '"
                           << i.value().typeName() << "'");
            }
        }
    }
}

void Properties::deserialize(IO::DataStream & io)
{
    const auto ver = io.readHeader("props", 2);

    if (ver < 2) /* there was ever only one scene file really.. */
    {
        Map dummy;
        io >> dummy;
        MO_WARNING("Properties::deserialize() ignoring version 1 ");
        return;
    }

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
            MO_WARNING("Properties::deserialize() can't restore "
                       "unhandled QVariant for '" << key << "'");
            continue;
        }
        // default QVariant
        if (type == 0)
            io >> v;
        // NamedState
        else if (type == 1)
        {
            QString name, state;
            io >> name >> state;

            auto ns = getNamedStates(name);
            if (!ns)
            {
                MO_WARNING("Properties::deserialize() ignoring unknown NamedStates '"
                           << name << "' (with state '" << state << "')");
                continue;
            }
            v = ns->value(state);
        }
        // unknown
        else
        {
            MO_WARNING("Properties::deserialize() ignoring unknown usertype " << type
                       << " for item '" << key << "'");
            continue;
        }

        temp.insert(key, v);
    }

    // finally assign
    p_val_.swap(temp);
}

void Properties::serialize(IO::XmlStream & io) const
{
    io.newSection("properties");

        io.write("version", 1);

        for (auto i = p_val_.begin(); i != p_val_.end(); ++i)
        {
            io.newSection("property");

                io.write("id", i.key());

                // non-user types use the default QVariant version
                if (QMetaType::Type(i.value().type()) < QMetaType::User)
                    io.write("v", i.value());
                else
                {
                    // NamedStates
                    if (auto ns = getNamedStates(i.value()))
                    {
                        // store state group
                        io.write("ns", ns->name());
                        // store value id string
                        io.write("nv", ns->id(i.value()));
                    }
                    else
                    {
                        MO_WARNING("Properties::serialize() unhandled QVariant '"
                                   << i.value().typeName() << "'");
                    }
                }

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
                // NamedStates?
                else if (io.hasAttribute("nv"))
                {
                    QString name = io.expectString("ns"),
                            nv = io.expectString("nv");
                    //MO_DEBUG("readns " << id << ", " << ns << ", " << nv);
                    auto ns = getNamedStates(name);
                    if (!ns)
                    {
                        MO_WARNING("Properties::deserialize() ignoring unknown NamedStates '"
                                   << name << "' (with state '" << nv << "')");
                    }
                    else
                    {
                        tmp.set(id, ns->value(nv));
                    }
                }
            }

            io.leaveSection();
        }

    // get all new values
    unify(tmp);
}



QVariant Properties::get(const QString &id, const QVariant& def) const
{
    auto i = p_val_.find(id);

    return i == p_val_.end() ? def : i.value();
}

QVariant Properties::get(const QString &id) const
{
#define MO__GETTER(mem__, ret__) \
    auto i = mem__.find(id); \
    if (i == mem__.end()) \
        MO_DEBUG_PROP("Properties::get[" #mem__ "](\"" << id << "\") unknown"); \
    return i == mem__.end() ? ret__() : i.value();

    MO__GETTER(p_val_, QVariant);
}

QVariant Properties::getDefault(const QString &id) const
{
    MO__GETTER(p_val_, QVariant);
}

QVariant Properties::getMin(const QString &id) const
{
    MO__GETTER(p_min_, QVariant);
}

QVariant Properties::getMax(const QString &id) const
{
    MO__GETTER(p_max_, QVariant);
}

QVariant Properties::getStep(const QString &id) const
{
    MO__GETTER(p_step_, QVariant);
}

QString Properties::getName(const QString &id) const
{
    MO__GETTER(p_name_, QString);
}


QString Properties::getTip(const QString &id) const
{
    MO__GETTER(p_tip_, QString);
    #undef MO__GETTER
}

void Properties::set(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::set '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    p_val_.insert(id, v);
}

void Properties::setDefault(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::setDefault '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    p_def_.insert(id, v);
}

void Properties::setMin(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::setMin '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    p_min_.insert(id, v);
}

void Properties::setMax(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::setMax '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    p_max_.insert(id, v);
}

void Properties::setRange(const QString &id, const QVariant & mi, const QVariant & ma)
{
    MO_DEBUG_PROP("Properties::setRange '" << id << "': " << mi << "-" << ma << " type "
                  << v.typeName() << " (" << v.type() << ")");
    p_min_.insert(id, mi);
    p_max_.insert(id, ma);
}

void Properties::setStep(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("Properties::setStep '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    p_step_.insert(id, v);
}


void Properties::setName(const QString &id, const QString& v)
{
    MO_DEBUG_PROP("Properties::setName '" << id << "': " << v << ")");
    p_name_.insert(id, v);
}

void Properties::setTip(const QString &id, const QString& v)
{
    MO_DEBUG_PROP("Properties::setTip '" << id << "': " << v << ")");
    p_tip_.insert(id, v);
}


bool Properties::change(const QString &id, const QVariant & v)
{
    MO_DEBUG_PROP("property '" << id << "': " << v << " type "<< v.typeName() << " (" << v.type() << ")");
    auto i = p_val_.find(id);
    if (i == p_val_.end())
        return false;
    i.value() = v;
    return true;
}

void Properties::unify(const Properties &other)
{
    for (auto i = other.p_val_.begin(); i != other.p_val_.end(); ++i)
    {
        p_val_.insert(i.key(), i.value());
        auto j = other.p_def_.find(i.key());
        if (j != other.p_def_.end())
            p_def_.insert(i.key(), j.value());
        j = other.p_min_.find(i.key());
        if (j != other.p_min_.end())
            p_min_.insert(i.key(), j.value());
        j = other.p_max_.find(i.key());
        if (j != other.p_max_.end())
            p_max_.insert(i.key(), j.value());
        j = other.p_step_.find(i.key());
        if (j != other.p_step_.end())
            p_step_.insert(i.key(), j.value());
        auto k = other.p_name_.find(i.key());
        if (k != other.p_name_.end())
            p_name_.insert(i.key(), k.value());
        k = other.p_tip_.find(i.key());
        if (k != other.p_tip_.end())
            p_tip_.insert(i.key(), k.value());
    }
}



QString Properties::toString(const QString &indent) const
{
    QString r;
    for (auto i = begin(); i != end(); ++i)
    {
        /** @todo print correct value for all types */
        r += indent + i.key() + ": " + i.value().toString()
                + "; // " + QString::number(i.value().type()) + " " + i.value().typeName()
                + "\n";
    }
    return r;
}


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



} // namespace MO
