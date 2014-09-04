/** @file helpexporterhtml.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/3/2014</p>
*/

#ifndef MOSRC_IO_HELPEXPORTERHTML_H
#define MOSRC_IO_HELPEXPORTERHTML_H

#include <QObject>
#include <QString>
#include <QMap>

class QDomDocument;
class QDomElement;

namespace MO {

class HelpSystem;


class HelpExporterHtml : public QObject
{
    Q_OBJECT
public:
    explicit HelpExporterHtml(QObject *parent = 0);

    void save(const QString& directory);

private:

    void prepareHtml_(QDomDocument& doc);
    void writeHtml_(const QString& filename, const QDomDocument& doc);
    void writeString_(const QString& filename, const QString& text);
    void writeImages_(const QString& directory);

    void gatherLinks_(QDomElement & e);
    void getPotentialLink_(const QDomElement& e);
    void gatherImages_(QDomElement & e);

    QString getFilenameFor_(const QString& url);

    HelpSystem * help_;

    QList<QString> htmls_, imgs_;
    QMap<QString, bool> htmlsExp_, imgsExp_;
    QMap<QString, QString> imageNames_;
    int imageCounter_;
    QString imageExtension_;
};

} // namespace MO

#endif // MOSRC_IO_HELPEXPORTERHTML_H
