/** @file helpsystem.h

    @brief Pseudo-xhtml parser for resource help texts

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/3/2014</p>
*/

#ifndef MOSRC_IO_HELPSYSTEM_H
#define MOSRC_IO_HELPSYSTEM_H

#include <QObject>
#include <QVariant>
#include <QStringList>

class QDomDocument;
class QDomElement;
class QTextStream;

namespace MO {

class HelpSystem : public QObject
{
    Q_OBJECT
public:

    enum ResourceType
    {
        // these match QTextDocument::ResourceType
        HtmlResource = 1,
        ImageResource = 2,
        StyleSheetResource = 3
    };

    explicit HelpSystem(QObject *parent = 0);

signals:

public slots:

    /** Tries to load the resource. */
    QVariant loadResource(const QString& partial_url, ResourceType type);

    /** Loads the pseudo-html from the given url and creates a dom document from it. */
    bool loadXhtml(const QString &partial_url, QDomDocument& doc);

    /** Will prefix @p partial_url with the search path where
        the resource can be found.
        If it's not found, an empty QString is returned. */
    QString findResource(const QString& partial_url, ResourceType type);

    void renderHtml(const QDomDocument&, QTextStream&);
private:

    void renderHtml_(const QDomElement&, QTextStream&);
    void renderHtmlImg_(const QDomElement&, QTextStream&);

    QStringList searchPaths_;
};

} // namespace MO

#endif // MOSRC_IO_HELPSYSTEM_H
