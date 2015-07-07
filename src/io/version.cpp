/** @file version.cpp

    @brief Version information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.10.2014</p>
*/

#include <angelscript.h>
#include <portaudio.h>
#include <portmidi.h>
#include <sndfile.h>
#ifndef MO_DISABLE_DUMB
#include <dumb.h>
#endif

#include <QTextStream>

#include "version.h"

namespace MO {


const QString& versionString()
{
#if 1 //ndef NDEBUG
    static const QString ret =
            QString("%1.%2.%3.%4")
            .arg(MO_VERSION_MAJOR)
            .arg(MO_VERSION_MINOR)
            .arg(MO_VERSION_TINY)
            .arg(MO_VERSION_MICRO, 3, 10, QChar('0'));
#else
    static const QString ret =
            QString("%1.%2.%3")
            .arg(MO_VERSION_MAJOR)
            .arg(MO_VERSION_MINOR)
            .arg(MO_VERSION_TINY);
#endif

    return ret;
}

const QString& applicationName()
{
    static const QString ret =
            "Matrix Optimizer "
//            "Client "
            + versionString()
#ifdef MO_HAMBURG
            + " (Planetarium Hamburg)"
#endif
            ;

    return ret;
}


QString apiVersionString()
{
    QString str;
    QTextStream s(&str);

    s << "<table>\n"
      << "<tr><td>AngelScript</td><td>" << ANGELSCRIPT_VERSION_STRING << "</td></tr>\n"
      << "<tr><td>PortAudio</td><td>" << Pa_GetVersionText() << "</td></tr>\n"
      << "<tr><td>PortMidi</td><td>latest (2006)</td></tr>\n"
      << "<tr><td>sndfile</td><td>" << sf_version_string() << "</td></tr>\n";
#ifndef MO_DISABLE_DUMB
    s << "<tr><td>dumb</td><td>" << DUMB_VERSION_STR << "</td></tr>\n";
#endif
    s << "</table>";

    return str;
}


} // namespace MO
