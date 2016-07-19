/** @file docbookexporter.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/26/2015</p>
*/

#include <QDomDocument>
#include <QTextStream>
#include <QFile>

#include "DocbookExporter.h"
#include "io/version.h"
#include "io/error.h"

namespace MO {
namespace IO {

struct DocBookExporter::Private
{
    QDomDocument doc;
};

DocBookExporter::DocBookExporter()
    : p_        (new Private)
{
}

DocBookExporter::DocBookExporter(const QDomDocument & d)
    : p_        (new Private)
{
    setDocument(d);
}

DocBookExporter::~DocBookExporter()
{
    delete p_;
}

QString DocBookExporter::tr(const char * s) { return QObject::tr(s); }

void DocBookExporter::setDocument(const QDomDocument &d)
{
    p_->doc = d;
}

void DocBookExporter::exportDocBook(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        MO_IO_ERROR(WRITE, "Could not create xml file '" << filename << "'");

    QTextStream str(&file);

    str << "<?xml version='1.0'?>\n"
           "<!DOCTYPE book PUBLIC \"-//OASIS//DTD DocBook V5.0//EN\" "
           "\"http://www.oasis-open.org/docbook/xml/5.0/docbook.dtd\">\n"
           "<!-- auto-created by " << applicationName() << " -->\n";

    str << "<book xmlns=\"http://docbook.org/ns/docbook\" version=\"5.0\">\n";
    str << "<title>" << tr("Matrix Optimizer III") << "</title>\n";
    str << "</book>\n";
}

} // namespace IO
} // namespace MO
