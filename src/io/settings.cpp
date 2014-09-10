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
    //const QString mopath = "/home/defgsus/prog/qt_project/mo/matrixoptimizer";
    const QString mopath = "/home/defgsus/prog/C/mo/matrixoptimizer3";

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

    defaultValues_["Client/index"] = -1;
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


int Settings::clientIndex()
{
    return getValue("Client/index").toInt();
}

void Settings::setClientIndex(int i)
{
    setValue("Client/index", i);
}


static const char * tmp_path =
        //"/home/defgsus/prog/qt_project/mo/matrixoptimizer/data/projection_settings/mapped01.xml-proj";
        "/home/defgsus/prog/C/mo/matrixoptimizer3/data/projection_settings/mapped01.xml-proj";

DomeSettings Settings::domeSettings() const
{
    ProjectionSystemSettings s;
    s.loadFile(tmp_path);
    return s.domeSettings();
}

ProjectorSettings Settings::projectorSettings() const
{
    ProjectionSystemSettings s;
    s.loadFile(tmp_path);
    return s.projectorSettings(0);
}

CameraSettings Settings::cameraSettings() const
{
    ProjectionSystemSettings s;
    s.loadFile(tmp_path);
    return s.cameraSettings(0);
}

} // namespace MO
