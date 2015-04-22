/** @file stringmanip.cpp

    @brief string manipulation functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include <QObject> // for tr()

#include "stringmanip.h"
#include "math/functions.h"

namespace MO {


void increase_id_number(QString& str, int init)
{
    if (str.isEmpty()) return;

    // is last character digit?
    int end = str.length()-1;

    // no number yet
    if (!str.at(end).isDigit())
    {
        if (init<0) return;

        // add the init digit
        str.append(QString::number(init));
        return;
    }

    // find start of number
    int start = end;
    while (start>1 && str.at(start-1).isDigit()) --start;

    // extract integer
    int value = str.mid(start).toInt();

    value += 1;

    // replace number
    str.replace(start, end-start+1, QString::number(value));
}



QString fit_in_length(const QString &str, int len)
{
    if (str.length() <= len)
        return str;

    return str.left(len-3) + "...";
}



QString byte_to_string(unsigned long int byte)
{
    if (byte <= 1024)
        return QObject::tr("%1b").arg(byte);
    auto p = byte; byte /= 1024;
    if (byte <= 1024)
        return QObject::tr("%1kb").arg(std::floor(double(p)/10.24)/100.);
    p = byte; byte /= 1024;
    if (byte <= 1024)
        return QObject::tr("%1mb").arg(std::floor(double(p)/10.24)/100.);
    p = byte; byte /= 1024;
    if (byte <= 1024)
        return QObject::tr("%1gb").arg(std::floor(double(p)/10.24)/100.);
    p = byte; byte /= 1024;
    if (byte <= 1024)
        return QObject::tr("%1tb").arg(std::floor(double(p)/1.24)/1000.);
    p = byte; byte /= 1024;
    return QObject::tr("%1pb").arg(std::floor(double(p)/1.24)/1000.);
}

QString time_to_string(double sec, bool with_ms)
{
    const int
            minute = sec / 60,
            hour = minute / 60;

    auto s = QString("%1:%2:%3")
        .arg(hour)
        .arg(minute % 60, 2, 10, QChar('0'))
        .arg(int(sec) % 60, 2, 10, QChar('0'));
    if (with_ms)
        s += QString(".%1").arg(int(sec * 1000) % 1000, 3, 10, QChar('0'));

    return s;
}

QString time_to_string_short(double sec, bool with_ms)
{
    sec = std::abs(sec);

    const int
            minute = int(sec / 60) % 60,
            hour = sec / 60 / 60,
            isec = int(sec) % 60;

    QString s;

    // ms
    if (with_ms && MATH::fract(sec))
    {
        s = QString(".%1").arg(int(sec * 1000) % 1000, 3, 10, QChar('0'));
        // nothing follows
        if (sec < 1.)
            return s + QObject::tr("ms");
    }

    // sec
    auto s2 = QString("%1").arg(isec, sec<60. && isec<10 ? 1 : 2, 10, QChar('0'));
    if (s.isEmpty())
        s = s2;
    else
        s = s2 + "." + s;
    // nothing follows?
    if (sec < 60.)
        return s + QObject::tr("s");

    // minutes
    s2 = QString("%1").arg(minute, 2, 10, QChar('0'));
    if (s.isEmpty())
        s = s2;
    else
        s = s2 + ":" + s;

    // hours
    s2 = QString("%1").arg(hour, 2, 10, QChar('0'));
    if (s.isEmpty())
        s = s2;
    else
        s = s2 + ":" + s;

    return s;
}



QString fit_text_block(const QString & s, int max_width, const QString& line_beginning)
{
    QString ret = line_beginning;

    int x = line_beginning.size(), x1, i1;
    bool newline = false;

    // go through string
    for (int i=0; i<s.length(); ++i)
    {
        // -- get next token --

        // process spaces
        while (i < s.length() && s.at(i).isSpace())
        {
            // check linebreak
            if (s.at(i) == '\n')
            {
                goto make_newline;
            }

            if (x < max_width)
            {
                ret.append(s.at(i));
                ++x;
                ++i;
            }
            else
            {
                newline = true;
                break;
            }
        }

        if (i == s.length())
            break;

        // see if next token would fit
        i1=i;
        x1=x;
        while (i1 < s.length() && !s.at(i1).isSpace())
        {
            ++i1; ++x1;
            if (x1 >= max_width)
                break;
        }

        // did not fit
        if (x1 >= max_width)
        {
            // token would match on next line?
            if (i1-i < max_width - line_beginning.size())
            {
                // break at 'i'
                newline = true;
                --i;
            }
            // token must be broken apart
            else
            {
                i1 = i + (max_width - x);
                ret.append(s.mid(i,i1-i));
                i = i1-1;
                newline = true;
            }
        }
        else
        {
            ret.append(s.mid(i,i1-i));
            i = i1-1;
            x = x1;
        }

        if (newline)
        {
make_newline:
            ret += "\n" + line_beginning;
            newline = false;
            x = line_beginning.size();
        }

    }

    return ret;
}


} // namespace MO
