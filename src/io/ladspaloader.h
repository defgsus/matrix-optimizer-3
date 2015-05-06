/** @file ladspaloader.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MO_DISABLE_LADSPA

#ifndef MOSRC_IO_LADSPALOADER_H
#define MOSRC_IO_LADSPALOADER_H

#include <QString>
#include <QList>
#include <QMap>

namespace MO {
namespace AUDIO { class LadspaPlugin; }
namespace IO {

/** Utility class for LADSPA plugins */
class LadspaLoader
{
public:
    LadspaLoader();
    ~LadspaLoader();

    /** Returns the LADSPA_PATH environemnt variable, or empty string */
    static QString getEnvironmentPath();

    /** Returns the plugin, or NULL. */
    AUDIO::LadspaPlugin * getPlugin(const QString& filename, const QString& label) const;

    /** Returns a list of all plugins currently loaded */
    QList<AUDIO::LadspaPlugin*> getPlugins() const;

    /** Loads the plugin or returns NULL */
    static AUDIO::LadspaPlugin * loadPlugin(const QString& filename, const QString& label);

    // -------------- setter -------------

    void clear();

    /** Calls loadLibrary() for every file in the given directory.
        Returns the number of plugins found.
        Does not throw Exception. */
    size_t loadDirectory(const QString& dir, bool recursive = true);

    /** Loads a library.
        Returns the number of plugins found.
        @throws IoException */
    size_t loadLibrary(const QString& name);

private:

    QMap<QString, AUDIO::LadspaPlugin*> p_map_;
};


} // namespace IO
} // namespace MO


#endif // MOSRC_IO_LADSPALOADER_H

#endif // #ifndef MO_DISABLE_LADSPA
