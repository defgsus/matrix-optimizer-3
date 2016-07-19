/** @file clientenginecommandline.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.10.2014</p>
*/

#ifndef CLIENTENGINECOMMANDLINE_H
#define CLIENTENGINECOMMANDLINE_H

#include <QObject>

namespace MO {
namespace IO { class CommandLineParser; }

class ClientEngineCommandLine : public QObject
{
    Q_OBJECT
public:

    enum ReturnValue
    {
        Ok = true,
        Error = false,
        Quit = true + 1
    };

    explicit ClientEngineCommandLine(QObject *parent = 0);
    ~ClientEngineCommandLine();

    // ------------------- parse -----------------------------------

    /** Processes the commandline.
        Setter for global values in Settings will be executed. */
    ReturnValue parse(int argc, char ** argv, int skip);


    // ------------------- getter after parse ----------------------

    /** Enable networking for client */
    bool doNetwork() const { return doNetwork_; }

    /** Show info window on start */
    bool doShowInfoWin() const { return doShowInfoWin_; }

    bool doLoadScene() const { return !sceneFile_.isEmpty(); }

    const QString sceneFile() const { return sceneFile_; }

private:

    IO::CommandLineParser * cl_;

    bool doNetwork_,
         doShowInfoWin_;

    QString sceneFile_;
};

} // namespace MO

#endif // CLIENTENGINECOMMANDLINE_H
