/** @file settings.cpp

    @brief Application settings

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#include <QMainWindow>
#include <QWindow>
#include <QDesktopWidget>
#include <QFile>
#include <QTextStream>

#include "settings.h"
#include "projection/projectionsystemsettings.h"
#include "gl/window.h"
#include "io/diskrendersettings.h"
#include "io/error.h"
#include "io/files.h"
#include "io/application.h"
#include "io/xmlstream.h"
#include "io/log_gui.h"
#include "io/isclient.h"

#ifdef Q_OS_LINUX
#   define MO_GEOMETRY_SAVE_HACK
#endif

namespace MO {

namespace {
    /** Name of the application ini file */
    QString appIniString()
    {
        QString n = isClient()
            ? "matrix-optimizer-3-client"
            : "matrix-optimizer-3";

        // append user name
        if (!application()->sessionId().isEmpty())
            n += "-" + application()->sessionId();

        return n;
    }
}

Settings * settings()
{
    static Settings * s = 0;
    if (!s)
        s = new Settings(application());
    return s;
}

Settings::Settings(QObject *parent) :
    QSettings(
        QSettings::IniFormat,
        QSettings::UserScope,
        "modular-audio-graphics",
        appIniString(),
        parent)
{
    createDefaultValues_();
}

QString Settings::infoString() const
{
    QString str;
    QTextStream s(&str);
    s
        << "mode       : " << (isClient() ? "CLIENT" : isServer() ? "SERVER" : "DESKTOP")
        << (application()->userName().isEmpty() ? "" : QString("\nuser       : %1").arg(application()->userName()))
        << "\nnum. runs  : " << getValue("Status/desktopRuns").toString()
#ifndef MO_DISABLE_SERVER
                    << " / " << getValue("Status/serverRuns").toString() << " server"
#endif
#ifndef MO_DISABLE_CLIENT
                    << " / " << getValue("Status/clientRuns").toString() << " client"
#endif
#ifndef MO_DISABLE_CLIENT
        << "\nserver     : " << serverAddress()
#endif
#if !defined(MO_DISABLE_CLIENT) || !defined(MO_DISABLE_SERVER)
        << "\ntcp comm.  : " << getValue("Network/tcpport").toString()
        << "\nudp comm.  : " << getValue("Network/udpport").toString()
        << "\nudp-audio  : " << settings()->udpAudioMulticastAddress() << ":"
                             << settings()->udpAudioMulticastPort()
#endif
    ;

    return str;
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
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_TEXTURE]]
                                = mopath + "/data/graphic";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_NORMAL_MAP]]
                                = mopath + "/data/graphic";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_SOUND]]
                                = mopath + "/data/audio";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_EQUATION_PRESET]]
                                = mopath + "/data/equations";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_PROJECTION_SETTINGS]]
                                = mopath + "/data/projection_settings";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_INTERFACE_XML]]
                                = mopath + "/data/interface";
    defaultValues_["Directory/" + IO::fileTypeIds[IO::FT_INTERFACE_PRESET]]
                                = mopath + "/data/interface/presets";

    defaultValues_["File/scene"] = "";

    defaultValues_["Directory/filecache"] = mopath + "/data/cache";
    defaultValues_["File/filecache"] = mopath + "/data/cache/filecache.xml";

    // --- asset browser default directories ---

    defaultValues_["AssetBrowser/Directory/0"] = mopath + "/data/scene";
    defaultValues_["AssetBrowser/Filter/0"] = "*.mo3";
    defaultValues_["AssetBrowser/Directory/1"] = mopath + "/data/object_templates";
    defaultValues_["AssetBrowser/Directory/2"] = mopath + "/data/geometry_presets";
    defaultValues_["AssetBrowser/Directory/3"] = mopath + "/data/graphic";
    defaultValues_["AssetBrowser/Directory/4"] = mopath + "/data/audio";

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
    defaultValues_["Network/udpAudioMulticastAddress"] = "239.255.43.21";
    defaultValues_["Network/udpAudioMulticastPort"] = 50005;

    defaultValues_["Client/index"] = 0;
    defaultValues_["Client/serverAddress"] = "192.168.1.33";

    // -- client --

    defaultValues_["Client/desktopIndex"] = 0;

    // -- server --

    defaultValues_["Server/running"] = false;
}

QVariant Settings::getValue(const QString &key) const
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

