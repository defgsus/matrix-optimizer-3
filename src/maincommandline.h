/** @file maincommandline.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.03.2015</p>
*/

#ifndef MOSRC_MAINCOMMANDLINE_H
#define MOSRC_MAINCOMMANDLINE_H

#include <QObject>

namespace MO {
namespace IO { class CommandLineParser; }

class MainCommandLine : public QObject
{
    Q_OBJECT
public:

    enum ReturnValue
    {
        Ok = true,
        Error = false,
        Quit = true + 1
    };

    explicit MainCommandLine(QObject *parent = 0);
    ~MainCommandLine();

    QString help(int max_width = 70) const;

    // ------------------- parse -----------------------------------

    /** Processes the commandline.
        Setter for global values in Settings will be executed. */
    ReturnValue parse(int argc, char ** argv, int skip);


    // ------------------- getter after parse ----------------------

    bool doLoadScene() const { return !sceneFile_.isEmpty(); }

    const QString sceneFile() const { return sceneFile_; }

private:

    IO::CommandLineParser * cl_;

    QString sceneFile_;
};

} // namespace MO

#endif // MOSRC_MAINCOMMANDLINE_H
