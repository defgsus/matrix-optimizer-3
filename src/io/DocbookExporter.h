/** @file docbookexporter.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/26/2015</p>
*/

#ifndef MOSRC_IO_DOCBOOKEXPORTER_H
#define MOSRC_IO_DOCBOOKEXPORTER_H

class QString;
class QDomDocument;

namespace MO {
namespace IO {

class DocBookExporter
{
public:
    DocBookExporter();
    explicit DocBookExporter(const QDomDocument&);
    ~DocBookExporter();

    void setDocument(const QDomDocument& );

    void exportDocBook(const QString& filename);

    static QString tr(const char *);
private:

    struct Private;
    Private * p_;
};


} // namespace IO
} // namespace MO


#endif // MOSRC_IO_DOCBOOKEXPORTER_H
