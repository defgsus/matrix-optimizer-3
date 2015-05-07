/** @file ladspaloader.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MO_DISABLE_LADSPA

#include <dlfcn.h>
#include <cstdlib>

#include "ladspaloader.h"
#include "audio/3rd/ladspa.h"
#include "audio/tool/ladspaplugin.h"
#include "io/files.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace IO {


LadspaLoader::LadspaLoader()
{
}

LadspaLoader::~LadspaLoader()
{
    clear();
}

void LadspaLoader::clear()
{
    for (auto & p : p_map_)
        p->releaseRef();
    p_map_.clear();
}

QList<AUDIO::LadspaPlugin*> LadspaLoader::getPlugins() const
{
    QList<AUDIO::LadspaPlugin*> list;
    for (auto & p : p_map_)
        list << p;
    return list;
}

AUDIO::LadspaPlugin * LadspaLoader::getPlugin(const QString &filename, const QString &label) const
{
    QString key = "LADSPA:" + filename + ":" + label;
    auto it = p_map_.find(key);
    return it == p_map_.end() ? 0 : it.value();
}


QString LadspaLoader::getEnvironmentPath()
{
    const char * content = getenv("LADSPA_PATH");
    if (!content)
        return QString();

    return QString::fromLocal8Bit(content);
}

size_t LadspaLoader::loadDirectory(const QString& dir, bool recursive)
{
    QStringList files;
    IO::Files::findFiles(FT_LADSPA, dir, recursive, files);
    if (files.isEmpty())
        return false;

    size_t num = 0;

    for (const auto & fn : files)
    {
        try
        {
            num += loadLibrary(fn);
        }
        catch (const Exception& e)
        {
            MO_WARNING("Failed to load library\n" << e.what());
        }
    }

    return num;
}

size_t LadspaLoader::loadLibrary(const QString &fn)
{
    //MO_DEBUG("loadLibrary(" << fn << ")");

    // open library

    /// @todo Need to close plugin DLs at some point
    void * res = dlopen(fn.toStdString().c_str(),
                        RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    if (!res)
        MO_IO_ERROR(READ, QObject::tr("Failed to load plugin\n'%1'\n%2")
                            .arg(fn).arg(dlerror()));

    // get descriptors

    LADSPA_Descriptor_Function pfDescriptorFunction;

    dlerror();
    pfDescriptorFunction =
            (LADSPA_Descriptor_Function)
                dlsym(res, "ladspa_descriptor");
    if (!pfDescriptorFunction)
    {
        MO_IO_ERROR(READ, QObject::tr("Could not find ladspa_descriptor function in plugin\n'%1'\n%2")
                    .arg(fn).arg(dlerror()));
    }

    // check for single plugins
    uint idx = 0;
    for (;; ++idx)
    {
        const LADSPA_Descriptor * desc = pfDescriptorFunction(idx);
        if (!desc)
        {
            //MO_DEBUG("Exit library, found " << idx);
            break;
        }

        // avoid duplicates
        QString key = "LADSPA:" + fn + ":" + desc->Label;
        if (p_map_.contains(key))
        {
            MO_WARNING("Duplicate ladspa key '" << key << "'");
            continue;
        }

        // create wrapper
        auto plug = new AUDIO::LadspaPlugin();
        // init
        plug->p_desc_ = desc;
        plug->p_filename_ = fn;

        plug->p_getPorts_();

        p_map_.insert(key, plug);
    }

    return idx;

    //MO_IO_ERROR(READ, QObject::tr("Could not find descriptor in plugin\n'%1'").arg(fn));
}

AUDIO::LadspaPlugin * LadspaLoader::loadPlugin(const QString &filename, const QString &label)
{
    LadspaLoader loader;
    try
    {
        size_t num = loader.loadLibrary(filename);
        if (!num)
            return 0;
    }
    catch (const Exception& e)
    {
        MO_WARNING("Loading plugin library failed\n" << e.what());
        return 0;
    }

    auto plug = loader.getPlugin(filename, label);
    if (plug)
        plug->addRef();

    return plug;
}

} // namespace IO
} // namespace MO

#endif // #ifndef MO_DISABLE_LADSPA
