/** @file testcommandlineparser.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.10.2014</p>
*/

#include "TestCommandLineParser.h"
#include "io/CommandLineParser.h"
#include "io/log.h"
#include "io/version.h"

namespace MO {


TestCommandLineParser::TestCommandLineParser()
{
}


int TestCommandLineParser::run(int argc, char *argv[], int skip)
{
    IO::CommandLineParser parser;

    parser.addParameter("help", "h,help",
                        parser.tr("Displays the help and exits"));

    parser.addParameter("version", "v,version",
                        parser.tr("Shows version information and exits"));

    parser.addParameter("name", "name",
                        parser.tr("Sets the name of something"),
                        "some");

    parser.addParameter("number", "n,number",
                        parser.tr("Sets the number of something"),
                        23);

    parser.addParameter("numbers", "ns,numbers",
                        parser.tr("Sets a few numbers of something else"),
                        QList<double>() << 1 << 2 << 3 << 4);

    parser.addParameter("answer", "a,answer",
        parser.tr("Sets the answer to life, the universe and everything. The purpose of this is merely to "
                  "create a lengthy description to see how it looks in the console. Let's insert a line-break here.\n"
                  "And maybe a double-line-break, although quite uncommon.\n\n"
                  "But that's not enough, let's try some special characters like ä, ö, ü, ß and the notorious < & >. "
                  "Yes, it's certainly not easy to understand the answer fully but then again, it's just lorem ipsum... "
                  "Almost_forgot_to_insert_a_very_long_word_to_see_what_happens_there. What can possibly go wrong?"
                  ), 42u);

    MO_PRINT("parsing...");
    bool ok = parser.parse(argc, argv, skip);

    MO_PRINT("parser log:\n" << parser.error());

    MO_PRINT("arguments:");
    for (auto & s : parser.arguments())
        MO_PRINT(s);

    MO_PRINT("values:\n" << parser.valueString());

    if (!ok)
        return -1;

    if (parser.contains("version"))
    {
        MO_PRINT("MatrixOptimizer " << versionString());
        return 0;
    }

    if (parser.contains("help"))
    {
        MO_PRINT(parser.helpString());
        return 0;
    }

    return 0;
}


} // namespace MO
