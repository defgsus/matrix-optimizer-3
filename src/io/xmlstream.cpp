/** @file io.cpp

    @brief Serializer and deserializer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/25/2014</p>
*/
#include "io/memory.h"

#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>

#include "xmlstream.h"
#include "log.h"
#include "error.h"

namespace MO {
namespace IO {

XmlStream::XmlStream()
    :   xmlw_   (0),
        xmlr_   (0)
{
    MO_DEBUGF("Io::Io");
}

XmlStream::~XmlStream()
{
    MO_DEBUGF("Io::~Io");

    if (xmlw_) delete xmlw_;
    if (xmlr_) delete xmlr_;
}


// -------------- file io -----------------------

void XmlStream::save(const QString& filename)
{
    MO_DEBUGF("Io::save("<<filename<<")");

    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        MO_IO_ERROR(WRITE, "Io could not save '" << filename << "'\n"
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
        MO_IO_ERROR(READ, "Io could not load '" << filename << "'\n"
                        << f.errorString());
    }

    QTextStream in(&f);
    data_ = in.readAll();

    f.close();
}


// -------------------- io ----------------------

void XmlStream::startWriting()
{
    MO_DEBUGF("Io::startWriting()");

    data_.clear();
    if (xmlw_) delete xmlw_;

    xmlw_ = new QXmlStreamWriter(&data_);
    xmlw_->setAutoFormatting(true);
    xmlw_->writeStartDocument();

    newSection("mo-io");
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

    qDebug() << data_ << "\n";
}

bool XmlStream::isWriteable() { return xmlw_ != 0; }

void XmlStream::startReading()
{
    MO_DEBUGF("Io::startReading()");

    if (xmlr_) delete xmlr_;

    xmlr_ = new QXmlStreamReader(data_);

    if (!(xmlr_->readNextStartElement() &&
          xmlr_->name() == "mo-io"))
        MO_IO_ERROR(VERSION_MISMATCH, "Io::startReading() on unknown stream")
                ;
    cur_section_ = "mo-io";

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




// --------------------------- test -------------------------------
/*

void testIo()
{
    Io io;

    io.startWriting();

    io.newSection("patch");
        io.write("version", "1");
        io.newSection("module");
            io.write("class", "Math");
            io.write("id", "Math1");
            io.write("name", "Mathematik");
            io.newSection("param");
                io.write("id", "num_in");
                io.write("v", 2);
            io.endSection();
        io.endSection();
        io.newSection("connections");
            io.newSection("c");
            io.write("fm", "Math1");
            io.write("fc", "out1");
            io.write("tm", "Math1");
            io.write("tc", "in1");
            io.endSection();
        io.endSection();
    io.endSection();

    io.stopWriting();

    qDebug("-------------");

    io.startReading();

    io.nextSubSection();
    if (io.section() != "patch") { std::cout << "expected patch\n"; exit(-1); }

    std::cout << "version " << io.readInt("version") << std::endl;



    io.stopReading();
}

*/

} // namespace IO
} // namespace MO
