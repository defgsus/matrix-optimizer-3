/** @file testcommandlineparser.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.10.2014</p>
*/

#ifndef TESTCOMMANDLINEPARSER_H
#define TESTCOMMANDLINEPARSER_H

namespace MO {

class TestCommandLineParser
{
public:
    TestCommandLineParser();

    int run(int argc, char *argv[], int skip);
};

} // namespace MO


#endif // TESTCOMMANDLINEPARSER_H
