/** @file properties.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#include "properties.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

const uint Properties::AlignmentType = QVariant::nameToType("MO::Properties::Alignment");
QString Properties::alignmentToName(uint a)
{
    if ((a & A_CENTER) == A_CENTER)
        return "center";
    QString s;
    if ((a & A_VCENTER) == A_VCENTER)
        s = "vcenter";
    else
    {
        if (a & A_BOTTOM)
            s = "bottom";
        else
            s = "top";
    }
    if ((a & A_HCENTER) == A_HCENTER)
        s += "-hcenter";
    else
    {
        if (a & A_RIGHT)
            s += "-right";
        else
            s += "-left";
    }

    return s;
}

uint Properties::alignmentFromName(const QString & s)
{
    for (uint i=0; i<16; ++i)
        if (s == alignmentToName(i))
            return i;
    return 0;
}




Properties::Properties()
{
}

void Properties::serialize(IO::DataStream & io) const
{
    io.writeHeader("props", 1);
    io << p_props_;
}

void Properties::deserialize(IO::DataStream & io)
{
    io.readHeader("props", 1);
    io >> p_props_;
}

QVariant Properties::get(const QString &id) const
{
    auto i = p_props_.find(id);

#ifdef MO_DO_DEBUG
    if (i == p_props_.end())
        MO_DEBUG("Properties::get(\"" << id << "\") unknown");
#endif

    return i == p_props_.end() ? QVariant() : i.value();
}

QVariant Properties::get(const QString &id, const QVariant& def) const
{
    auto i = p_props_.find(id);

    return i == p_props_.end() ? def : i.value();
}

void Properties::p_set_(const QString &id, const QVariant & v)
{
    //MO_DEBUG("property '" << id << "': type " << v.typeName() << " (" << v.type() << ")");
    p_props_.insert(id, v);
}

void Properties::merge(const Properties &other)
{
    for (auto i = other.p_props_.begin(); i != other.p_props_.end(); ++i)
        p_props_.insert(i.key(), i.value());
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


QRectF Properties::align(const QRectF &rect, const QRectF &parent, int alignment)
{
    QRectF r(rect);
    // center or right (it's already left)
    if ((alignment & A_HCENTER) == A_HCENTER)
        r.moveLeft( parent.left() + (parent.width() - rect.width()) / 2.);
    else if (alignment & A_RIGHT)
        r.moveRight( parent.right() );

    // vcenter or bottom (it's already top)
    if ((alignment & A_VCENTER) == A_VCENTER)
        r.moveTop( parent.top() + (parent.height() - rect.height()) / 2.);
    else if (alignment & A_BOTTOM)
        r.moveBottom( parent.bottom() );

    return r;
}



} // namespace MO