QVariant Settings::getValue(const QString &key, const QVariant& def) const
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

    //MO_WARNING("unknown setting '" << key << "', using default");

    // not found
    return def;
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

    // save fullscreen data
    if (GL::Window * glwin = dynamic_cast<GL::Window*>(win))
    {
        setValue("Geometry/Fullscreen/" + win->objectName(), glwin->isFullscreen());
        if (glwin->isFullscreen())
            setValue("Geometry/OldGeom/" + win->objectName(), glwin->oldGeometry());
    }
}

bool Settings::restoreGeometry(QWindow * win)
{
    MO_ASSERT(!win->objectName().isEmpty(),
              "Settings::restoreGeometry(): no objectName set for QWindow");

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

    // restore fullscreen data
    if (GL::Window * glwin = dynamic_cast<GL::Window*>(win))
    {
        const QString
                keyFs = "Geometry/Fullscreen/" + win->objectName(),
                keyOldGeom = "Geometry/OldGeom/" + win->objectName();
        if (contains(keyFs) && value(keyFs).toBool())
        {
            const QRect r = getValue(keyOldGeom).value<QRect>();
            win->setGeometry(r);
            glwin->setFullscreen(true);
        }
    }

    if (!found)
    {
        // center on screen
        QRect r = application()->desktop()->screenGeometry(win->position());
        int w = r.width() / 3, h = r.height() / 3;
        win->setGeometry((r.width() - w)/2,
                         (r.height() - h)/2,
                         w, h);
    }
    else
        MO_DEBUG_GUI("Settings::restoreGeometry(" << win << ", " << win->geometry());

    return found;
}

