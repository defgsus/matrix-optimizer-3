/** @file settings.cpp

    @brief Application settings

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#include <QMainWindow>
#include <QWindow>

#include "settings.h"
#include "io/error.h"
#include "io/files.h"
#include "projection/projectionsystemsettings.h"

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
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_GEOMETRY_SETTINGS]]
                                = "./data/geometry_presets";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_SCENE]] = "./data/scene";
    defaultValues_["File/scene"] = "";

    defaultValues_["Audio/api"] = "";
    defaultValues_["Audio/device"] = "";
    defaultValues_["Audio/samplerate"] = 44100;
    defaultValues_["Audio/buffersize"] = 128;
    defaultValues_["Audio/channelsIn"] = 2;
    defaultValues_["Audio/channelsOut"] = 2;

    defaultValues_["EquEdit/equation"] = "sin(x*TWO_PI)";
    defaultValues_["EquEdit/paintmode"] = 0;
    defaultValues_["EquEdit/x0"] = -1;
    defaultValues_["EquEdit/x1"] = 1;
    defaultValues_["EquEdit/y0"] = -1;
    defaultValues_["EquEdit/y1"] = 1;
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

    if (i != defaultValues_.end())
        return i.value();

    // not found

    MO_WARNING("unknown setting '" << key << "' requested");

    return QVariant();
}
/*
void Settings::saveGeometry(QMainWindow * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "no objectName set for QMainWindow");
    setValue("Geometry/" + win->objectName(), win->saveState());
}

void Settings::restoreGeometry(QMainWindow * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "no objectName set for QMainWindow");
    const QString key = "Geometry/" + win->objectName();
    if (contains(key))
        win->restoreState( getValue(key).toByteArray() );
}
*/
void Settings::saveGeometry(QWindow * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "no objectName set for QWindow");
    setValue("Geometry/" + win->objectName(), win->geometry());
}

bool Settings::restoreGeometry(QWindow * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "no objectName set for QWindow");
    const QString key = "Geometry/" + win->objectName();
    if (contains(key))
    {
        win->setGeometry( getValue(key).value<QRect>() );
        return true;
    }
    return false;
}

void Settings::saveGeometry(QWidget * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "no objectName set for QWidget");
    setValue("Geometry/" + win->objectName(), win->saveGeometry());
}

bool Settings::restoreGeometry(QWidget * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "no objectName set for QWidget");
    const QString key = "Geometry/" + win->objectName();
    if (contains(key))
    {
        win->restoreGeometry( getValue(key).toByteArray() );
        return true;
    }
    return false;
}


DomeSettings Settings::domeSettings() const
{
    ProjectionSystemSettings s;
    s.loadFile("/home/defgsus/prog/qt_project/mo/matrixoptimizer/data/projection_settings/mapped01.xml-proj");
    return s.domeSettings();
}

ProjectorSettings Settings::projectorSettings() const
{
    ProjectionSystemSettings s;
    s.loadFile("/home/defgsus/prog/qt_project/mo/matrixoptimizer/data/projection_settings/mapped01.xml-proj");
    return s.projectorSettings(0);
}

CameraSettings Settings::cameraSettings() const
{
    ProjectionSystemSettings s;
    s.loadFile("/home/defgsus/prog/qt_project/mo/matrixoptimizer/data/projection_settings/mapped01.xml-proj");
    return s.cameraSettings(0);
}

} // namespace MO
