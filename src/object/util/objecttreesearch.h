/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/7/2016</p>
*/

#ifndef MOSRC_OBJECT_UTIL_OBJECTTREESEARCH_H
#define MOSRC_OBJECT_UTIL_OBJECTTREESEARCH_H

#include "object/object.h"

namespace MO {

class ObjectTreeSearch
{
public:

    /** Scans the child tree of @p root, for objects matching
        the regexes in @p rInc and ignoring the regexes in @p rIgn.
        If either one is an empty list, they are ignored.
        Found objects are appended to @p list. */
    template <class ObjectClass>
    static void getObjectsRegex(
            Object * root, QList<ObjectClass*>& list,
            const QList<QRegExp>& rInc, const QList<QRegExp>& rIgn)
    {
        for (Object* o : root->childObjects())
        {
            bool doinc = true;

            // ignore objects
            for (const auto & regx : rIgn)
            if (regx.exactMatch(o->name()))
            {
                doinc = false;
                break;
            }
            if (!doinc)
                continue;

            // check for type
            if (auto omatch = dynamic_cast<ObjectClass*>(o))
            {
                // include objects
                doinc = false;
                for (const auto & regx : rInc)
                if (regx.exactMatch(omatch->name()))
                {
                    doinc = true;
                    break;
                }
                if (doinc)
                    list << omatch;
            }

            // traverse childs
            getObjectsRegex(o, list, rInc, rIgn);
        }
    }


    /** Scans the child tree of @p root, for objects matching
        wildcard strings in @p includeStr and explicitly ignoring
        the wildcards in @p ignoreStr.
        The strings can contain a comma separated list of wildcards.
        Found objects are appended to @p list. */
    template <class ObjectClass>
    static void getObjectsWildcard(
            Object* root, QList<ObjectClass*>& list,
            const QString& includeStr = "*", const QString& ignoreStr = "")
    {
        // shortcut for default strings
        if (includeStr == "*" && ignoreStr.isEmpty())
        {
            list << root->findChildObjects<ObjectGl>(QString(), true);
            return;
        }

        // split by commas
        auto listInc = includeStr.split(
                    QChar(','), QString::SkipEmptyParts, Qt::CaseInsensitive);
        auto listIgn = ignoreStr.split(
                    QChar(','), QString::SkipEmptyParts, Qt::CaseInsensitive);

        // create regexps
        QList<QRegExp> rlistInc, rlistIgn;
        for (const auto & s : listInc)
            rlistInc << QRegExp(s.simplified(),
                                Qt::CaseSensitive, QRegExp::WildcardUnix);
        for (const auto & s : listIgn)
            rlistIgn << QRegExp(s.simplified(),
                                Qt::CaseSensitive, QRegExp::WildcardUnix);

        getObjectsRegex(root, list, rlistInc, rlistIgn);

    }

};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_OBJECTTREESEARCH_H

