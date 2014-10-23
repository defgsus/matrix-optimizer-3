/** @file clientenginecommandline.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.10.2014</p>
*/

#include <QUrl>

#include "clientenginecommandline.h"
#include "io/commandlineparser.h"
#include "io/settings.h"
#include "io/version.h"
#include "io/log.h"
#include "io/systeminfo.h"

namespace MO {

ClientEngineCommandLine::ClientEngineCommandLine(QObject *parent)
    : QObject           (parent),
      cl_               (new IO::CommandLineParser()),
      doNetwork_        (false),
      doShowInfoWin_    (false),
      sceneFile_        ()
{
    // ---- init commandline parameters -----


    // -- display --

    cl_->addParameter("help", "h,help",
                      tr("Prints the help and exits"));
    cl_->addParameter("info", "info",
                      tr("Prints the system info and exits"));

    // -- client settings --

    cl_->addParameter("server", "server",
                      tr("Sets the server IP. The client will constantly try to "
                         "connect itself to that address and the ip is stored as the default.\n"
                         "The parameter accepts the common ip notation (xxx.yyy.zzz.www)"),
                      settings->serverAddress());
    cl_->addParameter("clientindex", "cindex, client-index",
                      tr("Sets the client index to the given value.\n"
                         "This is the index used by the server to associate a projector."),
                      settings->clientIndex());
    cl_->addParameter("desktop", "desktop",
                      tr("Selects the desktop for output windows. "
                         "The value will be stored as default."),
                      settings->desktop());

    // -- particular modis --

    cl_->addParameter("infowin", "infowin, info-window",
                      tr("Displays the info window on the selected desktop"));

    cl_->addParameter("scene", "scene",
                      tr("The client will load and run the specified scene file upon start."),
                      "");

    cl_->addParameter("nonet", "nonet",
                      tr("Turns off networking for the client. "
                         "This is for debugging purposes only."));

}


ClientEngineCommandLine::~ClientEngineCommandLine()
{
    delete cl_;
}

ClientEngineCommandLine::ReturnValue
    ClientEngineCommandLine::parse(int argc, char **argv)
{
#if (0)
    // compile-time commandline params for debug purposes
    Q_UNUSED(argc)
    Q_UNUSED(argv)
    QStringList args;
    args
//            << "-info"
            << "-nonet"
            << "-desktop" << "1"
            << "-infowin"
            //<< "-scene" << "../matrixoptimizer3/data/scene/boxgrid2.mo3"
            ;
    if (!cl_->parse(args))
#else

    // check commandline
    if (!cl_->parse(argc, argv, 1))
#endif
    {
        MO_PRINT(tr("Use -h to get help"));
        return Error;
    }


    doNetwork_ = !cl_->contains("nonet");
    doShowInfoWin_ = cl_->contains("infowin");

    // set server IP
    if (cl_->contains("server"))
    {
        QString ip = cl_->value("server").toString();
        QUrl url(ip);
        if (!url.isValid())
        {
            MO_PRINT(tr("Could not parse the server ip") << " '" << ip << "'");
            return Error;
        }
        settings->setServerAddress(ip);
        MO_PRINT("server ip: " << ip);
    }

    // set desktop
    if (cl_->contains("desktop"))
    {
        const int idx = cl_->value("desktop").toInt();
        settings->setDesktop(idx);
        MO_PRINT(tr("desktop index") << ": " << idx);
    }

    // set client index
    if (cl_->contains("clientindex"))
    {
        const int idx = cl_->value("clientindex").toInt();
        settings->setClientIndex(idx);
        MO_PRINT(tr("client index") << ": " << idx);
    }

    // scene file
    if (cl_->contains("scene"))
    {
        sceneFile_ = cl_->value("scene").toString();
        MO_PRINT(tr("fixed scene file") << ": " << sceneFile_);
    }


    // ------------------- display ---------------------

    if (cl_->contains("help"))
    {
        MO_PRINT(tr("Usage") << ":\n" << cl_->helpString());
        return Quit;
    }

    if (cl_->contains("info"))
    {
        SystemInfo info;
        info.get();
        MO_PRINT(tr("Info") << ":\n" << info.toString());
        return Quit;
    }

    return Ok;
}

} // namespace MO
