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
#include "io/xmlstream.h"
#include "io/log.h"

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
    //const QString mopath = "/home/defgsus/prog/qt_project/mo/matrixoptimizer";
    //const QString mopath = "/home/defgsus/prog/C/mo/matrixoptimizer3";
    const QString mopath = ".";

    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_GEOMETRY_SETTINGS]]
                                = mopath + "/data/geometry_presets";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_SCENE]]
                                = mopath + "/data/scene";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_EQUATION_PRESET]]
                                = mopath + "/data/equations";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_PROJECTION_SETTINGS]]
                                = mopath + "/data/projection_settings";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_GEOMETRY_SETTINGS]]
                                = mopath + "/data/geometry_presets";

    defaultValues_["File/scene"] = "";

    defaultValues_["Directory/filecache"] = mopath + "/data/cache";
    defaultValues_["File/filecache"] = mopath + "/data/cache/filecache.xml";

    defaultValues_["Audio/api"] = "";
    defaultValues_["Audio/device"] = "";
    defaultValues_["Audio/samplerate"] = 44100;
    defaultValues_["Audio/buffersize"] = 128;
    defaultValues_["Audio/channelsIn"] = 2;
    defaultValues_["Audio/channelsOut"] = 2;

    defaultValues_["MidiIn/api"] = "";
    defaultValues_["MidiIn/device"] = "";

    defaultValues_["EquEdit/equation"] = "sin(x*TWO_PI)";
    defaultValues_["EquEdit/paintmode"] = 0;
    defaultValues_["EquEdit/x0"] = -1;
    defaultValues_["EquEdit/x1"] = 1;
    defaultValues_["EquEdit/y0"] = -1;
    defaultValues_["EquEdit/y1"] = 1;

    defaultValues_["Network/name"] = "";
    defaultValues_["Network/tcpport"] = 50000;
    defaultValues_["Network/udpport"] = 50001;

    defaultValues_["Client/index"] = 0;
    defaultValues_["Client/serverAddress"] = "192.168.1.33";
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
    MO_DEBUG_GUI("Settings::saveGeometry(" << win << ", " << win->geometry());

    MO_ASSERT(!win->objectName().isEmpty(), "Settings::saveGeometry(): no objectName set for QWindow");
    setValue("Geometry/" + win->objectName(), win->geometry());
}

bool Settings::restoreGeometry(QWindow * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "Settings::restoreGeometry(): no objectName set for QWindow");
    const QString key = "Geometry/" + win->objectName();
    if (contains(key))
    {
        win->setGeometry( getValue(key).value<QRect>() );
        MO_DEBUG_GUI("Settings::restoreGeometry(" << win << ", " << win->geometry());
        return true;
    }
    return false;
}

void Settings::saveGeometry(QWidget * win)
{
    MO_DEBUG_GUI("Settings::saveGeometry(" << win << ", " << win->geometry());

    MO_ASSERT(!win->objectName().isEmpty(), "Settings::saveGeometry(): no objectName set for QWidget");
    setValue("Geometry/" + win->objectName(), win->saveGeometry());
    //setValue("Geometry/" + win->objectName(), win->geometry());
}

bool Settings::restoreGeometry(QWidget * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "Settings::restoreGeometry(): no objectName set for QWidget");
    const QString key = "Geometry/" + win->objectName();
    if (contains(key))
    {
        win->restoreGeometry( getValue(key).toByteArray() );
        //win->setGeometry( getValue(key).value<QRect>() );
        MO_DEBUG_GUI("Settings::restoreGeometry(" << win << ", " << win->geometry());
        return true;
    }
    return false;
}


int Settings::clientIndex()
{
    return getValue("Client/index").toInt();
}

void Settings::setClientIndex(int i)
{
    setValue("Client/index", i);
}


void Settings::setDefaultProjectionSettings(const ProjectionSystemSettings &p)
{
    QByteArray data;
    p.serialize(data);

    setValue("ProjectionSystem/xml", data);
}

ProjectionSystemSettings Settings::getDefaultProjectionSettings()
{
    ProjectionSystemSettings s;

    QByteArray data = getValue("ProjectionSystem/xml").toByteArray();
    if (data.isEmpty())
    {
        s.appendProjector(ProjectorSettings());
        return s;
    }

    try
    {
        s.deserialize(data);
    }
    catch (const Exception& e)
    {
        MO_IO_WARNING(READ, "Failed to deserialize ProjectionSystemSettings from bytearray\n"
                      << e.what());
        s.clear();
        s.appendProjector(ProjectorSettings());
    }

    return s;
}


DomeSettings Settings::domeSettings()
{
    ProjectionSystemSettings s(getDefaultProjectionSettings());
    return s.domeSettings();
}

ProjectorSettings Settings::projectorSettings()
{
    ProjectionSystemSettings s(getDefaultProjectionSettings());

    const int idx = clientIndex();
    if (idx >= (int)s.numProjectors())
    {
        MO_WARNING("Settings::projectorSettings() clientIndex == " << idx
                   << " numProjectors == " << s.numProjectors());
        return projectorSettings();
    }
    return s.projectorSettings(idx);
}

CameraSettings Settings::cameraSettings()
{
    ProjectionSystemSettings s(getDefaultProjectionSettings());

    const int idx = clientIndex();
    if (idx >= (int)s.numProjectors())
    {
        MO_WARNING("Settings::cameraSettings() clientIndex == " << idx
                   << " numProjectors == " << s.numProjectors());
        return cameraSettings();
    }
    return s.cameraSettings(idx);
}

} // namespace MO
