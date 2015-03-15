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
#include <QImage>

class QDomDocument;
class QDomElement;
class QTextStream;

namespace MO {

class Object;

/** Globally sets the help url for the next call of the context help */
void setHelpUrl(const QString& url);

QString currentHelpUrl();

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

    /** Returns an image for the special equation url
     *  _equ#xstart#xend#ystart#yend#equation[#width[#height]] */
    QImage getEquationImage(const QString& url) const;

    /** Returns the image for the particular object.
        url is "className[#width[#height]]" */
    QImage getObjectImage(const QString& classNameUrl) const;

    /** Returns a runtime generated resource for the given url,
        or an invalid QVariant if the url is not known. */
    QVariant getRuntimeResource(const QString& partial_url);

    /** Returns the html documentation of an object */
    QString getObjectDoc(const Object * o);

private:

    void renderHtml_(const QDomElement&, QTextStream&);
    void renderHtmlImg_(const QDomElement&, QTextStream&);

    void loadEquationFunctions_();
    void addObjectIndex_(QString& doc);
    void addEquationInfo_(QString& doc);
    void addAngelScriptInfo_(QString& doc);

    QStringList searchPaths_;

    struct EquFunc_
    {
        int numParams;
        QString niceDisplay;
        QString help;
    };

    QMultiMap<QString, EquFunc_> funcMap_;
};

} // namespace MO

#endif // MOSRC_IO_HELPSYSTEM_H
