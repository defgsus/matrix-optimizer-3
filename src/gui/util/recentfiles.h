/** @file recentfiles.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/26/2015</p>
*/

#ifndef MOSRC_GUI_UTIL_RECENTFILES_H
#define MOSRC_GUI_UTIL_RECENTFILES_H

#include <QObject>
#include <QList>

class QMenu;

namespace MO {
namespace GUI {

/** Collector of recent used files, with QMenu handling */
class RecentFiles : public QObject
{
    Q_OBJECT
public:
    explicit RecentFiles(size_t maxFiles = 10, QObject *parent = 0);
    ~RecentFiles();

    // ---------------- getter -------------------

    /** The list of recent files, most recent first,
        full path filenames. */
    const QList<QString> filenames() const { return p_files_; }

    /** The list of recent files, most recent first,
        path stripped away. */
    QList<QString> filenamesStripped() const;

    // ---------------- qmenu --------------------

    /** Creates and returns a menu for all filenames.
        The menu is owned and further maintained by this class.
        Connect to QMenu::triggered(), each action will have
        the full path filename as it's data value. */
    QMenu * createMenu();

    // -------------- io -------------------------

    /** Enables automatic saving of app settings on addFilename() */
    void setAutoSave(bool enable) { p_autosave_ = enable; }

public slots:

    /** Stores current list in application settings via objectName() */
    void saveSettings();

    /** Loads current list in application settings via objectName() */
    void loadSettings();

    /** Adds a file, or puts an existing file to the top of the list.
        Call this whenever a file is saved or loaded. */
    void addFilename(const QString&);

private:

    void p_updateMenu_();

    int p_max_;
    QList<QString> p_files_;
    QMenu * p_menu_;
    bool p_autosave_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_UTIL_RECENTFILES_H
