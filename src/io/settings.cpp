/** @file settings.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#include "settings.h"
#include "io/error.h"

namespace MO {

Settings * settings = 0;

Settings::Settings(QObject *parent) :
    QSettings(
        QSettings::IniFormat,
        QSettings::UserScope,
        "modular-audio-graphics",
        "matrix-optimizer-3",
        parent)
{
    createDefaultValues_();
}

void Settings::createDefaultValues_()
{
    defaultValues_["Directory/scene"] = "./";
    defaultValues_["File/scene"] = "";
}

QVariant Settings::getValue(const QString &key)
{
    // return from settings

    if (contains(key))
    {
        return value(key);
    }

    // return from default settings

    auto i = defaultValues_.find(key);

    MO_ASSERT(i != defaultValues_.end(), "unknown setting '"
              << key << "'");

    if (i != defaultValues_.end())
        return i.value();

    // not found

    return QVariant();
}


} // namespace MO
