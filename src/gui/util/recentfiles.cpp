/** @file recentfiles.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/26/2015</p>
*/

#include <QFileInfo>
#include <QMenu>


#include "recentfiles.h"
#include "io/settings.h"
#include "io/error.h"

namespace MO {
namespace GUI {

RecentFiles::RecentFiles(size_t maxFiles, QObject *parent)
    : QObject       (parent)
    , p_max_        (maxFiles)
    , p_menu_       (0)
    , p_autosave_   (false)
{
}

RecentFiles::~RecentFiles()
{
    delete p_menu_;
}

QList<QString> RecentFiles::filenamesStripped() const
{
    QList<QString> r;
    for (auto & s : p_files_)
        r << QFileInfo(s).fileName();

    return r;
}

void RecentFiles::addFilename(const QString & fn)
{
    // find existing?
    for (auto i = p_files_.begin(); i != p_files_.end(); ++i)
    {
        if (*i == fn)
        {
            if (i == p_files_.begin())
                return;
            // remove when not top
            p_files_.erase(i);
            break;
        }
    }

    p_files_.prepend(fn);

    // schmeiss weg die überflüssigen
    while (p_files_.size() > p_max_)
        p_files_.pop_back();

    if (p_autosave_)
        saveSettings();

    p_updateMenu_();
}


QMenu * RecentFiles::createMenu()
{
    if (p_menu_)
        return p_menu_;

    p_menu_ = new QMenu();
    p_menu_->setTitle(tr("Recent files ..."));
    p_updateMenu_();

    return p_menu_;
}

void RecentFiles::p_updateMenu_()
{
    if (!p_menu_)
        return;

    p_menu_->clear();

    for (auto & fn : p_files_)
    {
        auto a = p_menu_->addAction( QFileInfo(fn).fileName() );
        a->setData(fn);
    }
}


void RecentFiles::saveSettings()
{
    MO_ASSERT(!objectName().isEmpty(), "");

    QVariant v = QVariant(p_files_);
    settings()->setValue("RecentFiles/" + objectName(), v);
}

void RecentFiles::loadSettings()
{
    MO_ASSERT(!objectName().isEmpty(), "");

    const QString id = "RecentFiles/" + objectName();
    if (settings()->contains(id))
    {
        QVariant v = settings()->getValue(id);
        p_files_ = v.toStringList();

        // schmeiss weg die überflüssigen
        while (p_files_.size() > p_max_)
            p_files_.pop_back();

        p_updateMenu_();
    }
}


} // namespace GUI
} // namespace MO
