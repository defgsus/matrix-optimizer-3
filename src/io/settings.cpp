/** @file settings.cpp

    @brief Application settings

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#include <QMainWindow>
#include <QWindow>
#include <QDesktopWidget>

#include "settings.h"
#include "io/error.h"
#include "io/files.h"
#include "io/application.h"
#include "projection/projectionsystemsettings.h"
#include "io/xmlstream.h"
#include "io/log.h"
#include "io/isclient.h"

#ifdef Q_OS_LINUX
#   define MO_GEOMETRY_SAVE_HACK
#endif

namespace MO {

Settings * settings = 0;

Settings::Settings(QObject *parent) :
    QSettings(
        QSettings::IniFormat,
        QSettings::UserScope,
        "modular-audio-graphics",
        isClient()
            ? "matrix-optimizer-3-client"
            : "matrix-optimizer-3",
        parent)
{
    createDefaultValues_();
}

void Settings::createDefaultValues_()
{
    // --- application stats ---

    defaultValues_["Status/desktopRuns"] = 0;
    defaultValues_["Status/serverRuns"] = 0;
    defaultValues_["Status/clientRuns"] = 0;

    // --- default directories ---

    //const QString mopath = "/home/defgsus/prog/qt_project/mo/matrixoptimizer";
    //const QString mopath = "/home/defgsus/prog/C/mo/matrixoptimizer3";
    const QString mopath = ".";

    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_GEOMETRY_SETTINGS]]
                                = mopath + "/data/geometry_presets";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_SCENE]]
                                = mopath + "/data/scene";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_OBJECT_TEMPLATE]]
                                = mopath + "/data/object_templates";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_EQUATION_PRESET]]
                                = mopath + "/data/equations";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_PROJECTION_SETTINGS]]
                                = mopath + "/data/projection_settings";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_GEOMETRY_SETTINGS]]
                                = mopath + "/data/geometry_presets";

    defaultValues_["File/scene"] = "";

    defaultValues_["Directory/filecache"] = mopath + "/data/cache";
    defaultValues_["File/filecache"] = mopath + "/data/cache/filecache.xml";

    // --- hardware settings ---

    defaultValues_["Audio/api"] = "";
    defaultValues_["Audio/indevice"] = "";
    defaultValues_["Audio/outdevice"] = "";
    defaultValues_["Audio/samplerate"] = 44100;
    defaultValues_["Audio/buffersize"] = 128;
    defaultValues_["Audio/channelsIn"] = 2;
    defaultValues_["Audio/channelsOut"] = 2;

    defaultValues_["MidiIn/api"] = "";
    defaultValues_["MidiIn/device"] = "";

    // -- equation editor ---

    defaultValues_["EquEdit/equation"] = "sin(x*TWO_PI)";
    defaultValues_["EquEdit/paintmode"] = 0;
    defaultValues_["EquEdit/x0"] = -1;
    defaultValues_["EquEdit/x1"] = 1;
    defaultValues_["EquEdit/y0"] = -1;
    defaultValues_["EquEdit/y1"] = 1;

    // -- networking ---

    defaultValues_["Network/name"] = "";
    defaultValues_["Network/tcpport"] = 50000;
    defaultValues_["Network/udpport"] = 50001;

    defaultValues_["Client/index"] = 0;
    defaultValues_["Client/serverAddress"] = "192.168.1.33";

    // -- client --

    defaultValues_["Client/desktopIndex"] = 0;
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
void Settings::storeGeometry(QWindow * win)
{
    MO_DEBUG_GUI("Settings::saveGeometry(" << win << ", " << win->geometry());

    MO_ASSERT(!win->objectName().isEmpty(), "Settings::saveGeometry(): no objectName set for QWindow");

#ifndef MO_GEOMETRY_SAVE_HACK
    setValue("Geometry/" + win->objectName(), win->geometry());
#else
    // try to fix the window header bug on X11
    setValue("Geometry/Geom/" + win->objectName(), win->geometry());
    setValue("Geometry/Frame/" + win->objectName(), win->frameGeometry());
#endif
}

bool Settings::restoreGeometry(QWindow * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "Settings::restoreGeometry(): no objectName set for QWindow");

    bool found = false;

#ifndef MO_GEOMETRY_SAVE_HACK
    const QString key = "Geometry/" + win->objectName();
    if (contains(key))
    {
        win->setGeometry( getValue(key).value<QRect>() );
        found = true;
    }
#else
    const QString
            keyGeom = "Geometry/Geom/" + win->objectName(),
            keyPos = "Geometry/Frame/" + win->objectName();

    if (contains(keyGeom))
    {
        const QRect r = getValue(keyGeom).value<QRect>();
        win->setGeometry(r);
        found = true;
    }
    /*
    if (contains(keyPos))
    {
        const QRect r = getValue(keyPos).value<QRect>();
        // XXX It's all a hack (X11 is a bitch regarding this)
        QRect winr = win->geometry();
        winr.moveTop(setTop(r.top() + 10);
        win->setGeometry(winr);
    }
    */
