/** @file helpexporterlatex.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/5/2014</p>
*/

#ifndef MOSRC_IO_HELPEXPORTERLATEX_H
#define MOSRC_IO_HELPEXPORTERLATEX_H

#include <QObject>
#include <QString>

class QDomDocument;
class QDomElement;
class QTextStream;

namespace MO {

class HelpSystem;


class HelpExporterLatex : public QObject
{
    Q_OBJECT
public:
    explicit HelpExporterLatex(QObject *parent = 0);
    ~HelpExporterLatex();

    void save(const QString& directory);

private:

    void scanHtml_(const QString& url, QDomDocument& doc);
    bool isScanned_(const QString& url) const;

    /*
    void prepareHtml_(QDomDocument& doc);
    void writeHtml_(const QString& filename, const QDomDocument& doc);
    void writeString_(const QString& filename, const QString& text);
    void writeImages_(const QString& directory);
    */
    void gatherLinks_(QDomElement & e);
    void getPotentialLink_(const QDomElement& e);
    void gatherImages_(QDomElement & e);

    void exportHtml_(QDomDocument & doc, const QString& url, QString& content);
    void prepareNode_(QDomElement& e);
    void exportNode_(QDomElement& e, QTextStream& stream);
    void exportNodeFragment_(QDomElement& e, QTextStream& stream);

    // returns a latex conformant text
    QString str_(const QString& text);

    /*
    QString getFilenameFor_(const QString& url);
    */
    HelpSystem * help_;

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_IO_HELPEXPORTERLATEX_H
