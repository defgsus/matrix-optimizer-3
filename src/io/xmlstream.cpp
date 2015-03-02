/** @file xmlstream.cpp

    @brief Serializer and deserializer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/25/2014</p>
*/

#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTextStream>
#include <QFile>

#include "xmlstream.h"
#include "log.h"
#include "error.h"

namespace MO {
namespace IO {

namespace
{
    const QString datetime_format = "yyyy.MM.dd-hh:mm:ss.zzz";
}


XmlStream::XmlStream()
    :   xmlw_   (0),
        xmlr_   (0)
{
    MO_DEBUGF("XmlStream::Io");
}

XmlStream::~XmlStream()
{
    MO_DEBUGF("XmlStream::~Io");

    if (xmlw_)
    {
        MO_IO_WARNING(WRITE, "Destructor of XmlStream without XmlStream::stopWriting()");
        delete xmlw_;
    }
    if (xmlr_) delete xmlr_;
}


// -------------- file io -----------------------

void XmlStream::save(const QString& filename)
{
    MO_DEBUGF("XmlStream::save("<<filename<<")");

    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        MO_IO_ERROR(WRITE, "Could not create xml '" << filename << "'\n"
                        << f.errorString());
    }

    QTextStream out(&f);
    out << data_;

    f.close();
}

void XmlStream::load(const QString& filename)
{
    MO_DEBUGF("XmlStream::load("<<filename<<")");

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        MO_IO_ERROR(READ, "Could not open xml '" << filename << "'\n"
                        << f.errorString());
    }

    QTextStream in(&f);
    data_ = in.readAll();

    f.close();
}


// -------------------- io ----------------------

void XmlStream::startWriting(const QString & main_section)
{
    MO_DEBUGF("XmlStream::startWriting()");

    data_.clear();
    if (xmlw_) delete xmlw_;

    xmlw_ = new QXmlStreamWriter(&data_);
    xmlw_->setAutoFormatting(true);
    xmlw_->writeStartDocument();

    newSection(main_section);
}

void XmlStream::stopWriting()
{
    MO_DEBUGF("XmlStream::stopWriting()");

    if (!xmlw_) return;

    xmlw_->writeEndElement();
    xmlw_->writeEndDocument();

    delete xmlw_;
    xmlw_ = 0;

    cur_section_ = "";
}

bool XmlStream::isWriteable() { return xmlw_ != 0; }

void XmlStream::startReading(const QString & main_section)
{
    MO_DEBUGF("XmlStream::startReading()");

    if (xmlr_) delete xmlr_;

    xmlr_ = new QXmlStreamReader(data_);

    if (!(xmlr_->readNextStartElement() &&
          xmlr_->name() == main_section))
        MO_IO_ERROR(VERSION_MISMATCH, "XmlStream::startReading() on unknown xml stream, "
                    "expected '" << main_section << "' section");

    cur_section_ = main_section;

    section_stack_.clear();
}

void XmlStream::stopReading()
{
    MO_DEBUGF("XmlStream::stopReading()");

    if (!xmlr_) return;

    delete xmlr_;
    xmlr_ = 0;
}

bool XmlStream::isReadable() { return xmlr_ != 0; }


// -------------- verification ------------------

void XmlStream::verifySection(const QString &name) const
{
    if (!isSection(name))
        MO_IO_ERROR(VERSION_MISMATCH, "expected section '" << name << "' in xml but "
                    "found " << section());
}

bool XmlStream::hasAttribute(const QString &name) const
{
    if (!xmlr_)
        return false;
    return xmlr_->attributes().hasAttribute(name);
}


// ------------------ sections ------------------

void XmlStream::newSection(const QString& name)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::newSection('"<<name<<"') on unitialized stream");

    xmlw_->writeStartElement(name);

    section_stack_.push_back(cur_section_);
    cur_section_ = name;
}


void XmlStream::endSection()
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::endSection() on unitialized stream");

    xmlw_->writeEndElement();

    // get previous section
    if (!section_stack_.empty())
    {
        cur_section_ = section_stack_.back();
        section_stack_.pop_back();
    }
        else cur_section_ = "";
}

bool XmlStream::isSection(const QString& name) const
{
    return (name == cur_section_);
}

const QString& XmlStream::section() const
{
    return cur_section_;
}

bool XmlStream::nextSubSection()
{
    if (!xmlr_)
        MO_IO_ERROR(READ, "XmlStream::nextSubSection() on non-readable stream");

    cur_section_ = "";
    //qDebug() << ":: " << xmlr_->name().toString() << "\n";

    if (!xmlr_->readNextStartElement())
        return false;

    cur_section_ = xmlr_->name().toString();
    return true;
}

void XmlStream::leaveSection()
{
    if (!xmlr_)
        MO_IO_ERROR(READ, "XmlStream::leaveSection() on non-readable stream");

    if (!xmlr_->isEndElement())
        xmlr_->skipCurrentElement();

    cur_section_ = xmlr_->name().toString();

}

// ----------------- data write -----------------

void XmlStream::write(const QString& key, const QString& v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, v);
}

void XmlStream::write(const QString& key, bool v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, int v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, unsigned int v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, long int v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, long unsigned int v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, float v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, double v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, const QDateTime& v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    //qDebug() << "writing " << v << " " << v.toString(datetime_format);

    xmlw_->writeAttribute(key, v.toString(datetime_format));
}