#endif

    if (!found)
    {
        // center on screen
        QRect r = application->desktop()->screenGeometry(win->position());
        win->setGeometry((r.width() - win->width())/2,
                         (r.height() - win->height())/2,
                         win->width(), win->height());
    }
    else
        MO_DEBUG_GUI("Settings::restoreGeometry(" << win << ", " << win->geometry());

    return found;
}

void Settings::storeGeometry(QWidget * win)
{
    MO_DEBUG_GUI("Settings::saveGeometry(" << win << ", " << win->geometry()
             << ", " << win->frameGeometry() << ")");

    MO_ASSERT(!win->objectName().isEmpty(), "Settings::saveGeometry(): no objectName set for QWidget");

#ifndef MO_GEOMETRY_SAVE_HACK
    setValue("Geometry/" + win->objectName(), win->saveGeometry());
#else
    // try to fix the window header bug on X11
    setValue("Geometry/Geom/" + win->objectName(), win->geometry());
    setValue("Geometry/Frame/" + win->objectName(), win->frameGeometry());
#endif

    // write dockwidget/toolbar state
    if (QMainWindow * main = qobject_cast<QMainWindow*>(win))
    {
        setValue("Geometry/WinState/" + win->objectName(), main->saveState(1));
    }
}

bool Settings::restoreGeometry(QWidget * win)
{
    MO_ASSERT(!win->objectName().isEmpty(), "Settings::restoreGeometry(): no objectName set for QWidget");

    bool found = false;

#ifndef MO_GEOMETRY_SAVE_HACK
    const QString key = "Geometry/" + win->objectName();
    if (contains(key))
    {
        win->restoreGeometry( getValue(key).toByteArray() );
        found = true;
    }
#else
    const QString
            keyGeom = "Geometry/Geom/" + win->objectName(),
            keyPos = "Geometry/Frame/" + win->objectName();

    if (contains(keyGeom))
    {
        const QRect r = getValue(keyGeom).value<QRect>();
        win->setGeometry(r);
        found = true;
    }
    if (contains(keyPos))
    {
        const QRect r = getValue(keyPos).value<QRect>();
        // XXX It's all a hack (X11 is a bitch regarding this)
        win->move(win->x(), r.top()
                  //+ 10
                  );
    }
#endif

    if (!found)
    {
        // center on screen
        QRect r = application->desktop()->screenGeometry(win->pos());
        win->setGeometry((r.width() - win->width())/2,
                         (r.height() - win->height())/2,
                         win->width(), win->height());
    }
    else
        MO_DEBUG_GUI("Settings::restoreGeometry(" << win << ", " << win->geometry());

    // restore dockwidgets state
    if (QMainWindow * main = qobject_cast<QMainWindow*>(win))
    {
        const QString key = "Geometry/WinState/" + win->objectName();
        if (contains(key))
        {
            const QByteArray data = getValue(key).toByteArray();
            main->restoreState(data, 1);
        }
    }

    return found;
}

QString Settings::serverAddress()
{
    return getValue("Client/serverAddress").toString();
}

void Settings::setServerAddress(const QString & a)
{
    setValue("Client/serverAddress", a);
}

int Settings::clientIndex()
{
    return getValue("Client/index").toInt();
}

void Settings::setClientIndex(int i)
{
    setValue("Client/index", i);
}

uint Settings::desktop()
{
    return getValue("Client/desktopIndex").toInt();
}

void Settings::setDesktop(uint index)
{
    setValue("Client/desktopIndex", index);
}

QString Settings::styleSheet() const
{
    return value("Application/styleSheet",
                 "* { background-color: #202020; color: #a0a0a0; "
                 "    border: 1px solid #404040; "
                 "    selection-background-color: #505060; "
                 "    selection-color: #ffffff } "
                 "*:hover { background-color: #242424 } "
                 "*:pressed { background-color: #121212 } "
                 "QLabel { border: 0px } "
                 "* { show-decoration-selected: 0 } "
                 "QAbstractItemView { background-color: #808080 } "
                 "QAbstractItemView:hover { background-color: #868686 } ").toString();
}

void Settings::setStyleSheet(const QString & s)
{
    setValue("Application/styleSheet", s);
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
