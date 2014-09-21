/** @file io.cpp

    @brief Serializer and deserializer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/25/2014</p>
*/

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
    MO_DEBUGF("Io::Io");
}

XmlStream::~XmlStream()
{
    MO_DEBUGF("Io::~Io");

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
    MO_DEBUGF("Io::save("<<filename<<")");

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
    MO_DEBUGF("Io::load("<<filename<<")");

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
    MO_DEBUGF("Io::startWriting()");

    data_.clear();
    if (xmlw_) delete xmlw_;

    xmlw_ = new QXmlStreamWriter(&data_);
    xmlw_->setAutoFormatting(true);
    xmlw_->writeStartDocument();

    newSection(main_section);
}

void XmlStream::stopWriting()
{
    MO_DEBUGF("Io::stopWriting()");

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
    MO_DEBUGF("Io::startReading()");

    if (xmlr_) delete xmlr_;

    xmlr_ = new QXmlStreamReader(data_);

    if (!(xmlr_->readNextStartElement() &&
          xmlr_->name() == main_section))
        MO_IO_ERROR(VERSION_MISMATCH, "Io::startReading() on unknown xml stream, "
                    "expected '" << main_section << "' section");

    cur_section_ = main_section;

    section_stack_.clear();
}

void XmlStream::stopReading()
{
    MO_DEBUGF("Io::stopReading()");

    if (!xmlr_) return;

    delete xmlr_;
    xmlr_ = 0;
}

bool XmlStream::isReadable() { return xmlr_ != 0; }


// -------------- verification ------------------

void XmlStream::verifySection(const QString &name)
{
    if (!isSection(name))
        MO_IO_ERROR(VERSION_MISMATCH, "expected section '" << name << "' in xml but "
                    "found " << section());
}



// ------------------ sections ------------------

void XmlStream::newSection(const QString& name)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::newSection('"<<name<<"') on unitialized stream");

    xmlw_->writeStartElement(name);

    section_stack_.push_back(cur_section_);
    cur_section_ = name;
}


void XmlStream::endSection()
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::endSection() on unitialized stream");

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
        MO_IO_ERROR(READ, "Io::nextSubSection() on non-readable stream");

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
        MO_IO_ERROR(READ, "Io::leaveSection() on non-readable stream");

    if (!xmlr_->isEndElement())
        xmlr_->skipCurrentElement();

    cur_section_ = xmlr_->name().toString();

}

// ----------------- data write -----------------

void XmlStream::write(const QString& key, const QString& v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, v);
}

void XmlStream::write(const QString& key, bool v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, int v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, unsigned int v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, long int v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, long unsigned int v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, float v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, double v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, QString::number(v));
}

void XmlStream::write(const QString& key, const QDateTime& v)
{
    if (!xmlw_)
        MO_IO_ERROR(WRITE, "Io::write('"<<key<<"') on non-writeable stream");

    xmlw_->writeAttribute(key, v.toString(datetime_format));
}


// ----------------- data read ------------------

#define MO_IO_CHECK_ATTRIBUTE(key__, value__, default__) \
    if (!xmlr_) \
        MO_IO_ERROR(READ, "Io::read('"<<key<<"') on non-readable stream"); \
    if (!xmlr_->attributes().hasAttribute(key__)) \
    { \
        value__ = default__; \
        return false; \
    } \
    const auto a = xmlr_->attributes().value(key__);

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



#undef MO_IO_CHECK_ATTRIBUTE


// ----------------- expect read ------------------


#define MO_IO_CHECK_ATTRIBUTE(key__, value__) \
    if (!xmlr_)                                                                         \
        MO_IO_ERROR(READ, "Io::read('"<<key<<"') on non-readable stream");              \
    if (!xmlr_->attributes().hasAttribute(key__))                                       \
    {                                                                                   \
        MO_IO_ERROR(VERSION_MISMATCH, "Expected key '" << key__ << "' in xml, section " \
                    "'" << section() << "'");                                           \
    }                                                                                   \
    const auto a = xmlr_->attributes().value(key__);

void XmlStream::expect(const QString& key, QString& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v);
    v = a.toString();
}

void XmlStream::expect(const QString& key, bool& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v);
    v = a.toInt();
}

void XmlStream::expect(const QString& key, int& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v);
    v = a.toInt();
}

void XmlStream::expect(const QString& key, unsigned int& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v);
    v = a.toUInt();
}

void XmlStream::expect(const QString& key, long int& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v);
    v = a.toInt();
}

void XmlStream::expect(const QString& key, long unsigned int& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v);
    v = a.toUInt();
}

void XmlStream::expect(const QString& key, float& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v);
    v = a.toFloat();
}

void XmlStream::expect(const QString& key, double& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v);
    v = a.toDouble();
}


void XmlStream::expect(const QString& key, QDateTime& v) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v);
    v = QDateTime::fromString(a.toString(), datetime_format);
}


#undef MO_IO_CHECK_ATTRIBUTE


} // namespace IO
} // namespace MO