void XmlStream::write(const QString& key, const QVariant& v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "XmlStream::write('"<<key<<"') on non-writeable stream");

    QString vs;
    // handle compound types
    if (v.type() == QVariant::Color)
    {
        // Note: QColor -> QVariant -> QString ignores the alpha value
        // hence the selfmade conversion..
        const auto c = v.value<QColor>();
        vs = QString("%1").arg(c.rgba(), 8, 16);
    }
    else
    if (v.type() == QVariant::Size)
    {
        const auto val = v.toSize();
        vs = QString("%1,%2").arg(val.width()).arg(val.height());
    }
    else
    if (v.type() == QVariant::SizeF)
    {
        const auto val = v.toSizeF();
        vs = QString("%1,%2").arg(val.width()).arg(val.height());
    }
    else
    if (v.type() == QVariant::Point)
    {
        const auto val = v.toPoint();
        vs = QString("%1,%2").arg(val.x()).arg(val.y());
    }
    else
        vs = v.toString();

    if (vs.isEmpty() && v.type() != QVariant::String)
        MO_IO_WARNING(LOGIC, "XmlStream::write(" << key
                      << "): unsupported variant type " << v.typeName());

    xmlw_->writeAttribute(key, QString("%1#%2").arg(v.typeName()).arg(vs));
}

// ----------------- data read ------------------

#define MO_IO_CHECK_ATTRIBUTE(key__, value__, default__) \
    if (!xmlr_) \
        MO_IO_ERROR(READ, "XmlStream::read('"<<key<<"') on non-readable stream"); \
    if (!xmlr_->attributes().hasAttribute(key__)) \
    { \
        value__ = default__; \
        return false; \
    } \
    const QStringRef a = xmlr_->attributes().value(key__);

bool XmlStream::read(const QString& key, QString& v, const QString& def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toString();
    return true;
}

bool XmlStream::read(const QString& key, bool& v, bool def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toInt();
    return true;
}

bool XmlStream::read(const QString& key, int& v, int def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toInt();
    return true;
}

bool XmlStream::read(const QString& key, unsigned int& v, unsigned int def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toUInt();
    return true;
}

bool XmlStream::read(const QString& key, long int& v, long int def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toInt();
    return true;
}

bool XmlStream::read(const QString& key, long unsigned int& v, long unsigned int def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toUInt();
    return true;
}

bool XmlStream::read(const QString& key, float& v, float def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toFloat();
    return true;
}

bool XmlStream::read(const QString& key, double& v, double def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toDouble();
    return true;
}


bool XmlStream::read(const QString& key, QDateTime& v, const QDateTime& def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = QDateTime::fromString(a.toString(), datetime_format);
    return true;
}

bool XmlStream::read(const QString& key, QVariant& v, const QVariant& def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);

    // extract type info
    int idx = a.indexOf("#");
    if (idx <= 0) { v = def; return false; }

    QStringRef typeName = a.left(idx);
    QStringRef value = a.right(idx+1);

    int typeId = QVariant::nameToType(typeName.toLatin1().constData());
    if (typeId == QVariant::Invalid) { v = def; return false; }

    // handle compound types
    if (typeId == QVariant::Color)
    {
        if (value.size() < 8) { v = def; return false; }
        v = QColor(value.mid(2,2).toInt(0,16),
                   value.mid(4,2).toInt(0,16),
                   value.mid(6,2).toInt(0,16),
                   value.mid(0,2).toInt(0,16));
    }
    else
    if (typeId == QVariant::Size)
    {
        auto list = value.toString().split(",");
        if (list.size() < 2) { v = def; return false; }
        v = QSize(list[0].toInt(), list[1].toInt());
    }
    else
    if (typeId == QVariant::SizeF)
    {
        auto list = value.toString().split(",");
        if (list.size() < 2) { v = def; return false; }
        v = QSizeF(list[0].toDouble(), list[1].toDouble());
    }
    else
    if (typeId == QVariant::Point)
    {
        auto list = value.toString().split(",");
        if (list.size() < 2) { v = def; return false; }
        v = QPoint(list[0].toInt(), list[1].toInt());
    }
    else
    {
        QVariant var;
        var = value.toString();
        if (!var.convert(typeId)) { v = def; return false; }
        v = var;
    }

    return true;
}


#undef MO_IO_CHECK_ATTRIBUTE


// ----------------- expect read ------------------


#define MO_IO_CHECK_ATTRIBUTE(key__) \
    if (!xmlr_)                                                                         \
        MO_IO_ERROR(READ, "XmlStream::read('"<<key__<<"') on non-readable stream");     \
    if (!xmlr_->attributes().hasAttribute(key__))                                       \
    {                                                                                   \
        MO_IO_ERROR(VERSION_MISMATCH, "Expected key '" << key__ << "' in xml, section " \
                    "'" << section() << "'");                                           \
    }                                                                                   \
    const auto a = xmlr_->attributes().value(key__);

void XmlStream::expect(const QString& key, QString& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);
    v = a.toString();
}

void XmlStream::expect(const QString& key, bool& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);
    v = a.toInt();
}

void XmlStream::expect(const QString& key, int& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);
    v = a.toInt();
}

void XmlStream::expect(const QString& key, unsigned int& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);
    v = a.toUInt();
}

void XmlStream::expect(const QString& key, long int& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);
    v = a.toInt();
}

void XmlStream::expect(const QString& key, long unsigned int& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);
    v = a.toUInt();
}

void XmlStream::expect(const QString& key, float& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);
    v = a.toFloat();
}

void XmlStream::expect(const QString& key, double& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);
    v = a.toDouble();
}


void XmlStream::expect(const QString& key, QDateTime& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);
    v = QDateTime::fromString(a.toString(), datetime_format);
}

void XmlStream::expect(const QString& key, QVariant& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key);

    v = readVariant(key);
}


#undef MO_IO_CHECK_ATTRIBUTE


} // namespace IO
} // namespace MO