void Settings::storeGeometry(QWidget * win)
{
    MO_DEBUG_GUI("Settings::saveGeometry(" << win << ", " << win->geometry()
             << ", " << win->frameGeometry() << ")");

    MO_ASSERT(!win->objectName().isEmpty(),
              "Settings::saveGeometry(): no objectName set for QWidget");

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
        QRect r = application()->desktop()->screenGeometry(win->pos());
        int w = r.width() / 3, h = r.height() / 3;
        win->setGeometry((r.width() - w)/2,
                         (r.height() - h)/2,
                         w, h);
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

namespace {
    const QString presetKey = "WinStatePresets";
    static const char* defaultPreset =
            "%00%00%00%FF%00%00%00%01%FD%00%00%00%02%00%00%00%00%00%00%00%F8%00%00%02%1D%FC%02%00%00%00%01%FC%00%00%00%1B%00%00%01%0D%00%00%00%00%00%FF%FF%FF%FA%00%00%00%00%01%00%00%00%01%FB%00%00%00%1E%00_%00F%00r%00o%00n%00t%00V%00i%00e%00w%00_%00D%00o%00c%00k%01%00%00%00%00%FF%FF%FF%FF%00%00%00%00%00%00%00%00%00%00%00%01%00%00%05%9E%00%00%01w%FC%01%00%00%00%04%FB%00%00%00%2A%00O%00b%00j%00e%00c%00t%00O%00u%00t%00p%00u%00t%00V%00i%00e%00w%00_%00D%00o%00c%00k%00%00%00%00%A6%00%00%01%18%00%00%00K%00%FF%FF%FF%FC%00%00%00%00%00%00%02%EB%00%00%02%BA%00%FF%FF%FF%FA%00%00%00%00%02%00%00%00%0A%FB%00%00%00%2A%00_%00O%00b%00j%00e%00c%00t%00G%00r%00a%00p%00h%00V%00i%00e%00w%00_%00D%00o%00c%00k%01%00%00%00%00%FF%FF%FF%FF%00%00%00%BF%00%FF%FF%FF%FB%00%00%00%2A%00_%00t%00r%00a%00n%00s%00p%00o%00r%00t%00W%00i%00d%00g%00e%00t%00_%00D%00o%00c%00k%01%00%00%00%00%FF%FF%FF%FF%00%00%00Y%00%FF%FF%FF%FB%00%00%00%24%00_%00S%00e%00q%00u%00e%00n%00c%00e%00V%00i%00e%00w%00_%00D%00o%00c%00k%01%00%00%00%00%FF%FF%FF%FF%00%00%00%3F%00%FF%FF%FF%FB%00%00%00%1A%00_%00O%00s%00c%00V%00i%00e%00w%00_%00D%00o%00c%00k%01%00%00%00%00%FF%FF%FF%FF%00%00%00k%00%FF%FF%FF%FB%00%00%00%26%00_%00S%00e%00q%00u%00e%00n%00c%00e%00r%00V%00i%00e%00w%00_%00D%00o%00c%00k%01%00%00%00%00%FF%FF%FF%FF%00%00%01Y%00%FF%FF%FF%FB%00%00%00%2A%00_%00F%00r%00o%00n%00t%00I%00t%00e%00m%00E%00d%00i%00t%00o%00r%00_%00D%00o%00c%00k%01%00%00%00%00%FF%FF%FF%FF%00%00%00%00%00%00%00%00%FB%00%00%00%20%00_%00S%00e%00r%00v%00e%00r%00V%00i%00e%00w%00_%00D%00o%00c%00k%00%00%00%00%00%FF%FF%FF%FF%00%00%01Y%00%FF%FF%FF%FB%00%00%00%1C%00_%00C%00l%00i%00p%00V%00i%00e%00w%00_%00D%00o%00c%00k%00%00%00%00%00%FF%FF%FF%FF%00%00%00%91%00%FF%FF%FF%FB%00%00%00%0A%00_%00D%00o%00c%00k%01%00%00%00%00%FF%FF%FF%FF%00%00%00%00%00%00%00%00%FB%00%00%00%24%00_%00A%00s%00s%00e%00t%00B%00r%00o%00w%00s%00e%00r%00_%00D%00o%00c%00k%01%00%00%00%00%FF%FF%FF%FF%00%00%00%A3%00%FF%FF%FF%FB%00%00%00%26%00O%00b%00j%00e%00c%00t%00T%00r%00e%00e%00V%00i%00e%00w%00_%00D%00o%00c%00k%01%00%00%02%F0%00%00%00%EE%00%00%00K%00%FF%FF%FF%FB%00%00%00%20%00_%00O%00b%00j%00e%00c%00t%00V%00i%00e%00w%00_%00D%00o%00c%00k%01%00%00%03%E3%00%00%01%BB%00%00%00j%00%FF%FF%FF%00%00%00%00%00%00%01w%00%00%00%04%00%00%00%04%00%00%00%08%00%00%00%08%FC%00%00%00%00";
}

QStringList Settings::viewPresets()
{
    beginGroup(presetKey);
    auto list = childKeys();
    endGroup();

    list.prepend("factory default");
    return list;
}

void Settings::storeViewPreset(const QString& name, QMainWindow* w)
{
    setValue(presetKey + "/" + name, w->saveState(1));
}

bool Settings::restoreViewPreset(const QString& name, QMainWindow* w)
{
    if (name == "factory default")
    {
        QByteArray data = QByteArray::fromPercentEncoding(
                    defaultPreset);
        return w->restoreState(data, 1);
    }

    const QString key = presetKey + "/" + name;
    if (!contains(key))
        return false;
    return w->restoreState(value(key).toByteArray(), 1);
}

QString Settings::dataPath() const
{
    return "./data";
}


QString Settings::serverAddress() const
{
    return getValue("Client/serverAddress").toString();
}

void Settings::setServerAddress(const QString & a)
{
    setValue("Client/serverAddress", a);
}

QString Settings::udpAudioMulticastAddress() const
{
    return getValue("Network/udpAudioMulticastAddress").toString();
}

void Settings::setUdpAudioMulticastAddress(const QString & a)
{
    setValue("Network/udpAudioMulticastAddress", a);
}

uint16_t Settings::udpAudioMulticastPort() const
{
    return getValue("Network/udpAudioMulticastPort").toUInt();
}

void Settings::setUdpAudioMulticastPort(uint16_t p)
{
    setValue("Network/udpAudioMulticastPort", p);
}

int Settings::clientIndex() const
{
    return getValue("Client/index").toInt();
}

void Settings::setClientIndex(int i)
{
    setValue("Client/index", i);
}

uint Settings::desktop() const
{
    return getValue("Client/desktopIndex").toInt();
}

void Settings::setDesktop(uint index)
{
    setValue("Client/desktopIndex", index);
}

QString Settings::styleSheet() const
{
    if (contains("Application/styleSheet"))
        return value("Application/styleSheet").toString();

    QFile f(":/stylesheet.css");
    if (!f.open(QFile::ReadOnly | QFile::Text))
        MO_WARNING("Could not load default stylesheet from resources.\n"
                   << f.errorString());
    QTextStream s(&f);
    return s.readAll();
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

    QByteArray data;
    if (contains("ProjectionSystem/xml"))
        data = getValue("ProjectionSystem/xml").toByteArray();

    // create default settings
    if (data.isEmpty())
    {
        s.appendProjector(ProjectorSettings());
        return s;
    }

    // load settings
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

DiskRenderSettings Settings::getDiskRenderSettings()
{
    return DiskRenderSettings();
}

void Settings::setDiskRenderSettings(const DiskRenderSettings & s)
{

}

} // namespace MO
