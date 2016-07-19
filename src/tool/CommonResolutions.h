/** @file commonresolutions.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.10.2014</p>
*/

#ifndef MOSRC_TOOL_COMMONRESOLUTIONS_H
#define MOSRC_TOOL_COMMONRESOLUTIONS_H

#include <QSize>
#include <QSizeF>
#include <QStringList>

class QMenu;

namespace MO {

class CommonResolutions
{
    CommonResolutions();
public:

    struct Resolution
    {
        const QString name() const { return name_; }
        QString descriptiveName() const;
        const QSize& size() const { return size_; }
        const QSizeF& ratio() const { return ratio_; }

    private:
        friend class CommonResolutions;
        Resolution(const QSize& s, const QSizeF& ratio, const QString& n)
            : size_(s), ratio_(ratio), name_(n) { }
        QSize size_;
        QSizeF ratio_;
        QString name_;
    };

    static const QList<Resolution> resolutions;

    /** Adds an action for each resolution to the given menu.
        Action::data() will contain the index into the resolutions list. */
    static void addResolutionActions(QMenu *, bool checkable = false);

    /** Returns the pointer to the matching Resolution struct, or NULL */
    static const Resolution * findResolution(const QSize&);
private:


};

} // namespace MO

#endif // MOSRC_TOOL_COMMONRESULTIONS_H
