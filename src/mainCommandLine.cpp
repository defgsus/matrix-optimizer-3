/** @file maincommandline.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.03.2015</p>
*/

#include <QUrl>

#include "maincommandline.h"
#include "io/CommandLineParser.h"
#include "io/Settings.h"
#include "io/version.h"
#include "io/log.h"
#include "io/systeminfo.h"
#include "io/Application.h"

namespace MO {

MainCommandLine::MainCommandLine(QObject *parent)
    : QObject           (parent),
      cl_               (new IO::CommandLineParser()),
      sceneFile_        ()
{
    // ---- init commandline parameters -----


    // -- display --

/*
    cl_->addParameter("help", "h,help",
                      tr("Prints the help and exits"));
*/
    cl_->addParameter("info", "info",
                      tr("Prints the system info and exits."));

    // -- particular modis --
/*
    cl_->addParameter("scene", "scene",
                      tr("The specified scene will be loaded and run upon start."),
                      "");
*/

    cl_->addParameter("user", "user",
                      tr("Identifies the user that runs the application.\n"
                         "Each user will maintain her/his own application settings."),
                      "");

#ifndef MO_DISABLE_SERVER
/*
    cl_->addParameter("nonet", "nonet",
                      tr("Disables the server functionality."));
*/
#endif
}

QString MainCommandLine::help(int max_width) const
{
    return cl_->helpString(max_width);
}

MainCommandLine::~MainCommandLine()
{
    delete cl_;
}

MainCommandLine::ReturnValue
    MainCommandLine::parse(int argc, char **argv, int skip)
{
    // check commandline
    if (!cl_->parse(argc, argv, skip))
    {
        MO_PRINT(cl_->error() << "\n" << tr("Use -h to get help"));
        return Error;
    }

    // scene file
    if (cl_->contains("scene"))
    {
        sceneFile_ = cl_->value("scene").toString();
        MO_PRINT(tr("fixed scene file") << ": " << sceneFile_);
    }


    // ------------------- display ---------------------
/*
    if (cl_->contains("help"))
    {
        MO_PRINT(tr("Usage") << ":\n" << cl_->helpString());
        return Quit;
    }
*/
    if (cl_->contains("user"))
    {
        application()->setUserName(cl_->value("user").toString());
    }



    // should be last to reflect previous settings
    if (cl_->contains("info"))
    {
        SystemInfo info;
        info.get();
        MO_PRINT("\n" << info.toString() << "\n" << settings()->infoString() << "\n");
        return Quit;
    }


    return Ok;
}

} // namespace MO
