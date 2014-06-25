/** @file io.cpp

    @brief Serializer and deserializer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/25/2014</p>
*/

#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>

#include "io.h"
#include "log.h"
#include "error.h"

namespace MO {


Io::Io()
    :   xmlw_   (0),
        xmlr_   (0)
{
    MO_DEBUGF("Io::Io");
}

Io::~Io()
{
    MO_DEBUGF("Io::~Io");

    if (xmlw_) delete xmlw_;
    if (xmlr_) delete xmlr_;
}


// -------------- file io -----------------------

bool Io::save(const QString& filename)
{
    MO_DEBUGF("Io::save("<<filename<<")");

    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        MO_IO_ERROR("could not save '" << filename << "'");
        return false;
    }

    QTextStream out(&f);
    out << data_;

    f.close();
    return true;
}

bool Io::load(const QString& filename)
{
    MO_DEBUGF("Io::load("<<filename<<")");

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        MO_IO_ERROR("could not load '" << filename << "'");
        return false;
    }

    QTextStream in(&f);
    data_ = in.readAll();

    f.close();
    return true;
}


// -------------------- io ----------------------

bool Io::startWriting()
{
    MO_DEBUGF("Io::startWriting()");

    data_.clear();
    if (xmlw_) delete xmlw_;
    xmlw_ = new QXmlStreamWriter(&data_);
    xmlw_->setAutoFormatting(true);
    xmlw_->writeStartDocument();
    return newSection("csmod-io");
}

bool Io::stopWriting()
{
    MO_DEBUGF("Io::stopWriting()");

    if (!xmlw_) return false;

    xmlw_->writeEndElement();
    xmlw_->writeEndDocument();

    delete xmlw_;
    xmlw_ = 0;

    cur_section_ = "";

    qDebug() << data_ << "\n";
    return true;
}

bool Io::writeable() { return xmlw_; }

bool Io::startReading()
{
    MO_DEBUGF("Io::startReading()");

    if (xmlr_) delete xmlr_;
    xmlr_ = new QXmlStreamReader(data_);
    if (!(xmlr_->readNextStartElement() &&
          xmlr_->name() == "csmod-io"))
        return false;
    cur_section_ = "csmod-io";
    section_stack_.clear();
    return true;
}

bool Io::stopReading()
{
    MO_DEBUGF("Io::stopReading()");

    if (!xmlr_) return false;
    delete xmlr_;
    xmlr_ = 0;
    return true;
}

bool Io::readable() { return xmlr_; }

// ------------------ sections ------------------

bool Io::newSection(const QString& name)
{
    if (!xmlw_) return false;
    xmlw_->writeStartElement(name);
    section_stack_.push_back(cur_section_);
    cur_section_ = name;
    return true;
}

bool Io::endSection()
{
    if (!xmlw_) return false;
    xmlw_->writeEndElement();
    // get previous section
    if (!section_stack_.empty())
    {
        cur_section_ = section_stack_.back();
        section_stack_.pop_back();
    }
    else cur_section_ = "";
    return true;
}

bool Io::isSection(const QString& name) const
{
    return (name == cur_section_);
}

const QString& Io::section() const
{
    return cur_section_;
}

bool Io::nextSubSection()
{
    if (!xmlr_) return false;
    cur_section_ = "";
    //qDebug() << ":: " << xmlr_->name().toQString() << "\n";
    if (!xmlr_->readNextStartElement()) return false;
    cur_section_ = xmlr_->name().toString();
    return true;
}

bool Io::leaveSection()
{
    if (!xmlr_->isEndElement())
        xmlr_->skipCurrentElement();
    cur_section_ = xmlr_->name().toString();
    return true;
}

// ----------------- data write -----------------

bool Io::write(const QString& key, const QString& v)
{
    if (!xmlw_) return false;
    xmlw_->writeAttribute(key, v);
    return true;
}

bool Io::write(const QString& key, bool v)
{
    if (!xmlw_) return false;
    xmlw_->writeAttribute(key, QString::number(v));
    return true;
}

bool Io::write(const QString& key, int v)
{
    if (!xmlw_) return false;
    xmlw_->writeAttribute(key, QString::number(v));
    return true;
}

bool Io::write(const QString& key, unsigned int v)
{
    if (!xmlw_) return false;
    xmlw_->writeAttribute(key, QString::number(v));
    return true;
}

bool Io::write(const QString& key, long int v)
{
    if (!xmlw_) return false;
    xmlw_->writeAttribute(key, QString::number(v));
    return true;
}

bool Io::write(const QString& key, long unsigned int v)
{
    if (!xmlw_) return false;
    xmlw_->writeAttribute(key, QString::number(v));
    return true;
}

bool Io::write(const QString& key, float v)
{
    if (!xmlw_) return false;
    xmlw_->writeAttribute(key, QString::number(v));
    return true;
}

bool Io::write(const QString& key, double v)
{
    if (!xmlw_) return false;
    xmlw_->writeAttribute(key, QString::number(v));
    return true;
}

// ----------------- data read ------------------

#define MO_IO_CHECK_ATTRIBUTE(key__, value__, default__) \
    if ((!xmlr_) || \
        (!xmlr_->attributes().hasAttribute(key__))) \
    { \
        value__ = default__; \
        return false; \
    } \
    const auto a = xmlr_->attributes().value(key__);

bool Io::read(const QString& key, QString& v, const QString& def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toString();
    return true;
}

bool Io::read(const QString& key, bool& v, bool def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toInt();
    return true;
}

bool Io::read(const QString& key, int& v, int def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toInt();
    return true;
}

bool Io::read(const QString& key, unsigned int& v, unsigned int def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toUInt();
    return true;
}

bool Io::read(const QString& key, long int& v, long int def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toInt();
    return true;
}

bool Io::read(const QString& key, long unsigned int& v, long unsigned int def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toUInt();
    return true;
}

bool Io::read(const QString& key, float& v, float def) const
{
    MO_IO_CHECK_ATTRIBUTE(key, v, def);
    v = a.toFloat();
    return true;
}

bool Io::read(const QString& key, double& v, double def) const
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

} // namespace MO
